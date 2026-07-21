void elemid (int ch) 
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
//Displays the number of entries in the TTree.
    cout << "Entries = " << T->GetEntries() << endl;

//Inserting a number to pick a specific channel to display the histogram for that channel.
        cout << "Processing channel " << ch << endl;

        // Create a scatter plot graph for this channel
        TH1D *h = new TH1D(Form("h%d", ch),
                           Form("ADC Amplitude For Channel %d;Amplitude", ch),
                           100, 0, 1000);
        // Fill graph manually (correct way)
        int point = 0;
        Long64_t n = T->GetEntries(); //Long64_t is a data type large enough to hold up to 9 quintillion (9 x 10^18) entries.
       Double_t adcelemID [1000];
       Int_t Ndata;
         Double_t a_amp[1000];
        T->SetBranchAddress("sbs.activeAna_adc.adcelemID", &adcelemID);
        T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcelemID", &Ndata);
        T->SetBranchAddress("sbs.activeAna_adc.a_amp", &a_amp);

        for (Long64_t i = 0; i < n; i++) {
            T->GetEntry(i);
            if (Ndata == 0) {
                // cout << "Warning: Ndata is zero for entry " << i << endl;
                continue; // Skip this entry to avoid division by zero
            }
            for (int j = 0; j < Ndata; j++) {
                if (adcelemID[j] == ch) {
                   h->Fill(a_amp[j]);
                }
            } 
        }

        //Displays the number of points in the graph for the current channel.
        cout << "Channel " << ch << ": Points = " << h->GetEntries() << endl;

       // h->SetTitle(Form("ADC Amplitude For Channel %d;Amplitude", ch));
        TCanvas *c = new TCanvas(Form("c%d", ch),
                                 Form("Channel %d", ch),
                                 800, 600);
        h->Draw(); // A = axes, P = points

        // Saving the file and categorizing the graphs by channel number
        c->SaveAs(Form("channel_%02d.png", ch));
    
}