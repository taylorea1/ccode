// #include <TFile.h>
// #include <TTree.h>
// #include <TH2D.h>
// #include <TCanvas.h>
// #include <TBox.h>
// #include <TStyle.h>
// #include <TString.h>
// #include <TSystem.h>
// #include <algorithm>
// #include <cmath>
// #include <iomanip>
// #include <iostream>

// void track_acceptance(double xMin,double xMax,double yMin, double yMax, double deltaZ=0.0, bool savePlot=true){

//     if(xMin>=xMax||yMin>=yMax){
//         std::cerr<<"Error: require xMin<xMax and yMin<yMax.\n";
//         return;
//     }

//     TFile *f=TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
//     if(!f||f->IsZombie()){
//         cout<<"Error opening file!"<<endl;
//         return;
//     }
//     TTree *T=(TTree*)f->Get("T");
//     if(!T){
//         cout<<"Error: tree T was not found.\n";
//         f->Close();
//         return;
//     }

//     const int maxTracks=3000;
//     Double_t x[maxTracks]={0},y[maxTracks]={0},xp[maxTracks]={0},yp[maxTracks]={0};
//     Int_t nX=0,nY=0,nXp=0,nYp=0;

//     T->SetBranchAddress("sbs.gemCeR.track.x",x);
//     T->SetBranchAddress("sbs.gemCeR.track.y",y);
//     T->SetBranchAddress("sbs.gemCeR.track.xp",xp);
//     T->SetBranchAddress("sbs.gemCeR.track.yp",yp);
//     T->SetBranchAddress("Ndata.sbs.gemCeR.track.x",&nX);
//     T->SetBranchAddress("Ndata.sbs.gemCeR.track.y",&nY);
//     T->SetBranchAddress("Ndata.sbs.gemCeR.track.xp",&nXp);
//     T->SetBranchAddress("Ndata.sbs.gemCeR.track.yp",&nYp);

//     double xPad=max(0.10*(xMax-xMin),0.01);
//     double yPad=max(0.10*(yMax-yMin),0.01);
//     TH2D *hAll=new TH2D("hTrackAcceptance", "CeR tracks at analyzer plane;projected x;projected y", 250,xMin-xPad,xMax+xPad,250,yMin-yPad,yMax+yPad);

//     Long64_t totalTracks=0,insideTracks=0;
//     Long64_t entries=T->GetEntries();

//     for(Long64_t event=0;event<entries;event++){
//         T->GetEntry(event);
//         int nTracks=min({nX,nY,nXp,nYp,maxTracks});

//         for(int track=0;track<nTracks;track++){
//             if(!isfinite(x[track])||!isfinite(y[track])||
//                !isfinite(xp[track])||!isfinite(yp[track]))continue;

//             double xProjected=x[track]+xp[track]*deltaZ;
//             double yProjected=y[track]+yp[track]*deltaZ;
//             if(!isfinite(xProjected)||!isfinite(yProjected))continue;

//             totalTracks++;
//             hAll->Fill(xProjected,yProjected);

//             // Include the lower edges and exclude the upper edges to avoid overlap.
//             bool inside=xProjected>=xMin&&xProjected<xMax&&
//                         yProjected>=yMin&&yProjected<yMax;
//             if(inside)insideTracks++;
//         }
//     }

//     double fraction=totalTracks>0?(double)insideTracks/totalTracks:0.0;
//     cout<<"\nTrack-acceptance result\n";
//     cout<<"deltaZ: "<<deltaZ;
//     if(deltaZ==0.0) cout<<" (GEM reference-plane test; no projection distance used)";
//     cout<<"\nValid tracks: "<<totalTracks
//              <<"\nTracks inside analyzer: "<<insideTracks
//              <<"\nAcceptance fraction: "<<fixed<<setprecision(6)<<fraction
//              <<"\nAcceptance percentage: "<<setprecision(2)<<100.0*fraction<<"%\n";

