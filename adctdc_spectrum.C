#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TString.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>

#include <algorithm>
#include <cmath>
#include <iostream>


void adctdc_spectrum(int selectedBar=0, double tdcMin=500.0,double tdcMax=520.0)
{
    TFile *f=TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
    if(!f||f->IsZombie()){
        cout<<"Error opening file!"<<endl;
        return;
    }

    TTree *T=(TTree*)f->Get("T");
    if(!T){
        cout<<"Error: TTree not found!"<<endl;
        f->Close();
        return;
    }

    if(selectedBar<0||selectedBar>31){
    cout<<"Error: selectedBar must be between 0 and 31"<<endl;
    return;
}

    TTreeReader reader(T);
    TTreeReaderArray<Double_t> adc(reader,"sbs.activeAna_adc.a_p");
    TTreeReaderArray<Double_t> adcBar(reader,"sbs.activeAna_adc.adcelemID");
    TTreeReaderArray<Double_t> tdc(reader,"sbs.activeAna_tdc.hits.t");
    TTreeReaderArray<Double_t> tdcBar(reader,"sbs.activeAna_tdc.hits.TDCelemID");

    TH1D *hADCALL=new TH1D("hADCALL","Active Analyzer ADC;ADC pulse integral a_{p};Entries",200,-0.5,200.5);
    TH1D *hADCTime=new TH1D("hADCTime","ADC with same-bar in-time TDC;ADC pulse integral a_{p};Entries",200,-0.5,200.5);
    TH1D *hTDC=new TH1D("hTDC","Valid Active Analyzer TDC Times;TDC time;Entries",100,0,1200);
    TH2D *hADCBarAll=new TH2D("hADCBarAll","ADC versus Bar: No TDC Constraint;Bar index;ADC pulse integral a_{p}",32,-0.5,31.5,100,0,4000);
    TH2D *hADCBarTime=new TH2D("hADCBarTime","ADC versus Bar: Same-bar in-time TDC;Bar index;ADC pulse integral a_{p}",32,-0.5,31.5,100,0,4000);

    hADCALL->SetTitle(Form("Active Analyzer Bar %d ADC;ADC pulse integral a_{p};Entries",selectedBar));
    hADCTime->SetTitle(Form("Bar %d ADC with in-time TDC;ADC pulse integral a_{p};Entries",selectedBar));
    hTDC->SetTitle(Form("Active Analyzer Bar %d TDC Times;TDC time;Entries",selectedBar));

    Long64_t eventsRead=0;
    Long64_t adcEntries=0;
    Long64_t adcEntriesPassing=0;

    while(reader.Next()){
        ++eventsRead;

        //Fill the full TDC spectrum while ignoring invalid values
        const size_t nTDC=tdc.GetSize()<tdcBar.GetSize()?tdc.GetSize():tdcBar.GetSize();

        for(size_t it=0;it<nTDC;++it){
            const double time=tdc[it];
            const int hitBar=static_cast<int>(std::lround(tdcBar[it]));

            if(hitBar==selectedBar&&std::isfinite(time)&&time>=-1200.0&&time<=1200.0){
                hTDC->Fill(time);
            }
        }

        const size_t nADC=adc.GetSize()<adcBar.GetSize()?adc.GetSize():adcBar.GetSize();
        // const size_t nTDC=tdc.GetSize()<tdcBar.GetSize()?tdc.GetSize():tdcBar.GetSize();

        for(size_t ia=0;ia<nADC;++ia){
            const double pulse=adc[ia];
            const int bar=static_cast<int>(lround(adcBar[ia]));

            if(!isfinite(pulse)||bar!=selectedBar){
                    continue;
                }

            ++adcEntries;
            hADCALL->Fill(pulse);
            hADCBarAll->Fill(bar,pulse);

            bool sameBarInTime=false;

            for(size_t it=0;it<nTDC;++it){
                const double time=tdc[it];
                const int hitBar=static_cast<int>(lround(tdcBar[it]));

                if(hitBar==bar&&isfinite(time)&&time>=tdcMin&&time<=tdcMax){
                    sameBarInTime=true;
                    break;
                }
            }

            if(sameBarInTime){
                ++adcEntriesPassing;
                hADCTime->Fill(pulse);
                hADCBarTime->Fill(bar,pulse);
            }
        }
    }

    gStyle->SetOptStat(1110);

    TCanvas *cADC=new TCanvas("cADC","ADC Spectrum Comparison",1000,750);
    cADC->SetLogy();
    hADCALL->SetLineColor(kBlack);
    hADCALL->SetLineWidth(2);
    hADCTime->SetLineColor(kRed+1);
    hADCTime->SetLineWidth(2);
    hADCALL->Draw("HIST");
    hADCTime->Draw("HIST SAME");

    TLegend *leg=new TLegend(0.53,0.73,0.88,0.88);
    leg->AddEntry(hADCALL,"No TDC Constraint","l");
    leg->AddEntry(hADCTime,Form("Same bar, %.1f #leq TDC #leq %.1f",tdcMin,tdcMax),"l");
    leg->Draw();
    cADC->SaveAs(Form("activeAna_bar%d_adc_comparison.png",selectedBar));

    TCanvas *cTDC=new TCanvas("cTDC","TDC Timing Spectrum",1000,750);
    hTDC->SetLineWidth(2);
    hTDC->Draw("HIST");
    cTDC->SaveAs(Form("activeAna_bar%d_tdc_spectrum.png",selectedBar));

    TCanvas *cBars=new TCanvas("cBars","ADC versus Bar",1400,650);
    cBars->Divide(2,1);
    cBars->cd(1);
    hADCBarAll->Draw("COLZ");
    cBars->cd(2);
    hADCBarTime->Draw("COLZ");
    cBars->SaveAs("activeAna_adc_vs_bar.png");

    TFile output("activeAna_adc_tdc_spectra.root","RECREATE");
    hADCALL->Write();
    hADCTime->Write();
    hTDC->Write();
    hADCBarAll->Write();
    hADCBarTime->Write();
    output.Close();

    cout<<"Events read: "<<eventsRead<<'\n'
        <<"Valid ADC entries: "<<adcEntries<<'\n'
        <<"ADC entries passing same-bar TDC cut: "<<adcEntriesPassing<<'\n';

    if(adcEntries>0){
        cout<<"Passing fraction: "<<100.0*adcEntriesPassing/adcEntries<<"%\n";
    }

    cout<<"TDC window: ["<<tdcMin<<", "<<tdcMax<<"]\n";

    f->Close();
}