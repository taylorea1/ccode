/*  void Test() {

    TFile *f = TFile::Open(
        "/w/halla-scshelf2102/sbs/taylorea/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","c",800,600);

    T->Draw("sbs.activeAna_adc.adccol>>hADC(10,0,10)");
    T->Draw("sbs.activeAna_tdc.tdccol>>hTDC(10,0,10)", "", "same");

    TH1 *hADC = (TH1*)gDirectory->Get("hADC");
    TH1 *hTDC = (TH1*)gDirectory->Get("hTDC");

    hADC->Scale(1.0 / hADC->Integral());
    hTDC->Scale(1.0 / hTDC->Integral());

    hADC->SetLineColor(kRed);
    hTDC->SetLineColor(kBlue);

    hADC->Draw("hist");
    hTDC->Draw("hist same");

    c->SaveAs("col.pdf");
} */

/* void Test() {

    TFile *f = TFile::Open(
        "/w/halla-scshelf2102/sbs/taylorea/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","ADC vs TDC correlation",800,600);

    // 2D histogram:
    T->Draw("sbs.activeAna_tdc.tdccol:sbs.activeAna_adc.adccol>>h2d(10,0,5,10,0,5)","","COLZ");

    TH2 *h2d = (TH2*)gDirectory->Get("h2d");

    h2d->SetTitle("TDC vs ADC Column Correlation;ADC adccol;TDC tdccol");

    h2d->Draw("COLZ");

    c->SaveAs("col_2d.pdf");
} */

/* void Test() {

    TFile *f = TFile::Open(
        "/w/halla-scshelf2102/sbs/taylorea/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","ADC row vs ADC col correlation",800,600);

    // Clear histogram explicitly created by ROOT
    TH2D *h2d = new TH2D("h2d",
        "ADC Row vs Column Correlation;ADC adccol;ADC adcrow",
        4, -0.5, 3.5,
        8, -0.5, 7.5);

    T->Draw("sbs.activeAna_adc.adcrow:sbs.activeAna_adc.adccol>>h2d","","COLZ");

    c->SaveAs("adcrowvscol_2d.pdf");
}*/

/* void Test() {

    TFile *f = TFile::Open(
        "/w/halla-scshelf2102/sbs/taylorea/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","TDC row vs TDC col correlation",800,600);

    // Clear histogram explicitly created by ROOT
    TH2D *h2d = new TH2D("h2d",
        "TDC Row vs TDC Col Correlation;TDC tdcrow;TDC tdccol",
        4, 0, 4,
        8, 0, 8);

    T->Draw("sbs.activeAna_tdc.tdcrow:sbs.activeAna_tdc.tdccol>>h2d","","COLZ");

    c->SaveAs("tdcrowvscol_2d.pdf");
} */

/* void Test() {

    TFile *f = TFile::Open(
        "/w/halla-scshelf2102/sbs/taylorea/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","ADC amp vs ADC trig correlation",800,600);

    // Clear histogram explicitly created by ROOT
    TH2D *h2d = new TH2D("h2d",
        "ADC amp vs AMP trig Correlation;ADC amp;AMP trig",
        20, 0, 800,
        20, 0, 800);

    T->Draw("sbs.activeAna_adc.a_amp_p:sbs.activeAna_adc.a_amptrig_p>>h2d","","COLZ");

    c->SaveAs("adcampvstrig.pdf");
} */

/*
 void Test() {

    TFile *f = TFile::Open(
        "/w/halla-scshelf2102/sbs/taylorea/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","ADC Multi vs ADC C correlation",800,600);

    // Clear histogram explicitly created by ROOT
    TH2D *h2d = new TH2D("h2d",
        "ADC Multi vs ADC C Correlation;ADC Multi;ADC C",
        25, 0, 25,
        40, 0, 40);

    T->Draw("Ndata.sbs.activeAna_adc.a_mult:Ndata.sbs.activeAna_adc.a_c>>h2d","","COLZ");

    c->SaveAs("Ndatamultivc.pdf");
}
*/

/* void Test() {

    TFile *f = TFile::Open(
        "/w/halla-scshelf2102/sbs/taylorea/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","ADC mult vs ADC c correlation",800,600);

    // Clear histogram explicitly created by ROOT
    TH2D *h2d = new TH2D("h2d",
        "ADC C vs AMP Mult Correlation;ADC mult;AMP C",
        40, -0.2, 1.2,
        40, -100, 450);

    T->Draw("sbs.activeAna_adc.a_c:sbs.activeAna_adc.a_mult>>h2d","","COLZ");

    c->SaveAs("adcmultvsc2.pdf");
}*/

void Test() {

    TFile *f = TFile::Open(
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    TCanvas *c = new TCanvas("c","ADC Amp vs Amp_C",800,600);

    TH2D *h2d = new TH2D(
        "h2d",
        "ADC Amp vs ADC Amp_C;ADC Amp;ADC Amp_C",
        1000, 0, 400,   // X-axis bins/range
        1000, 0, 400     // Y-axis bins/range
    );

    T->Draw(
        "sbs.activeAna_adc.a_amp:sbs.activeAna_adc.a_amp_c>>h2d",
        "",
        "COLZ"
    );

    c->SaveAs("adcamp_vs_ampc.pdf");
}