//     gStyle->SetOptStat(0);
//     TCanvas *c=new TCanvas("cTrackAcceptance","Active-analyzer track acceptance",900,750);
//     c->SetRightMargin(0.14);
//     hAll->Draw("COLZ");
//     TBox *boundary=new TBox(xMin,yMin,xMax,yMax);
//     boundary->SetFillStyle(0);
//     boundary->SetLineColor(kRed+1);
//     boundary->SetLineWidth(3);
//     boundary->Draw("same");
//     c->Update();

//     if(savePlot){
//         TString output=Form("track_acceptance_dz_%g.png",deltaZ);
//         c->SaveAs(output);
//         cout<<"Saved plot: "<<gSystem->WorkingDirectory()<<"/"<<output<<"\n";
//     }
// }


#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TBox.h>
#include <TLine.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TString.h>
#include <TSystem.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

void track_acceptance(bool savePlot=true){
    // Survey coordinates are in inches and define negative z as downstream.
    // GEM CeR track x and y use the CE frame center as their reference plane.
    const double inchToMeter=0.0254;
    const double ceFrameCenterZ=-99.008*inchToMeter;
    const double analyzerFrontZ=-125.811*inchToMeter;
    const double deltaZ=analyzerFrontZ-ceFrameCenterZ; // -0.6807962 m

    // Analyzer center and surveyed physical boundaries.
    const double xMin=-3.559*inchToMeter;
    const double xMax= 3.559*inchToMeter;
    const double yMin=-6.724*inchToMeter;
    const double yMax= 6.724*inchToMeter;
    const int nRows=8;
    const int nColumns=4;

    TFile *f=TFile::Open(
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root","READ");
    if(!f||f->IsZombie()){
        cerr<<"Error opening file!\n";
        return;
    }

    TTree *T=(TTree*)f->Get("T");
    if(!T){
        cerr<<"Error: tree T was not found.\n";
        f->Close();
        return;
    }

    const int maxTracks=3000;
    Double_t x[maxTracks]={0},y[maxTracks]={0},xp[maxTracks]={0},yp[maxTracks]={0};
    Int_t nX=0,nY=0,nXp=0,nYp=0;

    T->SetBranchAddress("sbs.gemCeR.track.x",x);
    T->SetBranchAddress("sbs.gemCeR.track.y",y);
    T->SetBranchAddress("sbs.gemCeR.track.xp",xp);
    T->SetBranchAddress("sbs.gemCeR.track.yp",yp);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.x",&nX);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.y",&nY);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.xp",&nXp);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.yp",&nYp);

    vector<double> xBefore;
    vector<double> yBefore;
    vector<double> xProjected;
    vector<double> yProjected;

    Long64_t insideTracks=0;
    const Long64_t entries=T->GetEntries();

    // Store valid positions first so all plots can use data-driven axis ranges.
    for(Long64_t event=0;event<entries;event++){
        T->GetEntry(event);
        const int nTracks=min({nX,nY,nXp,nYp,maxTracks});

        for(int track=0;track<nTracks;track++){
            if(!isfinite(x[track])||!isfinite(y[track])||
               !isfinite(xp[track])||!isfinite(yp[track]))continue;

            const double projectedX=x[track]+xp[track]*deltaZ;
            const double projectedY=y[track]+yp[track]*deltaZ;
            if(!isfinite(projectedX)||!isfinite(projectedY))continue;

            xBefore.push_back(x[track]);
            yBefore.push_back(y[track]);
            xProjected.push_back(projectedX);
            yProjected.push_back(projectedY);

            const bool inside=projectedX>=xMin&&projectedX<xMax&&
                              projectedY>=yMin&&projectedY<yMax;
            if(inside)insideTracks++;
        }
    }

    const Long64_t totalTracks=xProjected.size();
    if(totalTracks==0){
        cerr<<"Error: no valid CeR GEM tracks were found.\n";
        f->Close();
        return;
    }

    double plotXMin=min({xMin,*min_element(xBefore.begin(),xBefore.end()),
                        *min_element(xProjected.begin(),xProjected.end())});
    double plotXMax=max({xMax,*max_element(xBefore.begin(),xBefore.end()),
                        *max_element(xProjected.begin(),xProjected.end())});
    double plotYMin=min({yMin,*min_element(yBefore.begin(),yBefore.end()),
                        *min_element(yProjected.begin(),yProjected.end())});
    double plotYMax=max({yMax,*max_element(yBefore.begin(),yBefore.end()),
                        *max_element(yProjected.begin(),yProjected.end())});

    const double xPad=max(0.10*(plotXMax-plotXMin),0.01);
    const double yPad=max(0.10*(plotYMax-plotYMin),0.01);
    plotXMin-=xPad;
    plotXMax+=xPad;
    plotYMin-=yPad;
    plotYMax+=yPad;

    TH2D *hBefore=new TH2D("hBefore",
        "CeR GEM tracks at CE frame center;x at CE frame center (m);y at CE frame center (m)",
        180,plotXMin,plotXMax,180,plotYMin,plotYMax);

    TH2D *hProjected=new TH2D("hProjected",
        "CeR tracks projected onto active analyzer;projected x (m);projected y (m)",
        180,plotXMin,plotXMax,180,plotYMin,plotYMax);

    TH1D *hProjectedX=new TH1D("hProjectedX",
        "Projected x distribution;projected x (m);tracks",
        250,plotXMin,plotXMax);

    TH1D *hProjectedY=new TH1D("hProjectedY",
        "Projected y distribution;projected y (m);tracks",
        250,plotYMin,plotYMax);

    const double columnWidth=(xMax-xMin)/nColumns;
    const double rowHeight=(yMax-yMin)/nRows;

    for(Long64_t track=0;track<totalTracks;track++){
        hBefore->Fill(xBefore[track],yBefore[track]);
        hProjected->Fill(xProjected[track],yProjected[track]);
        hProjectedX->Fill(xProjected[track]);
        hProjectedY->Fill(yProjected[track]);
    }

    const double fraction=(double)insideTracks/totalTracks;

    cout<<"\nTrack-acceptance result\n"
        <<"GEM reference: CE frame center\n"
        <<"CE frame center z: "<<ceFrameCenterZ<<" m\n"
        <<"Analyzer front z: "<<analyzerFrontZ<<" m\n"
        <<"deltaZ used: "<<deltaZ<<" m\n"
        <<"Analyzer x range: ["<<xMin<<", "<<xMax<<"] m\n"
        <<"Analyzer y range: ["<<yMin<<", "<<yMax<<"] m\n"
        <<"Geometry: "<<nRows<<" rows x "<<nColumns<<" columns\n"
        <<"Valid tracks: "<<totalTracks
        <<"\nTracks inside analyzer: "<<insideTracks
        <<"\nAcceptance fraction: "<<fixed<<setprecision(6)<<fraction
        <<"\nAcceptance percentage: "<<setprecision(2)<<100.0*fraction<<"%\n";

    gStyle->SetOptStat(0);

    // Plot 1: measured GEM coordinates at the CE frame center.
    TCanvas *cBefore=new TCanvas("cBeforeProjection",
        "CeR GEM tracks before projection",1000,850);
    cBefore->SetLeftMargin(0.12);
    cBefore->SetRightMargin(0.16);
    cBefore->SetBottomMargin(0.12);
    cBefore->SetLogz();
    hBefore->SetMinimum(1.0);
    hBefore->SetContour(100);
    hBefore->Draw("COLZ");
    cBefore->Update();

    // Plot 2: projected tracks, analyzer boundary, and 8x4 segmentation.
    TCanvas *cProjected=new TCanvas("cProjectedTracks",
        "CeR tracks projected onto active analyzer",1000,850);
    cProjected->SetLeftMargin(0.12);
    cProjected->SetRightMargin(0.16);
    cProjected->SetBottomMargin(0.12);
    cProjected->SetLogz();
    hProjected->SetMinimum(1.0);
    hProjected->SetContour(100);
    hProjected->Draw("COLZ");

    TBox *boundary=new TBox(xMin,yMin,xMax,yMax);
    boundary->SetFillStyle(0);
    boundary->SetLineColor(kRed+1);
    boundary->SetLineWidth(3);
    boundary->Draw("same");

    for(int column=1;column<nColumns;column++){
        const double xEdge=xMin+column*columnWidth;
        TLine *line=new TLine(xEdge,yMin,xEdge,yMax);
        line->SetLineColor(kRed+1);
        line->SetLineStyle(2);
        line->Draw("same");
    }
    for(int row=1;row<nRows;row++){
        const double yEdge=yMin+row*rowHeight;
        TLine *line=new TLine(xMin,yEdge,xMax,yEdge);
        line->SetLineColor(kRed+1);
        line->SetLineStyle(2);
        line->Draw("same");
    }

    TLatex *acceptanceText=new TLatex();
    acceptanceText->SetNDC();
    acceptanceText->SetTextSize(0.035);
    acceptanceText->SetTextColor(kRed+1);
    acceptanceText->DrawLatex(0.12,0.92,
        Form("Acceptance: %.2f%% (%lld/%lld)",
             100.0*fraction,insideTracks,totalTracks));
    cProjected->Update();

    // Plot 3: horizontal acceptance and losses.
    TCanvas *cProjectedX=new TCanvas("cProjectedX",
        "Projected x distribution",1000,700);
    cProjectedX->SetLeftMargin(0.12);
    cProjectedX->SetBottomMargin(0.12);
    hProjectedX->SetLineColor(kBlue+1);
    hProjectedX->SetLineWidth(2);
    hProjectedX->Draw("HIST");
    const double xLineTop=1.05*hProjectedX->GetMaximum();
    TLine *xLowerLine=new TLine(xMin,0,xMin,xLineTop);
    TLine *xUpperLine=new TLine(xMax,0,xMax,xLineTop);
    xLowerLine->SetLineColor(kRed+1);
    xUpperLine->SetLineColor(kRed+1);
    xLowerLine->SetLineWidth(2);
    xUpperLine->SetLineWidth(2);
    xLowerLine->Draw("same");
    xUpperLine->Draw("same");
    cProjectedX->Update();

    // Plot 4: vertical acceptance and losses.
    TCanvas *cProjectedY=new TCanvas("cProjectedY",
        "Projected y distribution",1000,700);
    cProjectedY->SetLeftMargin(0.12);
    cProjectedY->SetBottomMargin(0.12);
    hProjectedY->SetLineColor(kBlue+1);
    hProjectedY->SetLineWidth(2);
    hProjectedY->Draw("HIST");
    const double yLineTop=1.05*hProjectedY->GetMaximum();
    TLine *yLowerLine=new TLine(yMin,0,yMin,yLineTop);
    TLine *yUpperLine=new TLine(yMax,0,yMax,yLineTop);
    yLowerLine->SetLineColor(kRed+1);
    yUpperLine->SetLineColor(kRed+1);
    yLowerLine->SetLineWidth(2);
    yUpperLine->SetLineWidth(2);
    yLowerLine->Draw("same");
    yUpperLine->Draw("same");
    cProjectedY->Update();

    if(savePlot){
        TString beforeOutput="track_acceptance_before_projection.png";
        TString projectedOutput="track_acceptance_projected_xy.png";
        TString xOutput="track_acceptance_projected_x.png";
        TString yOutput="track_acceptance_projected_y.png";
        cBefore->SaveAs(beforeOutput);
        cProjected->SaveAs(projectedOutput);
        cProjectedX->SaveAs(xOutput);
        cProjectedY->SaveAs(yOutput);
        cout<<"Saved plots:\n"
            <<gSystem->WorkingDirectory()<<"/"<<beforeOutput<<"\n"
            <<gSystem->WorkingDirectory()<<"/"<<projectedOutput<<"\n"
            <<gSystem->WorkingDirectory()<<"/"<<xOutput<<"\n"
            <<gSystem->WorkingDirectory()<<"/"<<yOutput<<"\n";
    }
}
