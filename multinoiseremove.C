/* void multinoiseremove()
{
  TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
  TTree *T = (TTree*)f->Get("T");
  TCanvas *c = new TCanvas("c", "amp_1D", 800, 600);
  TH1D *h1 = new TH1D("h1", "Raw ADC Amp;ADC Amp", 100, 22, 74);
  T->Draw("sbs.activeAna_adc.a_amp>>h1");
  h1->SetLineColor(kBlue);
  h1->SetlineWidth(2);
  c->cd();
  h1->Draw(COLZ);
  c->SaveAs("adc_amp1.pdf");
}
{
    TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
    TTree *T = (TTree*)f->Get("T");
    TCanvas *c = new TCanvas("c", "a_amp_c", 800, 600);
    TH1D *h1 = new TH1D("h1", "Corrected ADC Amp;ADC Amp;Counts", 100, 14.8, 34.8);
    T->Draw("sbs.activeAna_adc.a_amp_c>>h1");
    h1->SetLineColor(kRed);
    h1->SetlineWidth(2);
    c->cd();
    h1->Draw(COLZ);
    c->SaveAs("adc_ampc1.pdf");
}
{
    TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
    TTree *T = (TTree*)f->Get("T");
    TCanvas *c = new TCanvas("c", "Raw vs Corrected", 800, 600);
    TH1D *h_raw = new TH1D("h_raw", "Raw vs Corrected;ADC Amp;Counts", 100, 31, 41);
    TH1D *h_corr = new TH1D("h_corr", "Raw vs Corrected;ADC Amp;Counts", 100, 31, 41);
    TH2D *h2 = new TH2D("h2", "Raw vs Corrected ADC Amp;Raw ADC Amp;Corrected ADC Amp", 100, 20, 800, 100, 10, 400);
    T->Draw("sbs.activeAna_adc.a_amp:sbs.activeAna_adc.a_amp_c>>h2", "sbs.activeAna_adc.a_amp<25 && sbs.activeAna_adc.a_amp_c<35", "COLZ");
    T->Draw("sbs.activeAna_adc.a_amp>>h_raw", " ", "goff");
    T->Draw("sbs.activeAna_adc.a_amp_c>>h_corr", " ", "goff");
    T->Draw("sbs.activeAna_adc.a_amp>>h_raw","sbs.activeAna_adc.a_amp<27", "goff");
    T->Draw("sbs.activeAna_adc.a_amp_c>>h_corr","sbs.activeAna_adc.a_amp_c>56.9", "goff");
    h_raw->SetLineColor(kBlue);
    h_raw->SetLineWidth(2);
    h_corr->SetLineColor(kRed);
    h_corr->SetLineWidth(2);
    double ymax = std::max(h2->GetMaximum(), h2->GetMaximum());
    h2->SetMaximum(ymax * 1.1);
    h2->Draw("COLZ");
    h_corr->Draw("HIST SAME");
    TLegend *leg = new TLegend(0.65, 0.75, 0.88, 0.88);
    leg->AddEntry(h_raw, "a_amp", "l");
    leg->AddEntry(h_corr, "a_amp_c", "l");
    leg->Draw();
    //h1->SetLineColor(kRed);
    //h1->SetlineWidth(2);
    c->cd();
    //h1->Draw(COLZ);
    c->SaveAs("removednoise.pdf");
}
{
    TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
    if (!f || f->IsZombie()) {
        printf("Error: Could not open the ROOT file.\n");
        return;
    }
    TTree *T = (TTree*)f->Get("T");
    if (!T) {
        printf("Error: Could not find TTree 'T' in the file.\n");
        f->Close();
        return;
    }
    TCanvas *c = new TCanvas("c", "Raw vs Corrected", 800, 600);
    TH2D *h2 = new TH2D("h2", "Raw vs Corrected ADC Amp;Raw ADC Amp;Corrected ADC Amp", 1000, 27, 56.9, 1000, 10, 50);
    T->Draw("sbs.activeAna_adc.a_amp_c:sbs.activeAna_adc.a_amp>>h2"," ", "COLZ");
    h2->Draw("COLZ");
    c->SaveAs("2damp_ampc.pdf");
} */

