void thresholdadc_tdc(int targetRow, int targetCol, double threshold)
{
    // ============================================================
    // Open ROOT file containing active analyzer data
    // ============================================================

    // if (targetRow < 0 || targetRow > 7 ||
    //     targetCol < 0 || targetCol > 3) {
    //     cout << "Invalid channel. Rows: 0-7, Columns: 0-3" << endl;
    //     return;
    // }

    TFile *f = TFile::Open(
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");

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
    Double_t tdcrow[1000];
    Double_t tdccol[1000];
    Double_t elemID[1000];
    Double_t tdcelemID[1000];
    Int_t Ndata;
    Int_t tdcNdata;

    T->SetBranchAddress( "sbs.activeAna_adc.adcrow",row);
    T->SetBranchAddress("sbs.activeAna_adc.adccol",col);
    T->SetBranchAddress( "sbs.activeAna_adc.a",amp);
    T->SetBranchAddress( "sbs.activeAna_tdc.tdcrow",tdcrow);
    T->SetBranchAddress("sbs.activeAna_tdc.tdccol",tdccol);
    T->SetBranchAddress( "Ndata.sbs.activeAna_adc.adcrow",&Ndata);
    T->SetBranchAddress( "Ndata.sbs.activeAna_tdc.tdcrow", &tdcNdata);
    T->SetBranchAddress("sbs.activeAna_adc.adcelemID", &elemID);
    T->SetBranchAddress("sbs.activeAna_tdc.tdcelemID", &tdcelemID);

    // int correct_mapping[32] = {0, 1, 2, 3, 4, 5, 6, 7,
    //                    11, 10,9,8,15,14,13,12,
    //                   19,18,17,16,23,22,21,20,
    //                   27,26,25,24,31,30,29,28};
    // int corrected_ch = correct_mapping[32];

    TH1D *h = new TH1D(Form("h%d_%d", targetRow, targetCol),
                   Form("ADC Amplitude Row %d Column %d;Amplitude", targetRow, targetCol),
                   100, 0, 1000);
    TH1D *tdcblock = new TH1D(Form("td%d_%d", targetRow, targetCol), Form("TDC Blocks Row %d Column %d;Block", targetRow, targetCol),
                   32, -0.5, 31.5);
    TH1D *adcblock = new TH1D(Form("ad%d_%d", targetRow, targetCol), Form("ADC Blocks Row %d Column %d;ABlock", targetRow, targetCol),
                   32, -0.5, 31.5);
    
    Long64_t nentries = T->GetEntries();
    for (Long64_t i = 0; i < nentries; i++) {
        T->GetEntry(i);
        if (Ndata == 0 || tdcNdata == 0) {
            continue; // Skip this entry to avoid division by zero
        }

        for (int tj = 0; tj <tdcNdata; tj++) {
             if (tdcrow[tj] != targetRow || tdccol[tj] != targetCol ){
                 continue;
            }
        for (int j = 0; j <Ndata; j++){
            // cout << i << '\t' << tj << '\t' << j << '\t' << tdcrow[tj] << '\t' << tdccol[tj] << '\t' << (int)col[j] << '\t' << (int)row[j] << endl;
            if ((int)col[j] != tdccol[tj])
                continue;

            if ((int)row[j] != tdcrow[tj])
                continue;


            //ADC threshold cut
             if (amp[j] < threshold)
                continue;

            // x = event number
            // y = ADC amplitude
            h->Fill(amp[j]);
            tdcblock->Fill(tdcelemID[tj]);
            adcblock->Fill(elemID[j]);
         }
        }  
    }
     cout << "Row " << targetRow
         << " Column " << targetCol
      << endl;


    
    TCanvas *c = new TCanvas("c", "TDC & ADC RowColumn", 900,600);
    c->SetLogy();
    h->Draw();
    c->SaveAs(Form("adc_tdc_histogram_r%d_c%d.pdf", targetRow, targetCol));
    TCanvas *c2 = new TCanvas("c2");
    c2->Divide(1,2);
    c2->cd(1);
    tdcblock->Draw();
    c2->cd(2);
    adcblock->Draw();
    c2->SaveAs(Form("tdc_adc_blocks_r%d_c%d.pdf", targetRow, targetCol));
}    