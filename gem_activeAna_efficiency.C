// Standalone companion to track_acceptance.C and adctdc_spectrumtiming.C.
// After a crash, start a fresh ROOT session and force compilation with symbols:
// ROOT: .L gem_activeAna_efficiency.C++g
//       gem_activeAna_efficiency();                 // all bars, display only
//       gem_activeAna_efficiency(18,40,60);          // predicted bar 18 only
//       gem_activeAna_efficiency(-1,40,60,true);     // also save PNGs and CSV
//       gem_activeAna_efficiency(-1,40,60,false,true); // one-track events only
// No need to load either earlier macro. Their files are not modified.

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
using namespace std;

namespace GAA {
struct Config {
    double tdcScale=0.0936;
    double tdcMin=40.0,tdcMax=60.0;
    // OFF matches the supplied timing macro: finite ADC, with no amplitude cut.
    bool useADCThreshold=false;
    double minADC=0.0; // When enabled, require ADC >= minADC.

    double deltaZ=(-125.811+99.008)*0.0254;
    double xMin=-3.559*0.0254,xMax=3.559*0.0254;
    double yMin=-6.724*0.0254,yMax=6.724*0.0254;
    // Preserve the earlier acceptance macro's transverse-coordinate convention.
    // CE center has survey y=-1.500 in. If track x,y are CE-centered LOCAL
    // coordinates with axes parallel to survey axes, set yOffset=-1.5*0.0254.
    // A CE reference z plane alone does not establish the transverse origin.
    double xOffset=0.0,yOffset=0.0;
    // Assumes xp,yp use the survey's negative-downstream z convention.
    // Do not pick the sign by maximizing efficiency; verify the replay axes.

    // Provisional equal-width 8x4 segmentation, with no inactive gaps.
    // The point-data sheet does not establish per-bar edges or channel direction.
    // Default: column 0 at low x, row 0 at low y, bar=4*row+column.
    bool flipColumns=false,flipRows=false;
    int selectedBar=-1; // -1 means all predicted bars.
    bool singleTrackOnly=false;
};

struct Track { double x,y,xp,yp; };
struct Projection {
    double x,y;
    bool inX,inY;
    int row=-1,column=-1,bar=-1;
};
struct Response {
    int adcHits=0,tdcInWindow=0;
    bool adc() const { return adcHits>0; }
    bool both() const { return adcHits>0&&tdcInWindow>0; }
};

inline int barID(double id){
    // Match lround in the supplied macro, but reject invalid IDs before rounding.
    if(!isfinite(id)||id<=-0.5||id>=31.5)return -1;
    return (int)lround(id);
}
inline bool adcPass(double value,const Config &c){
    return isfinite(value)&&(!c.useADCThreshold||value>=c.minADC);
}
inline bool tdcPass(double raw,const Config &c){
    // Identical inclusive calibrated cut to the supplied ADC-spectrum loop.
    const double calibrated=c.tdcScale*raw;
    return isfinite(calibrated)&&calibrated>=c.tdcMin&&calibrated<=c.tdcMax;
}
inline bool diagnosticTDC(double raw){
    // The source uses this raw-value guard for spectra/time diagnostics only.
    return isfinite(raw)&&fabs(raw)<=1.0e6;
}
inline bool project(const Track &t,const Config &c,Projection &p){
    if(!isfinite(t.x)||!isfinite(t.y)||!isfinite(t.xp)||!isfinite(t.yp))return false;
    p=Projection{};
    p.x=t.x+c.xOffset+t.xp*c.deltaZ;
    p.y=t.y+c.yOffset+t.yp*c.deltaZ;
    if(!isfinite(p.x)||!isfinite(p.y))return false;
    p.inX=p.x>=c.xMin&&p.x<c.xMax;
    p.inY=p.y>=c.yMin&&p.y<c.yMax;
    if(p.inX&&p.inY){
        int col=min(3,(int)(4*(p.x-c.xMin)/(c.xMax-c.xMin)));
        int row=min(7,(int)(8*(p.y-c.yMin)/(c.yMax-c.yMin)));
        p.column=c.flipColumns?3-col:col;
        p.row=c.flipRows?7-row:row;
        p.bar=4*p.row+p.column;
    }
    return true;
}
inline array<Response,32> responses(const double *adc,const double *adcBar,int nA,
                                   const double *tdc,const double *tdcBar,int nT,
                                   const Config &c){
    array<Response,32> result{};
    for(int i=0;i<nA;i++){
        const int b=barID(adcBar[i]);
        if(b>=0&&adcPass(adc[i],c))result[b].adcHits++;
    }
    for(int i=0;i<nT;i++){
        const int b=barID(tdcBar[i]);
        if(b>=0&&tdcPass(tdc[i],c))result[b].tdcInWindow++;
    }
    return result;
}
struct GeometryCounts {
    long long total=0,inX=0,inY=0,inside=0,failX=0,failY=0,failBoth=0;
    void add(const Projection &p){
        total++; inX+=p.inX; inY+=p.inY;
        if(p.inX&&p.inY)inside++;
        else if(!p.inX&&p.inY)failX++;
        else if(p.inX&&!p.inY)failY++;
        else failBoth++;
    }
};
struct Record {
    long long event;
    int track;
    Projection p;
    int adcHits,tdcHits;
};
} // namespace GAA

