//===============================================================
// File: event_projtrack.C
//
// Reads exactly one TTree entry.
// Finds CeR GEM hits in that entry.
// Uses hit.trackindex to find the associated track.
// Projects each unique track to another z-position.
// Saves one 2D histogram for that single event.
//
// watchModule=-1 means use every module.
// watchStrip=-1 means use every strip.
// useUStrips=true selects U strips.
// useUStrips=false selects V strips.
//===============================================================

#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2D.h>
#include <TString.h>
#include <TStyle.h>
#include <iostream>

using namespace std;

void event_projtrack(
Long64_t eventNumber,
Int_t watchModule=-1,
Int_t watchStrip=-1,
Bool_t useUStrips=true,
const char *inputFile="/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root",
Double_t zReference=0.0,
Double_t zProjection=-1.0)
{
const Int_t MAXHITS=100;
const Int_t MAXTRACKS=100;

// Open the ROOT file.
TFile *file=TFile::Open(inputFile,"READ");

if(!file||file->IsZombie()){
cerr<<"ERROR: Could not open "<<inputFile<<endl;
return;
}

// Get the tree named T.
TTree *T=nullptr;
file->GetObject("T",T);

if(!T){
cerr<<"ERROR: Could not find TTree T."<<endl;
file->Close();
return;
}

// Check that the requested entry exists.
Long64_t numberOfEvents=T->GetEntries();

if(eventNumber<0||eventNumber>=numberOfEvents){
cerr<<"ERROR: Entry "<<eventNumber<<" is outside the valid range."<<endl;
cerr<<"Valid range: 0 to "<<numberOfEvents-1<<endl;
file->Close();
return;
}

// List of branches required by the macro.
const char *requiredBranches[]={
"Ndata.sbs.gemCeR.track.x",
"Ndata.sbs.gemCeR.track.y",
"Ndata.sbs.gemCeR.track.xp",
"Ndata.sbs.gemCeR.track.yp",
"sbs.gemCeR.track.x",
"sbs.gemCeR.track.y",
"sbs.gemCeR.track.xp",
"sbs.gemCeR.track.yp",
"Ndata.sbs.gemCeR.hit.trackindex",
"Ndata.sbs.gemCeR.hit.module",
"Ndata.sbs.gemCeR.hit.ustriplo",
"Ndata.sbs.gemCeR.hit.ustriphi",
"Ndata.sbs.gemCeR.hit.vstriplo",
"Ndata.sbs.gemCeR.hit.vstriphi",
"sbs.gemCeR.hit.trackindex",
"sbs.gemCeR.hit.module",
"sbs.gemCeR.hit.ustriplo",
"sbs.gemCeR.hit.ustriphi",
"sbs.gemCeR.hit.vstriplo",
"sbs.gemCeR.hit.vstriphi"
};

// Stop if any required branch is missing.
Int_t requiredCount=sizeof(requiredBranches)/sizeof(requiredBranches[0]);

for(Int_t i=0;i<requiredCount;i++){
if(!T->GetBranch(requiredBranches[i])){
cerr<<"ERROR: Missing branch "<<requiredBranches[i]<<endl;
file->Close();
return;
}
}

// Number of values stored in each track array.
Int_t nTrackX=0;
Int_t nTrackY=0;
Int_t nTrackXp=0;
Int_t nTrackYp=0;

// Reconstructed track arrays.
Double_t trackX[MAXTRACKS];
Double_t trackY[MAXTRACKS];
Double_t trackXp[MAXTRACKS];
Double_t trackYp[MAXTRACKS];

// Number of values stored in each hit array.
Int_t nHitTrackIndex=0;
Int_t nHitModule=0;
Int_t nUStripLo=0;
Int_t nUStripHi=0;
Int_t nVStripLo=0;
Int_t nVStripHi=0;
Int_t nHitADCU=0;
Int_t nHitU=0;

// GEM hit arrays.
// These branches are /D, so Double_t is used.
Double_t hitTrackIndex[MAXHITS];
Double_t hitModule[MAXHITS];
Double_t uStripLo[MAXHITS];
Double_t uStripHi[MAXHITS];
Double_t vStripLo[MAXHITS];
Double_t vStripHi[MAXHITS];
Double_t hitADCU[MAXHITS];
Double_t hitU[MAXHITS];

// Check whether optional branches exist.
Bool_t hasADCU=
T->GetBranch("Ndata.sbs.gemCeR.hit.ADCU")&&
T->GetBranch("sbs.gemCeR.hit.ADCU");

Bool_t hasHitU=
T->GetBranch("Ndata.sbs.gemCeR.hit.u")&&
T->GetBranch("sbs.gemCeR.hit.u");

// Connect track-count branches.
T->SetBranchAddress("Ndata.sbs.gemCeR.track.x",&nTrackX);
T->SetBranchAddress("Ndata.sbs.gemCeR.track.y",&nTrackY);
T->SetBranchAddress("Ndata.sbs.gemCeR.track.xp",&nTrackXp);
T->SetBranchAddress("Ndata.sbs.gemCeR.track.yp",&nTrackYp);

// Connect track-data branches.
T->SetBranchAddress("sbs.gemCeR.track.x",trackX);
T->SetBranchAddress("sbs.gemCeR.track.y",trackY);
T->SetBranchAddress("sbs.gemCeR.track.xp",trackXp);
T->SetBranchAddress("sbs.gemCeR.track.yp",trackYp);

// Connect hit-count branches.
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.trackindex",&nHitTrackIndex);
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.module",&nHitModule);
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.ustriplo",&nUStripLo);
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.ustriphi",&nUStripHi);
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.vstriplo",&nVStripLo);
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.vstriphi",&nVStripHi);

// Connect hit-data branches.
T->SetBranchAddress("sbs.gemCeR.hit.trackindex",hitTrackIndex);
T->SetBranchAddress("sbs.gemCeR.hit.module",hitModule);
T->SetBranchAddress("sbs.gemCeR.hit.ustriplo",uStripLo);
T->SetBranchAddress("sbs.gemCeR.hit.ustriphi",uStripHi);
T->SetBranchAddress("sbs.gemCeR.hit.vstriplo",vStripLo);
T->SetBranchAddress("sbs.gemCeR.hit.vstriphi",vStripHi);

// Connect optional ADCU branches.
if(hasADCU){
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.ADCU",&nHitADCU);
T->SetBranchAddress("sbs.gemCeR.hit.ADCU",hitADCU);
}

// Connect optional hit-u branches.
if(hasHitU){
T->SetBranchAddress("Ndata.sbs.gemCeR.hit.u",&nHitU);
T->SetBranchAddress("sbs.gemCeR.hit.u",hitU);
}

// Read exactly one event.
T->GetEntry(eventNumber);

// Protect against array overflow.
if(nTrackX>MAXTRACKS||
nTrackY>MAXTRACKS||
nTrackXp>MAXTRACKS||
nTrackYp>MAXTRACKS){
cerr<<"ERROR: Entry "<<eventNumber
<<" has more than "<<MAXTRACKS<<" tracks."<<endl;
file->Close();
return;
}

if(nHitTrackIndex>MAXHITS||
nHitModule>MAXHITS||
nUStripLo>MAXHITS||
nUStripHi>MAXHITS||
nVStripLo>MAXHITS||
nVStripHi>MAXHITS){
cerr<<"ERROR: Entry "<<eventNumber
<<" has more than "<<MAXHITS<<" hits."<<endl;
file->Close();
return;
}

// Find the number of complete track entries.
Int_t numberOfTracks=nTrackX;

if(nTrackY<numberOfTracks)
numberOfTracks=nTrackY;

if(nTrackXp<numberOfTracks)
numberOfTracks=nTrackXp;

if(nTrackYp<numberOfTracks)
numberOfTracks=nTrackYp;

// Find the number of complete hit entries.
Int_t numberOfHits=nHitTrackIndex;

if(nHitModule<numberOfHits)
numberOfHits=nHitModule;

if(useUStrips){
if(nUStripLo<numberOfHits)
numberOfHits=nUStripLo;

if(nUStripHi<numberOfHits)
numberOfHits=nUStripHi;
}
else{
if(nVStripLo<numberOfHits)
numberOfHits=nVStripLo;

if(nVStripHi<numberOfHits)
numberOfHits=nVStripHi;
}

// Arrays for projected track positions.
Double_t projectedX[MAXTRACKS];
Double_t projectedY[MAXTRACKS];
Int_t projectedCount=0;

// Prevent the same track from being projected more than once.
Bool_t trackAlreadyUsed[MAXTRACKS];

for(Int_t track=0;track<MAXTRACKS;track++)
trackAlreadyUsed[track]=false;

// Counters for the summary.
Int_t selectedHits=0;
Int_t hitsWithoutTracks=0;
Int_t invalidTrackIndices=0;

// Print the selected event information.
cout<<endl;
cout<<"=================================================="<<endl;
cout<<"CeR GEM single-event track display"<<endl;
cout<<"Tree entry: "<<eventNumber<<endl;
cout<<"Hits in entry: "<<numberOfHits<<endl;
cout<<"Tracks in entry: "<<numberOfTracks<<endl;
cout<<"Module: ";

if(watchModule<0)
cout<<"all";
else
cout<<watchModule;

cout<<endl;
cout<<(useUStrips?"U":"V")<<" strip: ";

if(watchStrip<0)
cout<<"all";
else
cout<<watchStrip;

cout<<endl;
cout<<"Reference z: "<<zReference<<endl;
cout<<"Projection z: "<<zProjection<<endl;
cout<<"=================================================="<<endl;

// Loop over every hit in this one event.
for(Int_t hit=0;hit<numberOfHits;hit++){
Int_t module=(Int_t)hitModule[hit];

// Skip modules that were not requested.
if(watchModule>=0&&module!=watchModule)
continue;

Int_t stripLo=0;
Int_t stripHi=0;

// Select U-strip or V-strip information.
if(useUStrips){
stripLo=(Int_t)uStripLo[hit];
stripHi=(Int_t)uStripHi[hit];
}
else{
stripLo=(Int_t)vStripLo[hit];
stripHi=(Int_t)vStripHi[hit];
}

// Correct the strip order if necessary.
if(stripLo>stripHi){
Int_t temporary=stripLo;
stripLo=stripHi;
stripHi=temporary;
}

// Skip hits whose cluster does not contain the selected strip.
if(watchStrip>=0){
if(watchStrip<stripLo||watchStrip>stripHi)
continue;
}

selectedHits++;

Int_t trackIndex=(Int_t)hitTrackIndex[hit];

// Print hit information.
cout<<"Hit "<<hit
<<": module="<<module
<<", "<<(useUStrips?"U":"V")
<<" strip range="<<stripLo<<"-"<<stripHi
<<", trackindex="<<trackIndex;

if(hasADCU&&hit<nHitADCU)
cout<<", ADCU="<<hitADCU[hit];

if(hasHitU&&hit<nHitU)
cout<<", hit u="<<hitU[hit];

cout<<endl;

// Negative track indices mean no track is associated.
if(trackIndex<0){
hitsWithoutTracks++;
continue;
}

// Check that the track index is valid.
if(trackIndex>=numberOfTracks){
cout<<"  WARNING: Invalid track index "<<trackIndex
<<". Available tracks: "<<numberOfTracks<<endl;

invalidTrackIndices++;
continue;
}

// Skip a track that was already added from another GEM hit.
if(trackAlreadyUsed[trackIndex]){
cout<<"  Track "<<trackIndex
<<" was already included."<<endl;
continue;
}

trackAlreadyUsed[trackIndex]=true;

// Read the reconstructed track parameters.
Double_t x0=trackX[trackIndex];
Double_t y0=trackY[trackIndex];
Double_t xp=trackXp[trackIndex];
Double_t yp=trackYp[trackIndex];

// Project the track to the requested z-position.
Double_t deltaZ=zProjection-zReference;
Double_t xProjected=x0+xp*deltaZ;
Double_t yProjected=y0+yp*deltaZ;

// Store the projected position.
if(projectedCount<MAXTRACKS){
projectedX[projectedCount]=xProjected;
projectedY[projectedCount]=yProjected;
projectedCount++;
}

// Print track information.
cout<<"  Track "<<trackIndex<<endl;
cout<<"    x="<<x0<<endl;
cout<<"    y="<<y0<<endl;
cout<<"    xp="<<xp<<endl;
cout<<"    yp="<<yp<<endl;
cout<<"    projected x="<<xProjected<<endl;
cout<<"    projected y="<<yProjected<<endl;
}

// Default histogram limits.
Double_t xMinimum=-1.0;
Double_t xMaximum=1.0;
Double_t yMinimum=-1.0;
Double_t yMaximum=1.0;

// Automatically set the limits around projected tracks.
if(projectedCount>0){
xMinimum=projectedX[0];
xMaximum=projectedX[0];
yMinimum=projectedY[0];
yMaximum=projectedY[0];

for(Int_t i=1;i<projectedCount;i++){
if(projectedX[i]<xMinimum)
xMinimum=projectedX[i];

if(projectedX[i]>xMaximum)
xMaximum=projectedX[i];

if(projectedY[i]<yMinimum)
yMinimum=projectedY[i];

if(projectedY[i]>yMaximum)
yMaximum=projectedY[i];
}

// Prevent extremely small histogram ranges.
Double_t xRange=xMaximum-xMinimum;
Double_t yRange=yMaximum-yMinimum;

if(xRange<0.10)
xRange=0.10;

if(yRange<0.10)
yRange=0.10;

// Add margins around the projected points.
xMinimum-=0.25*xRange;
xMaximum+=0.25*xRange;
yMinimum-=0.25*yRange;
yMaximum+=0.25*yRange;
}

// Create the histogram title.
TString title;

title.Form(
"CeR projected tracks, tree entry %lld;"
"Projected x;"
"Projected y",
eventNumber
);

// Create one 2D histogram for this event.
TH2D *hProjectedXY=new TH2D(
"hProjectedXY",
title,
100,xMinimum,xMaximum,
100,yMinimum,yMaximum
);

// Fill one entry for each unique associated track.
for(Int_t i=0;i<projectedCount;i++)
hProjectedXY->Fill(projectedX[i],projectedY[i]);

// Remove the statistics box and format bin text.
gStyle->SetOptStat(0);
gStyle->SetPaintTextFormat(".0f");

// Create and draw the canvas.
TCanvas *canvas=new TCanvas(
"eventTrackCanvas",
"Single CeR GEM event",
900,
750
);

canvas->SetRightMargin(0.15);
canvas->SetGrid();
hProjectedXY->Draw("COLZ TEXT");

// Create an output name.
TString outputBase;

if(watchModule>=0&&watchStrip>=0){
outputBase.Form(
"gemCeR_event_%lld_module_%d_%cstrip_%d",
eventNumber,
watchModule,
useUStrips?'U':'V',
watchStrip
);
}
else if(watchModule>=0){
outputBase.Form(
"gemCeR_event_%lld_module_%d",
eventNumber,
watchModule
);
}
else{
outputBase.Form(
"gemCeR_event_%lld_all_hits",
eventNumber
);
}

// Save PNG and PDF files.
canvas->SaveAs(outputBase+".png");
canvas->SaveAs(outputBase+".pdf");

// Save the histogram and canvas in a ROOT file.
TFile *outputFile=new TFile(outputBase+".root","RECREATE");

hProjectedXY->Write();
canvas->Write();
outputFile->Close();

// Print the final summary.
cout<<"=================================================="<<endl;
cout<<"Finished tree entry "<<eventNumber<<endl;
cout<<"Selected hits: "<<selectedHits<<endl;
cout<<"Unique associated tracks: "<<projectedCount<<endl;
cout<<"Hits without tracks: "<<hitsWithoutTracks<<endl;
cout<<"Invalid track indices: "<<invalidTrackIndices<<endl;
cout<<"Histogram entries: "<<hProjectedXY->GetEntries()<<endl;
cout<<"Created "<<outputBase<<".png"<<endl;
cout<<"Created "<<outputBase<<".pdf"<<endl;
cout<<"Created "<<outputBase<<".root"<<endl;
cout<<"=================================================="<<endl;

file->Close();
}

