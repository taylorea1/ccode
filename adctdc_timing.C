#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <iostream>


void adctdc_timing(int selectedBar=0,double tdcCalMin=40.0,double tdcCalMax=60.0,bool savePlots=false)
{
    double tdcScale=0.0936; // Converts raw TDC units to ns
    int maxHits=10000;

    if(selectedBar<0||selectedBar>31){
        cout<<"Error: selectedBar must be between 0 and 31\n";
        return;
    }
    if(tdcCalMin>=tdcCalMax){
        cout<<"Error: tdcCalMin must be smaller than tdcCalMax\n";
        return;
    }

    TFile *f=TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
    if(!f||f->IsZombie()){
        cout<<"Error opening input file\n";
        return;
    }

    TTree *T=(TTree*)f->Get("T");
    if(!T){
        cout<<"Error: TTree T was not found\n";
        f->Close();
        return;
    }

    const char *requiredBranches[]={
        "sbs.activeAna_adc.a_p",
        "sbs.activeAna_adc.a_time",
        "sbs.activeAna_adc.adcelemID",
        "sbs.activeAna_tdc.hits.t",
        "sbs.activeAna_tdc.hits.TDCelemID",
        "Ndata.sbs.activeAna_adc.a_p",
        "Ndata.sbs.activeAna_adc.a_time",
        "Ndata.sbs.activeAna_adc.adcelemID",
        "Ndata.sbs.activeAna_tdc.hits.t",
        "Ndata.sbs.activeAna_tdc.hits.TDCelemID"
    };
    for(const char *name:requiredBranches){
        if(!T->GetBranch(name)){
            cout<<"Error: missing branch "<<name<<'\n';
            f->Close();
            return;
        }
    }

    Double_t adc[maxHits],adcTime[maxHits],adcBar[maxHits];
    Double_t tdcRaw[maxHits],tdcBar[maxHits];
    Int_t nADC=0,nADCTime=0,nADCBar=0,nTDC=0,nTDCBar=0;

    T->SetBranchAddress("sbs.activeAna_adc.a_p",adc);
    T->SetBranchAddress("sbs.activeAna_adc.a_time",adcTime);
    T->SetBranchAddress("sbs.activeAna_adc.adcelemID",adcBar);
    T->SetBranchAddress("sbs.activeAna_tdc.hits.t",tdcRaw);
    T->SetBranchAddress("sbs.activeAna_tdc.hits.TDCelemID",tdcBar);
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.a_p",&nADC);
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.a_time",&nADCTime);
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcelemID",&nADCBar);
    T->SetBranchAddress("Ndata.sbs.activeAna_tdc.hits.t",&nTDC);
    T->SetBranchAddress("Ndata.sbs.activeAna_tdc.hits.TDCelemID",&nTDCBar);

    TH1D *hADCAll=new TH1D("hADCAll",Form("Bar %d ADC: no TDC cut;ADC pulse integral a_{p};Entries",selectedBar),200,-0.5,200.5);
    TH1D *hADCCut=new TH1D("hADCCut",Form("Bar %d ADC: calibrated TDC cut;ADC pulse integral a_{p};Entries",selectedBar),200,-0.5,200.5);
    TH1D *hTDCRaw=new TH1D("hTDCRaw",Form("Bar %d raw TDC;raw TDC value;Entries",selectedBar),240,-1200,1200);
    TH1D *hTDCCal=new TH1D("hTDCCal",Form("Bar %d calibrated TDC;calibrated TDC time (ns);Entries",selectedBar),300,-100,500);
    TH1D *hADCTime=new TH1D("hADCTime",Form("Bar %d ADC and calibrated TDC times;time (ns);Entries",selectedBar),250,0,500);
    TH1D *hTDCCalOverlay=new TH1D("hTDCCalOverlay","",250,0,500);
    TH2D *hTimeCorr=new TH2D("hTimeCorr",Form("Bar %d: ADC time vs calibrated TDC time, all same-event pairs;ADC time (ns);calibrated TDC time (ns)",selectedBar),200,0,500,200,0,500);
    TH2D *hTimeCorr11=new TH2D("hTimeCorr11",Form("Bar %d: one ADC hit and one TDC hit;ADC time (ns);calibrated TDC time (ns)",selectedBar),200,0,500,200,0,500);

    Long64_t adcEntries=0,adcPassing=0,allPairs=0,oneToOnePairs=0;
    const Long64_t nEvents=T->GetEntries();

    for(Long64_t event=0;event<nEvents;++event){
        T->GetEntry(event);
        const int nAP=min({nADC,nADCBar,maxHits});
        const int nAT=min({nADC,nADCTime,nADCBar,maxHits});
        const int nT=min({nTDC,nTDCBar,maxHits});

        // Fill raw and calibrated TDC spectra without a physics timing cut.
        for(int it=0;it<nT;++it){
            const int bar=(int)lround(tdcBar[it]);
            const double raw=tdcRaw[it];
            if(bar!=selectedBar||!isfinite(raw)||fabs(raw)>1.0e6) continue;
            const double cal=tdcScale*raw;
            hTDCRaw->Fill(raw);
            hTDCCal->Fill(cal);
            hTDCCalOverlay->Fill(cal);
        }

        // Compare ADC pulse-integral spectra before and after the relaxed calibrated cut.
        for(int ia=0;ia<nAP;++ia){
            const int bar=(int)lround(adcBar[ia]);
            if(bar!=selectedBar||!isfinite(adc[ia])) continue;
            ++adcEntries;
            hADCAll->Fill(adc[ia]);

            bool sameBarInTime=false;
            for(int it=0;it<nT;++it){
                const int hitBar=(int)lround(tdcBar[it]);
                const double cal=tdcScale*tdcRaw[it];
                if(hitBar==bar&&isfinite(cal)&&cal>=tdcCalMin&&cal<=tdcCalMax){
                    sameBarInTime=true;
                    break;
                }
            }
            if(sameBarInTime){
                ++adcPassing;
                hADCCut->Fill(adc[ia]);
            }
        }

        // Correlate same-event, same-bar ADC and calibrated TDC times with no time cut.
        int adcHitCount=0,tdcHitCount=0,onlyADC=-1,onlyTDC=-1;
        for(int ia=0;ia<nAT;++ia){
            if((int)lround(adcBar[ia])!=selectedBar||!isfinite(adcTime[ia])) continue;
            ++adcHitCount;
            onlyADC=ia;
            hADCTime->Fill(adcTime[ia]);
            for(int it=0;it<nT;++it){
                const double cal=tdcScale*tdcRaw[it];
                if((int)lround(tdcBar[it])!=selectedBar||!isfinite(cal)||fabs(tdcRaw[it])>1.0e6) continue;
                hTimeCorr->Fill(adcTime[ia],cal);
                ++allPairs;
            }
        }
        for(int it=0;it<nT;++it){
            if((int)lround(tdcBar[it])!=selectedBar||!isfinite(tdcRaw[it])||fabs(tdcRaw[it])>1.0e6) continue;
            ++tdcHitCount;
            onlyTDC=it;
        }
        if(adcHitCount==1&&tdcHitCount==1){
            hTimeCorr11->Fill(adcTime[onlyADC],tdcScale*tdcRaw[onlyTDC]);
            ++oneToOnePairs;
        }
    }

    gStyle->SetOptStat(1110);

    TCanvas *cADC=new TCanvas("cADC","ADC Spectrum Comparison",1000,750);
    cADC->SetLogy();
    hADCAll->SetLineColor(kBlack);
    hADCCut->SetLineColor(kRed+1);
    hADCAll->SetLineWidth(2);
    hADCCut->SetLineWidth(2);
    hADCAll->Draw("HIST");
    hADCCut->Draw("HIST SAME");
    TLegend *adcLegend=new TLegend(0.50,0.73,0.88,0.88);
    adcLegend->AddEntry(hADCAll,"No TDC cut","l");
    adcLegend->AddEntry(hADCCut,Form("%.1f #leq calibrated TDC #leq %.1f ns",tdcCalMin,tdcCalMax),"l");
    adcLegend->Draw();

    TCanvas *cTDC=new TCanvas("cTDC","Raw and Calibrated TDC",1200,550);
    cTDC->Divide(2,1);
    cTDC->cd(1); hTDCRaw->Draw("HIST");
    cTDC->cd(2); hTDCCal->Draw("HIST");

    TCanvas *cCorr=new TCanvas("cCorr","ADC-TDC Time Correlation",1200,550);
    cCorr->Divide(2,1);
    cCorr->cd(1); hTimeCorr->Draw("COLZ");
    cCorr->cd(2); hTimeCorr11->Draw("COLZ");

    TCanvas *cOverlay=new TCanvas("cOverlay","ADC and Calibrated TDC Times",1000,750);
    hADCTime->SetLineColor(kBlue+1);
    hTDCCalOverlay->SetLineColor(kRed+1);
    hADCTime->SetLineWidth(2);
    hTDCCalOverlay->SetLineWidth(2);
    hADCTime->Draw("HIST");
    hTDCCalOverlay->Draw("HIST SAME");
    TLegend *timeLegend=new TLegend(0.58,0.75,0.88,0.88);
    timeLegend->AddEntry(hADCTime,"ADC a_{time}","l");
    timeLegend->AddEntry(hTDCCalOverlay,"0.0936 #times raw TDC","l");
    timeLegend->Draw();

    // Canvases remain open interactively. Files are written only when requested.
    if(savePlots){
        cADC->SaveAs(Form("activeAna_bar%d_adc_comparison.png",selectedBar));
        cTDC->SaveAs(Form("activeAna_bar%d_tdc_calibration.png",selectedBar));
        cCorr->SaveAs(Form("activeAna_bar%d_adc_tdc_time_correlation.png",selectedBar));
        cOverlay->SaveAs(Form("activeAna_bar%d_adc_tdc_time_overlay.png",selectedBar));
    }

    cout<<"Events read: "<<nEvents<<'\n'
        <<"ADC entries for bar "<<selectedBar<<": "<<adcEntries<<'\n'
        <<"ADC entries passing calibrated TDC cut: "<<adcPassing<<'\n'
        <<"All same-event/same-bar time pairs: "<<allPairs<<'\n'
        <<"Events with exactly one ADC and one TDC hit: "<<oneToOnePairs<<'\n'
        <<"Calibrated TDC cut: ["<<tdcCalMin<<", "<<tdcCalMax<<"] ns\n";
    if(adcEntries>0) cout<<"Passing fraction: "<<100.0*adcPassing/adcEntries<<"%\n";
    if(!savePlots) cout<<"Plots are displayed but not saved. Pass true as the fourth argument to save them.\n";
}