// This guard permits the exact matching/counting functions above to be tested
// with a normal C++ compiler when ROOT is not installed.
#ifndef GAA_LOGIC_ONLY
#include <TBranch.h>
#include <TCanvas.h>
#include <TEfficiency.h>
#include <TFile.h>
#include <TGraphAsymmErrors.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TLeaf.h>
#include <TLegend.h>
#include <TLine.h>
#include <TObjArray.h>
#include <TString.h>
#include <TStyle.h>
#include <TTree.h>

namespace GAA {
// Still SetBranchAddress, not TTreeReader. Read and validate each count BEFORE
// reading arrays. min(count,capacity) AFTER T->GetEntry would not prevent overflow.
struct InputArray {
    string name;
    Int_t count=0;
    vector<Double_t> values;
    TBranch *countBranch=nullptr,*dataBranch=nullptr;
    InputArray(const char *n,int capacity):name(n),values(capacity){}
    bool bind(TTree *t){
        const string countName="Ndata."+name;
        if(!t->GetBranch(name.c_str())||!t->GetBranch(countName.c_str())){
            cerr<<"Missing required branch: "<<name<<" or "<<countName<<'\n';
            return false;
        }
        if(t->SetBranchAddress(countName.c_str(),&count,&countBranch)<0||
           t->SetBranchAddress(name.c_str(),values.data(),&dataBranch)<0){
            cerr<<"Branch type/address mismatch: "<<name<<'\n';return false;
        }
        // This reader expects the standard one-dimensional Double_t leaf array.
        auto leaves=dataBranch->GetListOfLeaves();
        TLeaf *leaf=leaves&&leaves->GetEntriesFast()==1?
            dynamic_cast<TLeaf*>(leaves->At(0)):nullptr;
        if(!leaf||leaf->GetLenStatic()!=1||!leaf->GetLeafCount()||
           leaf->GetLeafCount()->GetBranch()!=countBranch){
            cerr<<"Unsupported array/count layout for "<<name<<"; stopping safely.\n";
            return false;
        }
        return true;
    }
    bool readCount(Long64_t event){
        count=0;
        if(countBranch->GetEntry(event)<=0||count<0||count>(int)values.size()){
            cerr<<"Invalid/oversize count or read error at entry "<<event
                <<" for "<<name<<": "<<count<<" (capacity "<<values.size()<<")\n";
            return false;
        }
        return true;
    }
    bool readData(Long64_t event){
        if(count==0||dataBranch->GetEntry(event)>0)return true;
        cerr<<"Array read failed at entry "<<event<<" for "<<name<<'\n';return false;
    }
    double operator[](int i) const {return values[i];}
};

inline TCanvas *canvas(const TString &name,const char *title,bool twoD=false){
    TCanvas *c=new TCanvas(name,title,1100,twoD?900:760);
    c->SetLeftMargin(0.12);c->SetRightMargin(twoD?0.16:0.05);
    c->SetBottomMargin(0.12);c->SetTopMargin(0.12);
    return c;
}
// Named helpers avoid the lambda code-generation path implicated by the
// ROOT 6.36.04 Cling crash trace (getLambdaStaticInvoker).
inline void keepHistogram(TH1 *h,vector<unique_ptr<TH1>> &owned){
    h->SetDirectory(nullptr);
    owned.emplace_back(h);
}
inline TCanvas *draw2D(TH2D *h,const char *suffix,const TString &tag,
                      vector<pair<TCanvas*,string>> &canvases){
    TCanvas *c=canvas(tag+suffix,h->GetTitle(),true);
    h->GetZaxis()->SetTitle("Entries");h->SetContour(80);
    h->SetMinimum(0.5);h->SetMaximum(max(2.0,h->GetMaximum()));
    if(h->GetEntries()>0)c->SetLogz();
    h->Draw("COLZ");c->Update();canvases.push_back({c,suffix});
    return c;
}
inline void interval(long long pass,long long total,double &low,double &high){
    low=TEfficiency::ClopperPearson(total,pass,0.682689492,false);
    high=TEfficiency::ClopperPearson(total,pass,0.682689492,true);
}
inline void printFraction(const char *label,long long pass,long long total){
    cout<<label<<": "<<pass<<" / "<<total;
    if(total)cout<<" = "<<fixed<<setprecision(2)<<100.0*pass/total<<"%\n";
    else cout<<" (undefined: no denominator tracks)\n";
}
}

