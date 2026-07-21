void rowcoladc(int ch)
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
        return;
    }
    cout << "Entries = " << T->GetEntries() << endl;
    cout << "Processing channel " << ch << endl;

    TH1D *h = new TH1D(Form("h%d", ch),
                       Form("ADC Row Column %d;Column", ch),
                       100, 0, 10);
    int point = 0;
    Long64_t n = T->GetEntries(); //Long64_t is a data type large enough to hold up to 9 quintillion (9 x 10^18) entries.
    Double_t row [1000];
    Double_t col [1000];
    Int_t Ndata;
    T->SetBranchAddress("sbs.activeAna_adc.adcrow", &row);                   
    T->SetBranchAddress("sbs.activeAna_adc.adccol", &col);                   
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcrow", &Ndata); 
    for (Long64_t i = 0; i < n; i++) {
        T->GetEntry(i);
        if (Ndata == 0) {
            continue; // Skip this entry to avoid division by zero
        }
        for (int j = 0; j < Ndata; j++) {
            if (row[j] == ch) {
                h->Fill(col[j]);
            }
        } 
    }
    cout << "Channel " << ch << ": Points = " << h->GetEntries() << endl;
    TCanvas *c = new TCanvas(Form("c%d", ch),
                             Form("Channel %d", ch),
                             800, 600);
    h->Draw(); // A = axes, P = points
                      
}