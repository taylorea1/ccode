void noiseremover()
{
    // Open file
    TFile *f = TFile::Open(
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    // Canvas
    TCanvas *c = new TCanvas("c", "Raw 1", 800, 600);

    // -------------------------------
    // STEP 1: Fill raw 2D histogram
    // -------------------------------
    TH2D *h2_raw = new TH2D(
        "h2_raw",
        "Raw ADC Amp;ADC Amp",
        10, 0, 140,
        10, 0, 100
    );

    T->Draw("sbs.activeAna_adc.a_amp>>h2_raw", "", "COLZ");
/*
    // -------------------------------
    // STEP 2: Estimate pedestal region
    // (assume low signal region = noise dominated)
    // -------------------------------

    // Project X and Y for pedestal estimation
    TH1D *px = h2_raw->ProjectionX("px");
    TH1D *py = h2_raw->ProjectionY("py");

    double pedX = px->GetMean();
    double pedY = py->GetMean();

    double sigmaX = px->GetRMS();
    double sigmaY = py->GetRMS();

    // -------------------------------
    // STEP 3: Define noise threshold
    // (3-sigma cut is standard for physics data)
    // -------------------------------
    double nSigma = 3.0;

    double threshX = pedX + nSigma * sigmaX;
    double threshY = pedY + nSigma * sigmaY;

    std::cout << "Pedestal X: " << pedX << " Y: " << pedY << std::endl;
    std::cout << "Noise thresholds X: " << threshX
              << " Y: " << threshY << std::endl;

    // -------------------------------
    // STEP 4: Build CLEAN histogram
    // -------------------------------
    TH2D *h2_clean = new TH2D(
        "h2_clean",
        "Noise-Filtered ADC Amp vs ADC Amp_C;ADC Amp_C;ADC Amp",
        1000, 187, 190,
        1000, 183, 188
    );

    for (int ix = 1; ix <= h2_raw->GetNbinsX(); ix++)
    {
        for (int iy = 1; iy <= h2_raw->GetNbinsY(); iy++)
        {
            double x = h2_raw->GetXaxis()->GetBinCenter(ix);
            double y = h2_raw->GetYaxis()->GetBinCenter(iy);

            double content = h2_raw->GetBinContent(ix, iy);

            // -------------------------------
            // STEP 5: Signal selection
            // keep only above pedestal noise
            // -------------------------------
            if (x > threshX && y > threshY)
            {
                h2_clean->SetBinContent(ix, iy, content);
            }
            else
            {
                h2_clean->SetBinContent(ix, iy, 0);
            }
        }
    }

    // -------------------------------
    // STEP 6: Draw cleaned result
    // -------------------------------
    */
    c->cd();
    h2_raw->Draw("COLZ");

    // Save output
    c->SaveAs("adc_amp_raw1.pdf");
}


//GemF

/* void noiseremover()
{
    // Open file
    TFile *f = TFile::Open(
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root"
    );

    TTree *T = (TTree*)f->Get("T");

    // Canvas
    TCanvas *c = new TCanvas("c", "Hits", 800, 600);

    // -------------------------------
    // STEP 1: Fill raw 2D histogram
    // -------------------------------
    TH2D *h2_raw = new TH2D(
        "h2_raw",
        "Raw adc.hits.a vs gemCeF.hitADCU;hitsADCU;hit.a",
        10, -0.01, 0.02,
        10, 27, 33.5
    );

    T->Draw("Ndata.sbs.activeAna_adc.nsamps:Ndata.sbs.gemCeF.hit.ADCU>>h2_raw", "", "COLZ");



    TH1D *px = h2_raw->ProjectionX("px");
    TH1D *py = h2_raw->ProjectionY("py");

    double pedX = px->GetMean();
    double pedY = py->GetMean();

    double sigmaX = px->GetRMS();
    double sigmaY = py->GetRMS();

    double nSigma = 2.0;

    double threshX = pedX + nSigma * sigmaX;
    double threshY = pedY + nSigma * sigmaY;

    std::cout << "Pedestal X: " << pedX << " Y: " << pedY << std::endl;
    std::cout << "Noise thresholds X: " << threshX
              << " Y: " << threshY << std::endl;

    TH2D *h2_clean = new TH2D(
        "h2_clean",
        "Noise-Filtered ADC hits vs GemCeF;ADC hits;GemCeF",
        10, 0, 10,
        10, 0, 10
    );

    for (int ix = 1; ix <= h2_raw->GetNbinsX(); ix++)
    {
        for (int iy = 1; iy <= h2_raw->GetNbinsY(); iy++)
        {
            double x = h2_raw->GetXaxis()->GetBinCenter(ix);
            double y = h2_raw->GetYaxis()->GetBinCenter(iy);

            double content = h2_raw->GetBinContent(ix, iy);

            if (x > threshX && y > threshY)
            {
                h2_clean->SetBinContent(ix, iy, content);
            }
            else
            {
                h2_clean->SetBinContent(ix, iy, 0);
            }
        }
    }
    c->cd();
    h2_raw->Draw("COLZ");
    c->SaveAs("adc_nsamps_gemCeF_hits.pdf");
} */