void gem_activeAna_efficiency(int selectedBar=-1,double tdcCalMin=40.0,
    double tdcCalMax=60.0,bool savePlots=false,bool singleTrackOnly=false,
    const char *inputFile="/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root"){
    using namespace GAA;
    Config cfg; // Geometry/orientation/optional ADC threshold are at the top.
    cfg.selectedBar=selectedBar;cfg.tdcMin=tdcCalMin;cfg.tdcMax=tdcCalMax;
    cfg.singleTrackOnly=singleTrackOnly;
    if(selectedBar<-1||selectedBar>31||!isfinite(tdcCalMin)||
       !isfinite(tdcCalMax)||tdcCalMin>=tdcCalMax){
        cerr<<"Require selectedBar=-1 or 0..31 and finite tdcCalMin<tdcCalMax.\n";return;
    }
    cout<<"gem_activeAna_efficiency: starting (named plotting helpers)."<<endl;

    InputArray x("sbs.gemCeR.track.x",3000),y("sbs.gemCeR.track.y",3000);
    InputArray xp("sbs.gemCeR.track.xp",3000),yp("sbs.gemCeR.track.yp",3000);
    InputArray adc("sbs.activeAna_adc.a_p",1000),at("sbs.activeAna_adc.a_time",1000);
    InputArray ab("sbs.activeAna_adc.adcelemID",1000);
    InputArray tr("sbs.activeAna_tdc.hits.t",1000),tb("sbs.activeAna_tdc.hits.TDCelemID",1000);
    array<InputArray*,9> arrays={{&x,&y,&xp,&yp,&adc,&at,&ab,&tr,&tb}};
    unique_ptr<TFile> f(TFile::Open(inputFile,"READ"));
    if(!f||f->IsZombie()){cerr<<"Cannot open "<<inputFile<<'\n';return;}
    TTree *t=nullptr;f->GetObject("T",t);
    if(!t){cerr<<"Tree T not found.\n";return;}
    for(auto a:arrays)if(!a->bind(t)){t->ResetBranchAddresses();return;}

    // Unique names allow repeated calls and coexistence with the timing macro.
    static int run=0;
    TString tag=Form("gaa_%d",++run);
    TString label=selectedBar<0?"all predicted bars":Form("predicted bar %d",selectedBar);
    vector<unique_ptr<TH1>> owned;
    TH1D *hADCAll=new TH1D(tag+"_adcAll",label+";ADC pulse integral a_{p};ADC hits",200,-0.5,200.5);
    TH1D *hADCCut=new TH1D(tag+"_adcCut","",200,-0.5,200.5);
    TH2D *hBars=new TH2D(tag+"_bars",
        "Same-event candidate bars (not unique associations);Predicted bar;ADC+TDC candidate bar",
        32,-0.5,31.5,32,-0.5,31.5);
    TH2D *hMatched=new TH2D(tag+"_matched",
        "Crossings with predicted-bar ADC + in-window TDC;Projected x (m);Projected y (m)",
        60,cfg.xMin,cfg.xMax,100,cfg.yMin,cfg.yMax);
    TH2D *hUnmatched=new TH2D(tag+"_unmatched",
        "Crossings failing the joint ADC+TDC requirement;Projected x (m);Projected y (m)",
        60,cfg.xMin,cfg.xMax,100,cfg.yMin,cfg.yMax);
    TH2D *hTimes=new TH2D(tag+"_times",
        "Predicted-bar timing: all same-event pairs, no time cut;ADC time (ns);0.0936 #times raw TDC (ns)",
        100,0,500,100,0,500);
    TH2D *hTimes11=new TH2D(tag+"_times11",
        "Predicted-bar timing: exactly one ADC time and one valid TDC;ADC time (ns);0.0936 #times raw TDC (ns)",
        100,0,500,100,0,500);
    for(TH1 *h:array<TH1*,7>{{hADCAll,hADCCut,hBars,hMatched,hUnmatched,hTimes,hTimes11}})
        keepHistogram(h,owned);

    GeometryCounts geo;
    array<long long,32> denominator{},adcPassed{},bothPassed{};
    vector<Record> records;
    long long countMismatch=0,multiTrackEvents=0,sharedBarTracks=0;
    long long invalidTracks=0,eventsWithCrossings=0,responseTracks=0;
    long long adcTotal=0,bothTotal=0,timePairs=0,timePairs11=0;
    const Long64_t nEvents=t->GetEntries();
    cout<<"Branches connected; reading "<<nEvents<<" events."<<endl;
    for(Long64_t event=0;event<nEvents;event++){
        // Never read unbounded arrays with T->GetEntry before checking counts.
        for(auto a:arrays)if(!a->readCount(event)){t->ResetBranchAddresses();return;}
        for(auto a:arrays)if(!a->readData(event)){t->ResetBranchAddresses();return;}
        const int nTracks=min({x.count,y.count,xp.count,yp.count});
        const int nAP=min(adc.count,ab.count),nAT=min({adc.count,at.count,ab.count});
        const int nT=min(tr.count,tb.count);
        if(x.count!=y.count||x.count!=xp.count||x.count!=yp.count||
           adc.count!=ab.count||adc.count!=at.count||tr.count!=tb.count)countMismatch++;
        if(nTracks>1)multiTrackEvents++;
        array<Response,32> response=responses(adc.values.data(),ab.values.data(),nAP,
                                             tr.values.data(),tb.values.data(),nT,cfg);
        array<int,32> crossings{};
        vector<pair<int,Projection>> accepted;
        for(int it=0;it<nTracks;it++){
            Projection p;
            if(!project({x[it],y[it],xp[it],yp[it]},cfg,p)){invalidTracks++;continue;}
            geo.add(p); // Geometry always includes all valid tracks, before response cuts.
            if(p.bar<0||(selectedBar>=0&&p.bar!=selectedBar))continue;
            if(singleTrackOnly&&!(x.count==1&&y.count==1&&xp.count==1&&yp.count==1))continue;
            crossings[p.bar]++;accepted.push_back({it,p});
        }
        if(!accepted.empty())eventsWithCrossings++;
        for(int b=0;b<32;b++)if(crossings[b]>1)sharedBarTracks+=crossings[b];
        for(const auto &entry:accepted){
            const Projection &p=entry.second;
            const Response &r=response[p.bar];
            denominator[p.bar]++;responseTracks++;
            if(r.adc()){adcPassed[p.bar]++;adcTotal++;}
            if(r.both()){bothPassed[p.bar]++;bothTotal++;hMatched->Fill(p.x,p.y);}
            else hUnmatched->Fill(p.x,p.y);
            if(savePlots)records.push_back({event,entry.first,p,r.adcHits,r.tdcInWindow});
            // One entry per candidate BAR, per predicted track; not per pulse.
            // Keep off-diagonal candidates so geometry/mapping errors are visible.
            for(int observed=0;observed<32;observed++)
                if(response[observed].both())hBars->Fill(p.bar,observed);
        }
        // Spectra/time plots fill once per event/predicted bar, avoiding duplicate
        // pulses when two tracks predict the same bar. They are HIT/PAIR counts,
        // unlike the efficiency's at-most-one pass per track.
        for(int b=0;b<32;b++)if(crossings[b]>0){
            for(int ia=0;ia<nAP;ia++)if(barID(ab[ia])==b&&adcPass(adc[ia],cfg)){
                hADCAll->Fill(adc[ia]);
                if(response[b].tdcInWindow>0)hADCCut->Fill(adc[ia]);
            }
            vector<double> adcTimes,tdcTimes;
            for(int ia=0;ia<nAT;ia++)if(barID(ab[ia])==b&&isfinite(at[ia]))adcTimes.push_back(at[ia]);
            for(int it=0;it<nT;it++)if(barID(tb[it])==b&&diagnosticTDC(tr[it]))
                tdcTimes.push_back(cfg.tdcScale*tr[it]);
            for(double a:adcTimes)for(double d:tdcTimes){hTimes->Fill(a,d);timePairs++;}
            if(adcTimes.size()==1&&tdcTimes.size()==1){
                hTimes11->Fill(adcTimes[0],tdcTimes[0]);timePairs11++;
            }
        }
    }
    t->ResetBranchAddresses();f->Close();
    cout<<"\nGeometry cut summary (all valid GEM tracks, before ADC/TDC cuts)\n"
        <<"Events read: "<<nEvents<<"\nValid tracks: "<<geo.total
        <<"\nInside x: "<<geo.inX<<"\nInside y: "<<geo.inY
        <<"\nInside both: "<<geo.inside<<"\nFailing only x: "<<geo.failX
        <<"\nFailing only y: "<<geo.failY<<"\nFailing both: "<<geo.failBoth<<'\n';
    printFraction("Geometric track acceptance",geo.inside,geo.total);
    cout<<"\nResponse sample: "<<label<<"; "<<(singleTrackOnly?"single-track events":"all tracks")
        <<"\nEvents contributing crossings: "<<eventsWithCrossings
        <<"\nSelected predicted crossings (denominator): "<<responseTracks<<'\n';
    printFraction("Predicted-bar ADC response",adcTotal,responseTracks);
    printFraction("Predicted-bar ADC AND calibrated-TDC response",bothTotal,responseTracks);
    cout<<"Crossings with no qualifying ADC: "<<responseTracks-adcTotal
        <<"\nCrossings with ADC but no in-window TDC: "<<adcTotal-bothTotal
        <<"\nTDC cut: "<<tdcCalMin<<" <= 0.0936*raw <= "<<tdcCalMax<<" ns (inclusive)\n"
        <<"ADC threshold: "<<(cfg.useADCThreshold?"enabled":"OFF (source behavior)")
        <<"; ADC-time/delta-time cut: NONE\n"
        <<"Events with unequal array counts: "<<countMismatch<<" (common indices only)\n"
        <<"Nonfinite tracks excluded: "<<invalidTracks
        <<"\nMulti-track events: "<<multiTrackEvents
        <<"\nTracks sharing a predicted bar in one event: "<<sharedBarTracks
        <<"\nTiming pairs, all / one-to-one: "<<timePairs<<" / "<<timePairs11<<'\n';
    cout<<"ASSUMPTIONS: parallel survey/GEM axes; deltaZ="<<cfg.deltaZ
        <<" m; xOffset="<<cfg.xOffset<<" m; yOffset="<<cfg.yOffset<<" m.\n"
        <<"If track x,y are CE-local, set Config::yOffset=-1.5*0.0254 m.\n"
        <<"8x4 equal bar cells and their orientation must be checked before interpreting efficiency.\n"
        <<"These are track-level response fractions, not a calibrated intrinsic detector efficiency.\n";
    if(!singleTrackOnly)cout<<"Binomial intervals below are nominal: tracks in the same event can share hits.\n";
    if(responseTracks==0)cout<<"No selected crossings: response efficiencies are undefined, not zero.\n";

    // Independent canvases, no occupancy plot, no subdivided small pads.
    gStyle->SetOptStat(0);
    vector<pair<TCanvas*,string>> canvases;
    draw2D(hBars,"_candidate_bars",tag,canvases);
    TLine *diagonal=new TLine(-0.5,-0.5,31.5,31.5);
    diagonal->SetLineColor(kRed+1);diagonal->SetLineStyle(2);diagonal->Draw();
    // Same spatial bins and shared color scale in the matched/unmatched maps.
    const double mapMax=max({2.0,hMatched->GetMaximum(),hUnmatched->GetMaximum()});
    hMatched->SetMaximum(mapMax);hUnmatched->SetMaximum(mapMax);
    draw2D(hMatched,"_matched_xy",tag,canvases);
    draw2D(hUnmatched,"_unmatched_xy",tag,canvases);
    draw2D(hTimes,"_time_pairs",tag,canvases);
    draw2D(hTimes11,"_time_one_to_one",tag,canvases);

    TCanvas *cADC=canvas(tag+"_adc","ADC spectra for predicted crossings");
    hADCAll->SetLineColor(kBlack);hADCCut->SetLineColor(kRed+1);
    hADCAll->SetLineWidth(2);hADCCut->SetLineWidth(2);
    hADCAll->SetMinimum(0.5);hADCAll->SetMaximum(max(2.0,2.0*hADCAll->GetMaximum()));
    if(hADCAll->GetEntries()>0)cADC->SetLogy();
    hADCAll->Draw("HIST");hADCCut->Draw("HIST SAME");
    TLegend *adcLegend=new TLegend(0.48,0.72,0.94,0.87);
    adcLegend->AddEntry(hADCAll,"Predicted-bar ADC, no TDC cut","l");
    adcLegend->AddEntry(hADCCut,Form("Same-bar TDC in [%.1f, %.1f] ns",tdcCalMin,tdcCalMax),"l");
    adcLegend->Draw();canvases.push_back({cADC,"_adc_spectra"});

    TCanvas *cEff=canvas(tag+"_eff","Response fractions by predicted bar");
    TH1D *frame=new TH1D(tag+"_eff_frame",
        "Track response by predicted bar;Predicted bar;Response fraction",32,-0.5,31.5);
    keepHistogram(frame,owned);frame->SetMinimum(0);frame->SetMaximum(1.12);frame->Draw("AXIS");
    TGraphAsymmErrors *gADC=new TGraphAsymmErrors(),*gBoth=new TGraphAsymmErrors();
    for(int b=0;b<32;b++)if(denominator[b]>0){
        for(int k=0;k<2;k++){
            TGraphAsymmErrors *g=k?gBoth:gADC;
            const long long pass=k?bothPassed[b]:adcPassed[b];
            const double value=(double)pass/denominator[b];double low,high;
            interval(pass,denominator[b],low,high);
            const int point=g->GetN();g->SetPoint(point,b+(k?0.12:-0.12),value);
            g->SetPointError(point,0,0,value-low,high-value);
        }
    }
    gADC->SetMarkerStyle(20);gADC->SetMarkerColor(kBlue+1);gADC->SetLineColor(kBlue+1);
    gBoth->SetMarkerStyle(21);gBoth->SetMarkerColor(kRed+1);gBoth->SetLineColor(kRed+1);
    gADC->Draw("P SAME");gBoth->Draw("P SAME");
    TLegend *effLegend=new TLegend(0.40,0.72,0.94,0.87);
    effLegend->AddEntry(gADC,"ADC / predicted crossings","lp");
    effLegend->AddEntry(gBoth,"ADC + in-window TDC / predicted crossings","lp");
    effLegend->Draw();
    TLatex *note=new TLatex();note->SetNDC();note->SetTextSize(0.026);
    note->DrawLatex(0.13,0.91,"Nominal 68.3% binomial intervals; bars with no crossings omitted");
    canvases.push_back({cEff,"_response_by_bar"});

    TH1D *hGeometry=new TH1D(tag+"_geometry",
        "Geometric acceptance: exclusive track categories;Category;Tracks",4,0,4);
    keepHistogram(hGeometry,owned);
    const char *names[]={"Inside both","Fail x only","Fail y only","Fail both"};
    const long long counts[]={geo.inside,geo.failX,geo.failY,geo.failBoth};
    for(int i=0;i<4;i++){hGeometry->GetXaxis()->SetBinLabel(i+1,names[i]);hGeometry->SetBinContent(i+1,counts[i]);}
    TCanvas *cCuts=canvas(tag+"_geometry","Geometry cut summary");
    hGeometry->SetFillColor(kAzure-9);hGeometry->SetMaximum(max(1.0,1.2*hGeometry->GetMaximum()));
    gStyle->SetPaintTextFormat(".0f");hGeometry->Draw("HIST TEXT");canvases.push_back({cCuts,"_geometry_cuts"});
    for(auto &item:canvases){item.first->Modified();item.first->Update();}

    // Plot ranges are for display, never additional efficiency cuts.
    for(TH1 *h:array<TH1*,4>{{hADCAll,hADCCut,hTimes,hTimes11}}){
        const double outside=h->GetEntries()-h->Integral();
        if(outside>0)cout<<h->GetName()<<": "<<outside<<" entries outside displayed range (not discarded).\n";
    }
    if(savePlots){
        TString scope=selectedBar<0?"all":Form("bar%d",selectedBar);
        TString prefix=Form("gem_activeAna_%s_%s_tdc_%g_%g",
            scope.Data(),
            singleTrackOnly?"singleTrack":"allTracks",tdcCalMin,tdcCalMax);
        for(auto &item:canvases)item.first->SaveAs(prefix+item.second.c_str()+".png");
        ofstream events((prefix+"_tracks.csv").Data());
        ofstream bars((prefix+"_bars.csv").Data());
        if(events){
            events<<"entry,track,xProjected_m,yProjected_m,row,column,predictedBar,adcHits,tdcInWindow,passADC,passADC_TDC\n";
            events<<setprecision(12);
            for(const auto &r:records)events<<r.event<<','<<r.track<<','<<r.p.x<<','<<r.p.y<<','
                <<r.p.row<<','<<r.p.column<<','<<r.p.bar<<','<<r.adcHits<<','<<r.tdcHits<<','
                <<(r.adcHits>0)<<','<<(r.adcHits>0&&r.tdcHits>0)<<'\n';
        }
        if(bars){
            bars<<"bar,predictedCrossings,passADC,passADC_TDC,ADC_fraction,ADC_TDC_fraction\n";
            for(int b=0;b<32;b++){
                bars<<b<<','<<denominator[b]<<','<<adcPassed[b]<<','<<bothPassed[b]<<',';
                if(denominator[b])bars<<(double)adcPassed[b]/denominator[b]<<','<<(double)bothPassed[b]/denominator[b];
                else bars<<","; // No denominator: blank fractions, not zero.
                bars<<'\n';
            }
        }
        events.close();bars.close();
        if(!events||!bars)cerr<<"One or more CSV files could not be written completely.\n";
        else cout<<"Saved track and bar summaries with prefix "<<prefix<<'\n';
    }
    // Canvases own references to these histograms: keep them alive after return.
    // Detached from the input file, so closing that file does not erase plots.
    for(auto &h:owned)h.release();
    if(!savePlots)cout<<"Canvases remain open; no output files written. Fourth argument true saves.\n";
}
#endif

