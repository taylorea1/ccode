void activeAnaPerformance(double threshold = 0.0)
// {

//     // ============================================================
//     // Open ROOT file containing active analyzer data
//     // ============================================================

//     TFile *f = TFile::Open(
//     "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");


//     if(!f || f->IsZombie())
//     {
//         cout<<"Error opening file!"<<endl;
//         return;
//     }


//     TTree *T = (TTree*)f->Get("T");


//     if(!T)
//     {
//         cout<<"Error: TTree not found!"<<endl;
//         return;
//     }


//     cout<<"Running Active Analyzer Performance Study"<<endl;



//     // ============================================================
//     // Detector information
//     //
//     // Rows:    0-3  (4 rows)
//     // Columns: 0-7  (8 columns)
//     //
//     // Channel number:
//     //
//     // channel = row*8 + column
//     //
//     // Gives channels 0-31
//     // ============================================================


//     Double_t row[1000];
//     Double_t col[1000];
//     Double_t adc[1000];
//     Int_t adcNdata;



//     T->SetBranchAddress( "sbs.activeAna_adc.adcrow", row);
//     T->SetBranchAddress("sbs.activeAna_adc.adccol", col);
//     T->SetBranchAddress( "sbs.activeAna_adc.a", adc);
//     T->SetBranchAddress( "Ndata.sbs.activeAna_adc.adcrow", &adcNdata);




//     // ============================================================
//     // Store channel information
//     // ============================================================


//     int hits[4][8] = {0};
//     double adcSum[4][8] = {0};
//     double adcSum2[4][8] = {0};
//     double adcMin[4][8];
//     double adcMax[4][8];

//     for(int r=0;r<4;r++)
//     {
//         for(int c=0;c<8;c++)
//         {

//             adcMin[r][c]=1e9;

//             adcMax[r][c]=-1e9;

//         }
//     }



//     // ============================================================
//     // ADC distributions for channels 0-31
//     // ============================================================


//     TH1D *hADC[32];


//     for(int ch=0;ch<32;ch++)
//     {

//         hADC[ch] =
//         new TH1D(Form("ADC_%d",ch),
//         Form("Channel %d ADC;ADC;Counts",ch),
//         200,0,4000);

//     }




//     Long64_t n = T->GetEntries();



//     // ============================================================
//     // Loop through events
//     // ============================================================


//     for(Long64_t i=0;i<n;i++)
//     {

//         T->GetEntry(i);



//         for(int j=0;j<adcNdata;j++)
//         {


//         int r = row[j];
//         int c = col[j];




//             // Check detector boundaries

//             if(r<0 || r>=4 ||
//                c<0 || c>=8)
//                 continue;



//             // ADC threshold cut

//             if(adc[j]<threshold)
//                 continue;



//             // Convert row/column to channel

//             int ch=r*8+c;



//             hits[r][c]++;


//             adcSum[r][c]+=adc[j];


//             adcSum2[r][c]+=adc[j]*adc[j];



//             if(adc[j]<adcMin[r][c])
//                 adcMin[r][c]=adc[j];



//             if(adc[j]>adcMax[r][c])
//                 adcMax[r][c]=adc[j];



//             hADC[ch]->Fill(adc[j]);

//         }

//     }





//     // ============================================================
//     // Calculate detector average ADC
//     // ============================================================


//     double totalMean=0;

//     int totalChannels=0;



//     for(int r=0;r<4;r++)
//     {

//         for(int c=0;c<8;c++)
//         {

//             if(hits[r][c]>0)
//             {

//                 totalMean += adcSum[r][c]/hits[r][c];

//                 totalChannels++;

//             }

//         }

//     }


//     if(totalChannels>0)

//         totalMean/=totalChannels;




//     // ============================================================
//     // CSV output
//     // ============================================================


//     ofstream outfile;

//     outfile.open("activeAna_summary.csv");



//     outfile<<"Channel,Row,Column,Hits,"
//            <<"MeanADC,RMS,SNR,"
//            <<"Occupancy,Gain"
//            <<endl;




//     // ============================================================
//     // Detector maps
//     // ============================================================


//     TH2D *hHits =
//     new TH2D("Hits",
//     "Hit Occupancy;Column;Row",
//     8,0,8,
//     4,0,4);



//     TH2D *hMean =
//     new TH2D("MeanADC",
//     "Mean ADC;Column;Row",
//     8,0,8,
//     4,0,4);



//     TH2D *hSNR =
//     new TH2D("SNR",
//     "Signal To Noise Ratio;Column;Row",
//     8,0,8,
//     4,0,4);



//     TH2D *hChannel =
//     new TH2D("ChannelMap",
//     "Channel Number;Column;Row",
//     8,0,8,
//     4,0,4);




//     // ============================================================
//     // Calculate channel performance
//     // ============================================================


//     cout<<endl;

//     cout<<"Channel Row Col Hits MeanADC RMS SNR Occupancy Gain"
//         <<endl;



//     for(int r=0;r<4;r++)
//     {

//         for(int c=0;c<8;c++)
//         {


//             int ch=r*8+c;


//             double mean=0;

