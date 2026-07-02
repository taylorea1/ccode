 void tdcelemid(int ch)
{
    TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
//If it cannot open the file or if the file is corrupted, it will print an error message and return.
    if (!f || f->IsZombie()) {
        cout << "Error opening file!" << endl;
        return;
    }
    TTree *T = (TTree*)f->Get("T");
    if (!T) {
        cout << "Error: TTree not found!" << endl;
        return;   }
     cout << "Entries = " << T->GetEntries() << endl;
     cout << "Processing Channel " << ch << endl;

     TH1D *h = new TH1D(Form("h%d", ch),
                           Form("TDC Amplitude For Channel %d;Amplitude", ch),
                           100, 0, 1000);
        int point = 0;
        Long64_t n = T->GetEntries(); 
        Double_t adcelemID [1000];
        Int_t Ndata;
        Int_t tdcNdata;
        Double_t a_amp[1000];
        Double_t tdcelemID[1000];

        T->SetBranchAddress("sbs.activeAna_adc.adcelemID", &adcelemID);
        T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcelemID", &Ndata);
        T->SetBranchAddress("sbs.activeAna_adc.a_amp", &a_amp);
        T->SetBranchAddress("sbs.activeAna_tdc.tdcelemID", &tdcelemID);
        T->SetBranchAddress("Ndata.sbs.activeAna_tdc.tdcelemID", &tdcNdata);

        for (Long64_t i = 0; i < n; i++) {
            T->GetEntry(i);
            if (Ndata == 0 && tdcNdata == 0) {
                continue; // Skip this entry to avoid division by zero
            }
            
            for (int tj = 0; tj < tdcNdata; tj++) {
               if (tdcelemID[tj] != ch) {
                continue;}
                for (int j = 0; j <Ndata; j++){
                    if (adcelemID[j] == tdcelemID[tj]) {
                        h->Fill(a_amp[j]);
                    }
                }
            }
        }
        cout << "Channel " << ch << ": Points = " << h->GetEntries() << endl;

        TCanvas *c = new TCanvas(Form("c%d", ch),
                                 Form("Channel %d", ch),
                                 800, 600);
        h->SetMaximum(1.2 * h->GetMaximum()); // Set the maximum of the y-axis to 1.2 times the maximum bin content
        h->Draw(); 

        double total = T->GetEntries();
        double selected = h->GetEntries();
        double percentage = (selected / total) * 100.0;





// Draw in data coordinates (do NOT call SetNDC())

        TLatex *t = new TLatex();
        t->SetNDC();
        t->SetTextSize(0.03);
        t->DrawLatex(0.7, 0.85, Form("Selected: %.0f / %.0f (%.2f%%)", selected, total, percentage));

        c->SaveAs(Form("t_by_a_channel_%02d.png", ch));
}

/*
void tdcelemid(int ch)
{
    TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
//If it cannot open the file or if the file is corrupted, it will print an error message and return.
    if (!f || f->IsZombie()) {
        cout << "Error opening file!" << endl;
        return;
    }
    TTree *T = (TTree*)f->Get("T");
    if (!T) {
        cout << "Error: TTree not found!" << endl;
        return;   }
     cout << "Entries = " << T->GetEntries() << endl;
     cout << "Processing Channel " << ch << endl;

     TH1D *h = new TH1D(Form("h%d", ch),
                           Form("TDC Amplitude For Channel %d;Amplitude", ch),
                           100, 0, 1000);
        int point = 0;
        Long64_t n = T->GetEntries(); 
        Double_t adcelemID [1000];
        Int_t Ndata;
        Int_t tdcNdata;
        Double_t a_amp_c[1000];
        Double_t tdcelemID[1000];

        T->SetBranchAddress("sbs.activeAna_adc.adcelemID", &adcelemID);
        T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcelemID", &Ndata);
        T->SetBranchAddress("sbs.activeAna_adc.a_amp_c", &a_amp_c);
        T->SetBranchAddress("sbs.activeAna_tdc.tdcelemID", &tdcelemID);
        T->SetBranchAddress("Ndata.sbs.activeAna_tdc.tdcelemID", &tdcNdata);

        for (Long64_t i = 0; i < n; i++) {
            T->GetEntry(i);
            if (Ndata == 0 && tdcNdata == 0) {
                continue; // Skip this entry to avoid division by zero
            }
            
            for (int tj = 0; tj < tdcNdata; tj++) {
               if (tdcelemID[tj] != ch) {
                continue;}
                for (int j = 0; j <Ndata; j++){
                    if (adcelemID[j] == tdcelemID[tj]) {
                        h->Fill(a_amp_c[j]);
                    }
                }
            }
        }
        cout << "Channel " << ch << ": Points = " << h->GetEntries() << endl;

        TCanvas *c = new TCanvas(Form("c%d", ch),
                                 Form("Channel %d", ch),
                                 800, 600);
        h->SetMaximum(1.2 * h->GetMaximum()); // Set the maximum of the y-axis to 1.2 times the maximum bin content
        h->Draw(); 
        double total = T->GetEntries();
        double selected = h->GetEntries();
        double percentage = (selected / total) * 100.0;


// Draw in data coordinates (do NOT call SetNDC())

       TLatex *t = new TLatex();
        t->SetNDC();
        t->SetTextSize(0.03);
        t->DrawLatex(0.7, 0.85, Form("Selected: %.0f / %.0f (%.2f%%)", selected, total, percentage));

        c->SaveAs(Form("t_by_a_c_channel_%02d.png", ch));
} */