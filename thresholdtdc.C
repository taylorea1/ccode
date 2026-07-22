void thresholdtdc(int targetCol, int targetRow, double threshold)
{
    if (targetRow < 0 || targetRow > 3 ||
        targetCol < 0 || targetCol > 7) {
        cout << "Invalid channel. Rows: 0-3, Columns: 0-7" << endl;
        return;
    }

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

    Double_t row[1000];
    Double_t col[1000];
    Double_t tdc[1000];
    Int_t Ndata;

    T->SetBranchAddress("sbs.activeAna_adc.adcrow", row);
    T->SetBranchAddress("sbs.activeAna_adc.adccol", col);
    T->SetBranchAddress("sbs.activeAna_tdc.tdc", tdc);
    T->SetBranchAddress("Ndata.sbs.activeAna_tdc.tdcrow", &Ndata);


 TGraph *g = new TGraph();
    int point = 0;

    Long64_t nentries = T->GetEntries();

    for (Long64_t i = 0; i < nentries; i++) {

        T->GetEntry(i);

        for (int j = 0; j < Ndata; j++) {

            // Select ONLY this row and column
            if ((int)col[j] != targetCol)
                continue;

            if ((int)row[j] != targetRow)
                continue;

            // TDC threshold cut
            if (tdc[j] < threshold)
                continue;

            // x = event number
            // y = TDC amplitude
            g->SetPoint(point++, i, tdc[j]);
        }
    }

    cout << "Row " << targetRow
         << " Column " << targetCol
         << " Hits found: " << point << endl;


    if (point == 0) {
        cout << "No hits passed the threshold." << endl;
        return;
    }


   TCanvas *c = new TCanvas("c","TDC Scatter",900,600);

g->SetTitle(Form("TDC Scatter Row %d Column %d;Event Number;TDC",
                 targetCol,targetRow));

g->SetMarkerStyle(20);
g->SetMarkerColor(kBlue);

g->Draw("AP");

TLegend *leg = new TLegend(0.65,0.75,0.88,0.88);

// leg->AddEntry(g,
//               Form("Column %d Row %d",targetCol,targetRow),
//               "p");

leg->AddEntry((TObject*)0, Form("Hits = %d", point), "");

leg->SetBorderSize(0);
leg->Draw();

c->SaveAs(Form("tdc_scatter_r%d_c%d.png", targetCol, targetRow));
}   