//             double rms=0;

//             double snr=0;

//             double gain=0;



//             if(hits[r][c]>0)
//             {

//                 mean =
//                 adcSum[r][c]/hits[r][c];


//                 rms =
//                 sqrt(adcSum2[r][c]/hits[r][c]
//                 -mean*mean);



//                 if(rms>0)

//                     snr=mean/rms;



//                 if(totalMean>0)

//                     gain=mean/totalMean;

//             }



//             double occupancy =
//             100.0*hits[r][c]/n;




//             cout
//             <<ch<<" "
//             <<r<<" "
//             <<c<<" "
//             <<hits[r][c]<<" "
//             <<mean<<" "
//             <<rms<<" "
//             <<snr<<" "
//             <<occupancy<<"%"
//             <<endl;
//             hMean->SetBinContent(c+1,r+1,mean);

//             hSNR->SetBinContent(c+1,r+1,snr);

//             hChannel->SetBinContent(c+1,r+1,ch);


//         }

//     }


//     outfile.close();





//     // ============================================================
//     // Save plots
//     // ============================================================


//     TCanvas *c1 =
//     new TCanvas("occupancy",
//     "Occupancy",800,600);


//     hHits->Draw("COLZ TEXT");

//     c1->SaveAs("activeAna_occupancy.png");




//     TCanvas *c2 =
//     new TCanvas("meanADC",
//     "Mean ADC",800,600);


//     hMean->Draw("COLZ TEXT");

//     c2->SaveAs("activeAna_meanADC.png");




//     TCanvas *c3 =
//     new TCanvas("snr",
//     "SNR",800,600);


//     hSNR->Draw("COLZ TEXT");

//     c3->SaveAs("activeAna_SNR.png");




//     TCanvas *c4 =
//     new TCanvas("channelMap",
//     "Channel Map",800,600);


//     hChannel->Draw("COLZ TEXT");

//     c4->SaveAs("activeAna_channelMap.png");




//     cout<<endl;
//     cout<<"================================"<<endl;
//     cout<<"Analysis Complete"<<endl;
//     cout<<"Created activeAna_summary.csv"<<endl;
//     cout<<"Created detector maps"<<endl;
//     cout<<"===============



//             outfile
//             <<ch<<","
//             <<r<<","
//             <<c<<","
//             <<hits[r][c]<<","
//             <<mean<<","
//             <<rms<<","
//             <<snr<<","
//             <<occupancy<<","
//             <<gain
//             <<endl;



//             hHits->SetBinContent(c+1,r+1,hits[r][c]);

//             hMean->SetBinContent(c+1,r+1,mean);

//             hSNR->SetBinContent(c+1,r+1,snr);

//             hChannel->SetBinContent(c+1,r+1,ch);


// }


