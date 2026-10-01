void trackbar_correlation(int selectedBar=0,double deltaZ=0.0,double minADC=0.0){
    
    TFile *f=TFile::Open("/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root");
    if(!f||f->IsZombie()){
        cout<<"Error opening file!"<<endl;
        return;
    }

    TTree *T=(TTree*)f->Get("T");
    if(!T){
        cout<<"Error: TTree not found!"<<endl;
        f->Close();
        return;
    }

    //Arrays store all GEM tracks and ADC hits from one event.
    const int maxHits=1000;
    Double_t x[maxHits],y[maxHits],xp[maxHits],yp[maxHits];
   
    Double_t adc[maxHits],adcBar[maxHits],adcRow[maxHits],adcCol[maxHits];
    Int_t nX=0,nY=0,nXp=0,nYp=0;
    Int_t nADC=0,nADCBar=0,nADCRow=0,nADCCol=0;

    //Connect the CeR GEM track branches and their array sizes.
    T->SetBranchAddress("sbs.gemCeR.track.x",x);
    T->SetBranchAddress("sbs.gemCeR.track.y",y);
    T->SetBranchAddress("sbs.gemCeR.track.xp",xp);
    T->SetBranchAddress("sbs.gemCeR.track.yp",yp);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.x",&nX);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.y",&nY);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.xp",&nXp);
    T->SetBranchAddress("Ndata.sbs.gemCeR.track.yp",&nYp);
    

    //Connect the active-analyzer ADC branches and their array sizes.
    T->SetBranchAddress("sbs.activeAna_adc.a_p",adc);
    T->SetBranchAddress("sbs.activeAna_adc.adcelemID",adcBar);
    T->SetBranchAddress("sbs.activeAna_adc.adcrow",adcRow);
    T->SetBranchAddress("sbs.activeAna_adc.adccol",adcCol);
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.a_p",&nADC);
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcelemID",&nADCBar);
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.adcrow",&nADCRow);
    T->SetBranchAddress("Ndata.sbs.activeAna_adc.adccol",&nADCCol);

    //The 1D plots make the track distributions for one bar easier to read.
    TH1D *hTrackX=new TH1D("hTrackX",Form("Bar %d Track X;Projected track x;Tracks",selectedBar),100,-1.5,1.5);
    TH1D *hTrackY=new TH1D("hTrackY",Form("Bar %d Track Y;Projected track y;Tracks",selectedBar),100,-1.5,1.5);
    TH1D *hADC=new TH1D("hADC",Form("Bar %d ADC;ADC pulse integral a_{p};Events",selectedBar),100,-50.5,200.5);

    //The 2D plot shows the complete track-position region for the selected bar.
    TH2D *hTrackXY=new TH2D("hTrackXY",Form("Bar %d Track Positions;Projected track x;Projected track y",selectedBar),100,-10.5,10.5,100,-10.5,10.5);

    //Save every selected event and track so individual matches can be inspected.
    ofstream csv(Form("track_bar%d_correlation.csv",selectedBar));
    csv<<"event,track,x,y,xp,yp,deltaZ,xProjected,yProjected,adcBar,row,column,adc\n";

    Long64_t eventsRead=T->GetEntries();
    Long64_t eventsWithTrack=0;
    Long64_t eventsWithSelectedADC=0;
    Long64_t rowsWritten=0;

    for(Long64_t event=0;event<eventsRead;++event){
        T->GetEntry(event);

        int nTracks=min(min(nX,nY),min(nXp,nYp));
        nTracks=min(nTracks,maxHits);
        //Skip events with no tracks or multiple possible tracks.
        if(nTracks!=1)continue;
        ++eventsWithTrack;
        //A single-track event always uses array index zero.
        int track=0;

        // //Use the smallest Ndata count so all four track arrays are valid.
        // int nTracks=min(min(nX,nY),min(nXp,nYp));
        // nTracks=min(nTracks,maxHits);
        // if(nTracks<=0)continue;
        // ++eventsWithTrack;

        //Use the smallest ADC Ndata count so the ADC arrays remain aligned.
       
        

    //     //Find the largest ADC pulse belonging only to selectedBar.
    //     for(int ia=0;ia<nADCHits;++ia){
    //     int bar=(int)lround(adcBar[ia]);
    //     int row=(int)lround(adcRow[ia]);
    //     int col=(int)lround(adcCol[ia]);

    //     if(!isfinite(adc[ia]))continue;
    //     if(bar<0||bar>31)continue;
    //     if(row<0||row>7||col<0||col>3)continue;
    //     if(bar!=row*4+col)continue;

    //     if(adc[ia]>leadingADC){
    //         leadingADC=adc[ia];
    //         leadingIndex=ia;
    //     }
    // }

    //     if(leadingIndex<0)continue;
    //     int bar=(int)lround(adcBar[leadingIndex]);
    //     int row=(int)lround(adcRow[leadingIndex]);
    //     int col=(int)lround(adcCol[leadingIndex]);
    //     //Keep only events where the selected bar has the largest ADC.
    //     if(bar!=selectedBar)continue;
    //     ++eventsWithSelectedADC;
    //     hADC->Fill(leadingADC);

    //     //Skip the event if the selected bar had no ADC above minADC.
    //     if(selectedIndex<0)continue;
    //     ++eventsWithSelectedADC;
    //     hADC->Fill(selectedADC);

    //     int bar=(int)lround(adcBar[selectedIndex]);
    //     int row=(int)lround(adcRow[selectedIndex]);
    //     int col=(int)lround(adcCol[selectedIndex]);



    //Find the number of aligned ADC entries in this event.
    int nADCHits=min(min(nADC,nADCBar),min(nADCRow,nADCCol));
    nADCHits=min(nADCHits,maxHits);

    //Store the index and amplitude of the largest ADC among all 32 bars.
    int leadingIndex=-1;
    double leadingADC=minADC;

    for(int ia=0;ia<nADCHits;++ia){
        int hitBar=(int)lround(adcBar[ia]);
        int hitRow=(int)lround(adcRow[ia]);
        int hitCol=(int)lround(adcCol[ia]);

        if(!isfinite(adc[ia]))continue;
        if(hitBar<0||hitBar>31)continue;
        if(hitRow<0||hitRow>7)continue;
        if(hitCol<0||hitCol>3)continue;

        //Verify the 8-row by 4-column channel mapping.
        if(hitBar!=hitRow*4+hitCol)continue;

        //Keep the largest ADC pulse in the entire event.
        if(adc[ia]>leadingADC){
            leadingADC=adc[ia];
            leadingIndex=ia;
        }
    }

    //Skip events with no valid ADC above minADC.
    if(leadingIndex<0)continue;

    //Get the bar, row and column belonging to the leading ADC.
    int bar=(int)lround(adcBar[leadingIndex]);
    int row=(int)lround(adcRow[leadingIndex]);
    int col=(int)lround(adcCol[leadingIndex]);

    //Keep only events where the chosen bar has the largest ADC.
    if(bar!=selectedBar)continue;

    ++eventsWithSelectedADC;

    //Fill ADC once per accepted event.
            hADC->Fill(leadingADC);
        if(!isfinite(x[track])||!isfinite(y[track])||
        !isfinite(xp[track])||!isfinite(yp[track]))continue;

        double xProjected=x[track]+xp[track]*deltaZ;
        double yProjected=y[track]+yp[track]*deltaZ;

        hTrackX->Fill(xProjected);
        hTrackY->Fill(yProjected);
        hTrackXY->Fill(xProjected,yProjected);

        csv<<event<<','<<track<<','<<x[track]<<','<<y[track]
        <<','<<xp[track]<<','<<yp[track]<<','<<deltaZ
        <<','<<xProjected<<','<<yProjected<<','<<bar
        <<','<<row<<','<<col<<','<<leadingADC<<'\n';

        ++rowsWritten;

    
    // //Reject invalid track values.
    // if(!isfinite(x[track])||!isfinite(y[track])||
    // !isfinite(xp[track])||!isfinite(yp[track]))continue;

    // //Project the selected GEM track.
    // //With deltaZ=0, these equal the original track x and y.
    // double xProjected=x[track]+xp[track]*deltaZ;
    // double yProjected=y[track]+yp[track]*deltaZ;

    // //Fill one entry for the best track.
    // hTrackX->Fill(xProjected);
    // hTrackY->Fill(yProjected);
    // hTrackXY->Fill(xProjected,yProjected);

    // //Record one track-bar match for this event.
    // csv<<event<<','<<track<<','<<x[track]<<','<<y[track]
    // <<','<<xp[track]<<','<<yp[track]<<','<<deltaZ
    // <<','<<xProjected<<','<<yProjected<<','<<bar
    // <<','<<row<<','<<col<<','<<leadingADC<<'\n';

    // ++rowsWritten;

    //     for(int track=0;track<nTracks;++track){
    //         // if(!isfinite(x[track])||!isfinite(y[track])||!isfinite(xp[track])||!isfinite(yp[track]))continue;

    //         // //Project the GEM track. With deltaZ=0, these equal the raw x and y.
    //         // double xProjected=x[track]+xp[track]*deltaZ;
    //         // double yProjected=y[track]+yp[track]*deltaZ;

    //         // hTrackX->Fill(xProjected);
    //         // hTrackY->Fill(yProjected);
    //         // hTrackXY->Fill(xProjected,yProjected);

    //         // csv<<event<<','<<track<<','<<x[track]<<','<<y[track]<<','<<xp[track]<<','<<yp[track]<<','<<deltaZ<<','<<xProjected<<','<<yProjected<<','<<bar<<','<<row<<','<<col<<','<<leadingADC<<'\n';
    //         // ++rowsWritten;
            
    //     }
    // }

    csv.close();
    gStyle->SetOptStat(1110);

    //Show three simple 1D plots and one supporting 2D position plot.
    TCanvas *cBar=new TCanvas("cBar",Form("Bar %d Track Correlation",selectedBar),1200,900);
    cBar->Divide(2,2);
    cBar->cd(1);
    hTrackX->Draw("HIST");
    cBar->cd(2);
    hTrackY->Draw("HIST");
    cBar->cd(3);
    hADC->Draw("HIST");
    cBar->cd(4);
    hTrackXY->Draw("COLZ");

    //Pause before saving so the plots can be inspected interactively.
    cBar->Update();
    gSystem->ProcessEvents();
    cout<<"Inspect the plots, then press Enter to save..."<<endl;
    cin.get();
    cBar->SaveAs(Form("track_bar%d_summary.png",selectedBar));

    TFile output(Form("track_bar%d_correlation.root",selectedBar),"RECREATE");
    hTrackX->Write();
    hTrackY->Write();
    hADC->Write();
    hTrackXY->Write();
    output.Close();

    cout<<"Events read: "<<eventsRead<<'\n';
    cout<<"Events with CeR tracks: "<<eventsWithTrack<<'\n';
    cout<<"Events with bar "<<selectedBar<<" ADC above "<<minADC<<": "<<eventsWithSelectedADC<<'\n';
    cout<<"Track-bar rows written: "<<rowsWritten<<'\n';
    cout<<"deltaZ used: "<<deltaZ<<'\n';

    // f->Close();
    }
}