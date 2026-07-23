//  void thresholdadc(int targetCol, double threshold)
// {
//     TFile *f = TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");

//     if (!f || f->IsZombie()) {
//         cout << "Error opening file!" << endl;
//         return;
//     }

//     TTree *T = (TTree*)f->Get("T");

//     if (!T) {
//         cout << "Error: TTree not found!" << endl;
//         return;
//     }

//     Double_t row[1000];
//     Double_t col[1000];
//     Double_t amp[1000];
//     Int_t Ndata;

//     T->SetBranchAddress("sbs.activeAna_adc.adcrow", row);
//     T->SetBranchAddress("sbs.activeAna_adc.adccol", col);
//     T->SetBranchAddress("sbs.activeAna_adc.a_amp", amp);
//     T->SetBranchAddress("Ndata.sbs.activeAna_adc.adccol", &Ndata);

//     TH2D *h = new TH2D(Form("h_col%d", targetCol),
//                        Form("Column %d;Row;Counts", targetCol),
//                        10, -0.5, 8.5, 100, 0, 200);

//     Long64_t n = T->GetEntries();

//     for (Long64_t i = 0; i < n; i++) {

//         T->GetEntry(i);

//         for (int j = 0; j < Ndata; j++) {

//             if ((int)col[j] != targetCol)
//                 continue;

//             if (amp[j] < threshold)
//                 continue;

//             h->Fill(row[j], amp[j]);
//         }
//     }
// cout << "Column " << targetCol << ": Points = " << h->GetEntries() << endl;
//     TCanvas *c = new TCanvas(Form("c%d", targetCol),
//                              Form("Column %d", targetCol),
//                              800, 600);

//     h->SetStats(0);
//     h->Draw();
//     c->SaveAs(Form("threshold_adc_col_%02d.png", targetCol));
// } 


void thresholdadc(int targetCol, int targetRow, double threshold)
{
    //  if (targetRow < 0 || targetRow > 3 ||
    //      targetCol < 0 || targetCol > 7) {
    //      cout << "Invalid channel. Rows: 0-3, Columns: 0-7" << endl;
    //      return;
    // }
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
    Double_t amp[1000];
    Int_t Ndata;

    T->SetBranchAddress("sbs.activeAna_adc.adcrow", row);
    T->SetBranchAddress("sbs.activeAna_adc.adccol", col);

    // ADC amplitude
    T->SetBranchAddress("sbs.activeAna_adc.a", amp);

    // Number of hits in the event
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcrow", &Ndata);

    TH1D *h = new TH1D(Form("h_r%d_c%d", targetRow, targetCol),
                   Form("ADC Amplitude Row %d Column %d;ADC", targetRow, targetCol),
                   100, 0, 1000);

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

            // ADC threshold cut
            if (amp[j] < threshold)
                continue;

            // x = event number
            // y = ADC amplitude
            h->Fill(amp[j]);
            g->SetPoint(point, i, amp[j]);
            point++;
        }
    }

    cout << "Row " << targetRow
         << " Column " << targetCol
         << " Hits found: " << point << endl;


    if (point == 0) {
        cout << "No hits passed the threshold." << endl;
        return;
    }


TCanvas *c = new TCanvas("c","ADC Scatter",900,600);

g->SetTitle(Form("ADC Scatter Row %d Column %d;Event Number;ADC",
                 targetCol,targetRow));

g->SetMarkerStyle(20);
g->SetMarkerColor(kBlue);

g->Draw("AP");

TLegend *leg = new TLegend(0.65,0.75,0.88,0.88);

// leg->AddEntry(g,
//               Form("Column %d Row %d",targetCol,targetRow),
//               "p");

leg->AddEntry((TObject*)0,
              Form("Hits = %d",point),
              "");

leg->SetBorderSize(0);
leg->Draw();

    c->SaveAs(Form("scatter_r%d_c%d.png",targetCol,targetRow));

    TCanvas *c2 = new TCanvas("c2","ADC Histogram",900,600);
    c2->SetLogy();
    h->Draw();
    c2->SaveAs(Form("histogram_r%d_c%d.png",targetCol,targetRow));
    
}