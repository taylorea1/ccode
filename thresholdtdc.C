void thresholdtdc(double threshold)
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


    Double_t tdcrow[1000];
    Double_t tdccol[1000];
    Double_t tdc[1000];
    Int_t Ndata;


    T->SetBranchAddress("sbs.activeAna_tdc.tdc", tdc);
    T->SetBranchAddress("sbs.activeAna_tdc.tdcrow", tdcrow);
    T->SetBranchAddress("sbs.activeAna_tdc.tdccol", tdccol);
    T->SetBranchAddress("Ndata.sbs.activeAna_tdc.tdcrow", &Ndata);


    // Total TDC hits per channel
    TH1D *h_channel = new TH1D(
        "h_channel",
        "TDC Hits Per Channel;Channel (row*4+col);Total TDC Hits",
        32,
        -1.5,
        31.5
    );


    // Multiplicity distribution (hits per event across detector)
    TH1D *h_mult = new TH1D(
        "h_mult",
        "TDC Multiplicity Per Event;Number of TDC hits;Events",
        60,
        -0.5,
        59.5
    );


    Long64_t nentries = T->GetEntries();

    int totalHits = 0;
    int eventsWithHit = 0;


    for (Long64_t i = 0; i < nentries; i++) {

        T->GetEntry(i);

        int eventHits = 0;


        for (int t = 0; t < Ndata; t++) {


            // Apply TDC threshold
            if (tdc[t] < threshold)
                continue;


            int row = (int)tdcrow[t];
            int col = (int)tdccol[t];


            // Ignore invalid channels
            if (row < 0 || row > 7)
                continue;

            if (col < 0 || col > 3)
                continue;


            int channel = row*4 + col;


            // Count hit for this channel
            h_channel->Fill(channel);


            totalHits++;
            eventHits++;

        }


        if (eventHits > 0)
            eventsWithHit++;


        h_mult->Fill(eventHits);

    }


    cout << "==============================" << endl;
    cout << "Total events: " << nentries << endl;
    cout << "Events with TDC hits: " << eventsWithHit << endl;
    cout << "Total TDC hits: " << totalHits << endl;
    cout << "==============================" << endl;



    // Save channel hit counts to CSV
    ofstream outfile("tdc_channel_counts.csv");

    outfile << "Channel,Row,Column,Hits\n";

    cout << "\nChannel hit counts:\n";

    for (int c = 0; c < 32; c++) {

        int row = c / 4;
        int col = c % 4;

        double hits = h_channel->GetBinContent(c+1);


        cout << "Channel "
             << c
             << "  Row=" << row
             << "  Col=" << col
             << "  Hits=" << hits
             << endl;


        outfile << c << ","
                << row << ","
                << col << ","
                << hits << "\n";
    }

    outfile.close();

    cout << "Saved channel counts to tdc_channel_counts.csv" << endl;



    // Draw channel hit distribution
    TCanvas *c1 = new TCanvas("c1","TDC Hits Per Channel",900,600);

    h_channel->Draw("hist");

    c1->SaveAs("tdc_hits_all_channels.pdf");



    // Draw multiplicity distribution
    TCanvas *c2 = new TCanvas("c2","Multiplicity",900,600);

    h_mult->Draw("hist");

    c2->SaveAs("tdc_multiplicity_all_channels.pdf");

}