{

    // ============================================================
    // Open ROOT file containing active analyzer data
    // ============================================================

    TFile *f = TFile::Open(
    "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");


    if(!f || f->IsZombie())
    {
        cout<<"Error opening file!"<<endl;
        return;
    }


    TTree *T = (TTree*)f->Get("T");


    if(!T)
    {
        cout<<"Error: TTree not found!"<<endl;
        return;
    }


    cout<<"Running Active Analyzer Performance Study"<<endl;



    // ============================================================
    // Detector information
    //
    // Rows:    0-3  (4 rows)
    // Columns: 0-7  (8 columns)
    //
    // Channel number:
    //
    // channel = row*8 + column
    //
    // Gives channels 0-31
    // ============================================================


    Double_t row[1000];
    Double_t col[1000];
    Double_t adc[1000];


    Int_t adcNdata;



    T->SetBranchAddress(
    "sbs.activeAna_adc.adcrow",
    row);


    T->SetBranchAddress(
    "sbs.activeAna_adc.adccol",
    col);


    T->SetBranchAddress(
    "sbs.activeAna_adc.a",
    adc);


    T->SetBranchAddress(
    "Ndata.sbs.activeAna_adc.adcrow",
    &adcNdata);




    // ============================================================
    // Store channel information
    // ============================================================


    int hits[8][4] = {0};


    double adcSum[8][4] = {0};


    double adcSum2[8][4] = {0};


    double adcMin[8][4];

    double adcMax[8][4];



    for(int r=0;r<8;r++)
    {
        for(int c=0;c<4;c++)
        {

            adcMin[r][c]=1e9;

            adcMax[r][c]=-1e9;

        }
    }



    // ============================================================
    // ADC distributions for channels 0-31
    // ============================================================


    TH1D *hADC[32];

    int correct_mapping[32] = {0, 1, 2, 3, 4, 5, 6, 7,
                       11, 10,9,8,15,14,13,12,
                      19,18,17,16,23,22,21,20,
                      27,26,25,24,31,30,29,28};
    int corrected_ch = correct_mapping[32];
    for(int ch=0;ch<32;ch++)
    {
        hADC[corrected_ch] =
        new TH1D(Form("ADC_%d",ch),
        Form("Channel %d ADC;ADC;Counts",ch),
        200,0,4000);

    }




    Long64_t n = T->GetEntries();



    // ============================================================
    // Loop through events
    // ============================================================


    for(Long64_t i=0;i<n;i++)
    {

        T->GetEntry(i);



        for(int j=0;j<adcNdata;j++)
        {


            int r=(int)row[j];

            int c=(int)col[j];



            // Check detector boundaries

            if(r<0 || r>=8 ||
               c<0 || c>=4)
                continue;



            // ADC threshold cut

            if(adc[j]<threshold)
                continue;



            // Convert row/column to channel

            int ch=r*4+c;

            // cout<<"Row: "<<r<<" Col: "<<c<<" Ch: "<<ch<<" ADC: "<<adc[j]<<endl;

            hits[r][c]++;


            adcSum[r][c]+=adc[j];


            adcSum2[r][c]+=adc[j]*adc[j];



            if(adc[j]<adcMin[r][c])
                adcMin[r][c]=adc[j];



            if(adc[j]>adcMax[r][c])
                adcMax[r][c]=adc[j];



            hADC[corrected_ch]->Fill(adc[j]);

        }

    }





    // ============================================================
    // Calculate detector average ADC
    // ============================================================


    double totalMean=0;

    int totalChannels=0;



    for(int r=0;r<8;r++)
    {

        for(int c=0;c<4 ;c++)
        {

            if(hits[r][c]>0)
            {

                totalMean += adcSum[r][c]/hits[r][c];

                totalChannels++;

            }

        }

    }


    if(totalChannels>0)

        totalMean/=totalChannels;




    // ============================================================
    // CSV output
    // ============================================================


    ofstream outfile;

    outfile.open("activeAna_summary.csv");



    outfile<<"Channel,Row,Column,Hits,"
           <<"MeanADC,RMS,SNR,"
           <<"Occupancy,Gain"
           <<endl;




    // ============================================================
    // Detector maps
    // ============================================================


    TH2D *hHits =
    new TH2D("Hits",
    "Hit Occupancy;Column;Row",
    8,0,8,
    4,0,4);



    TH2D *hMean =
    new TH2D("MeanADC",
    "Mean ADC;Column;Row",
    8,0,8,
    4,0,4);



    TH2D *hSNR =
    new TH2D("SNR",
    "Signal To Noise Ratio;Column;Row",
    8,0,8,
    4,0,4);



    TH2D *hChannel =
    new TH2D("ChannelMap",
    "Channel Number;Column;Row",
    8,0,8,
    4,0,4);




    // ============================================================
    // Calculate channel performance
    // ============================================================


    cout<<endl;

    cout<<"Channel Row Col Hits MeanADC RMS SNR Occupancy Gain"
        <<endl;



    for(int r=0;r<8;r++)
    {

        for(int c=0;c<4 ;c++)
        {


            int ch=r*4+c;


            double mean=0;

            double rms=0;

            double snr=0;

            double gain=0;



            if(hits[r][c]>0)
            {

                mean =
                adcSum[r][c]/hits[r][c];


                rms =
                sqrt(adcSum2[r][c]/hits[r][c]
                -mean*mean);



                if(rms>0)

                    snr=mean/rms;



                if(totalMean>0)

                    gain=mean/totalMean;

            }



            double occupancy =
            100.0*hits[r][c]/n;




            cout
            <<ch<<" "
            <<r<<" "
            <<c<<" "
            <<hits[r][c]<<" "
            <<mean<<" "
            <<rms<<" "
            <<snr<<" "
            <<occupancy<<"%"
            <<endl;




            outfile
            <<ch<<","
            <<r<<","
            <<c<<","
            <<hits[r][c]<<","
            <<mean<<","
            <<rms<<","
            <<snr<<","
            <<occupancy<<","
            <<gain
            <<endl;



            hHits->SetBinContent(c+1,r+1,hits[r][c]);

            hMean->SetBinContent(c+1,r+1,mean);

            hSNR->SetBinContent(c+1,r+1,snr);

            hChannel->SetBinContent(c+1,r+1,ch);


        }

    }


    outfile.close();





    // ============================================================
    // Save plots
    // ============================================================


    TCanvas *c1 =
    new TCanvas("occupancy",
    "Occupancy",800,600);


    hHits->Draw("COLZ TEXT");

    c1->SaveAs("activeAna_occupancy.png");




    TCanvas *c2 =
    new TCanvas("meanADC",
    "Mean ADC",800,600);


    hMean->Draw("COLZ TEXT");

    c2->SaveAs("activeAna_meanADC.png");




    TCanvas *c3 =
    new TCanvas("snr",
    "SNR",800,600);


    hSNR->Draw("COLZ TEXT");

    c3->SaveAs("activeAna_SNR.png");




    TCanvas *c4 =
    new TCanvas("channelMap",
    "Channel Map",800,600);


    hChannel->Draw("COLZ TEXT");

    c4->SaveAs("activeAna_channelMap.png");




    cout<<endl;
    cout<<"================================"<<endl;
    cout<<"Analysis Complete"<<endl;
    cout<<"Created activeAna_summary.csv"<<endl;
    cout<<"Created detector maps"<<endl;
    cout<<"================================"<<endl;


}