void multinoiseremove()
{
    TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");

    if (!f || f->IsZombie()) {
        cout << "Error opening file!" << endl;
        return;
    }

    TTree *T = (TTree*)f->Get("T");

    if (!T) {
        cout << "Error: TTree not found!" << endl;
        return;
    }

    cout << "Entries = " << T->GetEntries() << endl;

    // Uncomment these if the plots are still blank
    // T->Print();
    // T->Show(0);

    //---------------------------------------------
    // Raw ADC
    //---------------------------------------------
    TCanvas *c1 = new TCanvas("c1","Raw ADC",800,600);

    TH1D *hRaw = new TH1D("hRaw",
                          "Raw ADC Amplitude;Raw ADC Amp;Counts",
                          150,20,75.4);

    Long64_t nRaw =
        T->Draw("sbs.activeAna_adc.a_amp>>hRaw","","goff");

    cout << "Raw entries drawn = " << nRaw << endl;
    cout << "Histogram entries = " << hRaw->GetEntries() << endl;

    hRaw->SetLineColor(kBlue);
    hRaw->SetLineWidth(2);

    c1->cd();
    hRaw->Draw("HIST");
    c1->Modified();
    c1->Update();

    //---------------------------------------------
    // Corrected ADC
    //---------------------------------------------
    TCanvas *c2 = new TCanvas("c2","Corrected ADC",800,600);

    TH1D *hCorr = new TH1D("hCorr",
                           "Corrected ADC Amplitude;Corrected ADC Amp;Counts",
                           150,15,35);

    Long64_t nCorr =
        T->Draw("sbs.activeAna_adc.a_amp_c>>hCorr","","goff");

    cout << "Corrected entries drawn = " << nCorr << endl;
    cout << "Histogram entries = " << hCorr->GetEntries() << endl;

    hCorr->SetLineColor(kRed);
    hCorr->SetLineWidth(2);

    c2->cd();
    hCorr->Draw("HIST");
    c2->Modified();
    c2->Update();

    //---------------------------------------------
    // Overlay
    //---------------------------------------------
    TCanvas *c3 = new TCanvas("c3","Overlay",800,600);

    TH1D *hRawO  = new TH1D("hRawO","Raw vs Corrected;Amplitude;Counts",
                            80,17,50);

    TH1D *hCorrO = new TH1D("hCorrO","",
                            80,15,50);

    T->Draw("sbs.activeAna_adc.a_amp>>hRawO","","goff");
    T->Draw("sbs.activeAna_adc.a_amp_c>>hCorrO","","goff");

    hRawO->SetLineColor(kBlue);
    hRawO->SetLineWidth(2);

    hCorrO->SetLineColor(kRed);
    hCorrO->SetLineWidth(2);

    c3->cd();

    hRawO->Draw("HIST");
    hCorrO->Draw("HIST SAME");

    TLegend *leg = new TLegend(0.65,0.75,0.88,0.88);
    leg->AddEntry(hRawO,"Raw","l");
    leg->AddEntry(hCorrO,"Corrected","l");
    leg->Draw();

    c3->Modified();
    c3->Update();

    //---------------------------------------------
    // 2D Correlation
    //---------------------------------------------
   TCanvas *c4 = new TCanvas("c4","2D Correlation",800,600);

    TH2D *h2 = new TH2D("h2", "Corrected vs Raw ADC;Raw ADC Amp;Corrected ADC Amp", 100,30,60, 100,15,25);

    Long64_t n2 =
        T->Draw("sbs.activeAna_adc.a_amp_c:sbs.activeAna_adc.a_amp>>h2", "", "goff");

    cout << "2D entries = " << n2 << endl;

    c4->cd();
    h2->Draw("COLZ");
    c4->Modified();
    c4->Update();

    //---------------------------------------------
    // Save
    //---------------------------------------------
    c1->SaveAs("adc_amp2.pdf");
    c2->SaveAs("adc_ampc2.pdf");
    c3->SaveAs("removednoise1.pdf");
    c4->SaveAs("2damp_ampc2.pdf");
}