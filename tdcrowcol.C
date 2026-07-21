void tdcrowcol (int ch)
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
    cout << "Processing channel " << ch << endl;

    TH1D *h = new TH1D(Form("h%d", ch),
                       Form("TDC Row Column %d;Column", ch),
                       100, 0, 7.5);
    int point = 0;
    Long64_t n = T->GetEntries(); //Long64_t is a data type
    Double_t adcrow [1000];
    Double_t adccol [1000];
    Int_t adcNdata;
    Int_t tdcNdata;
    Double_t tdccol[1000];
    T->SetBranchAddress("sbs.activeAna_adc.adcrow", &adcrow);                   
    T->SetBranchAddress("sbs.activeAna_adc.adccol", &adccol);                   
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcrow", &adcNdata); 
    T->SetBranchAddress("Ndata.sbs.activeAna_tdc.tdccol", &tdcNdata);
    T->SetBranchAddress("sbs.activeAna_tdc.tdccol", &tdccol);
    for (Long64_t i = 0; i < n; i++) {
        T->GetEntry(i);
        if (adcNdata == 0 && tdcNdata == 0) {
            continue; // Skip this entry to avoid division by zero
        }
        
         for (int tj = 0; tj < tdcNdata; tj++) {
               if (tdccol[tj] != ch) {
                continue;}
                for (int j = 0; j <adcNdata; j++){
                    if (adccol[j] == tdccol[tj]) {
                        h->Fill(adcrow[j]);
                    }
                }
            }
        }
         
    cout << "Channel " << ch << ": Points = " << h->GetEntries() << endl;
    TCanvas *c = new TCanvas(Form("c%d", ch), Form("TDC Row Column %d", ch), 800, 600);
    h->Draw();
    // double total = T->GetEntries();
    // double selected = h->GetEntries();
   //  double percentage = (selected / total) * 100.0;

    TLatex *latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.03);
    latex->DrawLatex(0.7, 0.85, Form("Selected: %.0f / %.0f (%.2f%%)", h->GetEntries(), n, (h->GetEntries() / n) * 100.0));
    c->SaveAs(Form("tdc_row_col_%02d.png", ch));
}