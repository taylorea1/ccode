
//===============================================================
// File: projtrack.C
//
// Selects one U or V strip in one CeR GEM module.
// The selected strip can occur anywhere inside the hit cluster.
// hit.trackindex links each selected hit to its reconstructed track.
// The matched track is projected to a chosen z-position.
//===============================================================

#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2D.h>
#include <TString.h>
#include <iostream>

using namespace std;

void projtrack(
int watchStrip=456,
int watchModule=0,
bool useUStrips=true,
const char *inputFile="/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root",
double zReference=0.0,
double zProjection=-1.0)
{
const Int_t MAXHITS=100;
const Int_t MAXTRACKS=100;

TFile *file=TFile::Open(inputFile,"READ");

if(!file||file->IsZombie()){
cerr<<"ERROR: Could not open "<<inputFile<<endl;
return;
}

TTree *T=nullptr;
file->GetObject("T",T);

if(!T){
cerr<<"ERROR: Could not find TTree T."<<endl;
file->Close();
return;
}

// Check that all required branches exist.
const char *requiredBranches[]={
"sbs.gemCeR.track.x",
"sbs.gemCeR.track.y",
"sbs.gemCeR.track.xp",
"sbs.gemCeR.track.yp",
"sbs.gemCeR.hit.trackindex",
"sbs.gemCeR.hit.module",
"sbs.gemCeR.hit.ustriplo",
"sbs.gemCeR.hit.ustriphi",
"sbs.gemCeR.hit.vstriplo",
"sbs.gemCeR.hit.vstriphi",
"sbs.gemCeR.hit.ADCU",
"sbs.gemCeR.hit.u"
};

Int_t numberOfRequiredBranches=
sizeof(requiredBranches)/sizeof(requiredBranches[0]);

for(Int_t i=0;i<numberOfRequiredBranches;i++){
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
// These branches are /D, so they must use Double_t.
Double_t hitTrackIndex[MAXHITS];
Double_t hitModule[MAXHITS];
Double_t uStripLo[MAXHITS];
Double_t uStripHi[MAXHITS];
Double_t vStripLo[MAXHITS];
Double_t vStripHi[MAXHITS];
Double_t hitADCU[MAXHITS];
Double_t hitU[MAXHITS];

// Connect the track-count branches.
T->SetBranchAddress("Ndata.sbs.gemCeR.track.x",&nTrackX);
T->SetBranchAddress("Ndata.sbs.gemCeR.track.y",&nTrackY);
T->SetBranchAddress("Ndata.sbs.gemCeR.track.xp",&nTrackXp);
T->SetBranchAddress("Ndata.sbs.gemCeR.track.yp",&nTrackYp);

// Connect the reconstructed track arrays.
T->SetBranchAddress("sbs.gemCeR.track.x",trackX);
T->SetBranchAddress("sbs.gemCeR.track.y",trackY);
T->SetBranchAddress("sbs.gemCeR.track.xp",trackXp);
T->SetBranchAddress("sbs.gemCeR.track.yp",trackYp);

// Connect the hit-count branches.
T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.trackindex",
&nHitTrackIndex
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.module",
&nHitModule
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.ustriplo",
&nUStripLo
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.ustriphi",
&nUStripHi
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.vstriplo",
&nVStripLo
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.vstriphi",
&nVStripHi
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.ADCU",
&nHitADCU
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.u",
&nHitU
);

// Connect the GEM hit arrays.
T->SetBranchAddress(
"sbs.gemCeR.hit.trackindex",
hitTrackIndex
);

T->SetBranchAddress(
"sbs.gemCeR.hit.module",
hitModule
);

T->SetBranchAddress(
"sbs.gemCeR.hit.ustriplo",
uStripLo
);

T->SetBranchAddress(
"sbs.gemCeR.hit.ustriphi",
uStripHi
);

T->SetBranchAddress(
"sbs.gemCeR.hit.vstriplo",
vStripLo
);

T->SetBranchAddress(
"sbs.gemCeR.hit.vstriphi",
vStripHi
);

T->SetBranchAddress(
"sbs.gemCeR.hit.ADCU",
hitADCU
);

T->SetBranchAddress(
"sbs.gemCeR.hit.u",
hitU
);

// Create the projected-track histogram.
TString title;

title.Form(
"Projected CeR tracks, module %d, %c strip %d;"
"Projected x;"
"Projected y",
watchModule,
useUStrips?'U':'V',
watchStrip
);

TH1D *hProjectedXY=new TH1D(
"hProjectedXY",
title,
10,-1.0,10.5);

Long64_t numberOfEvents=T->GetEntries();
Long64_t selectedHits=0;
Long64_t associatedTracks=0;
Long64_t invalidTrackIndices=0;
Long64_t oversizedEvents=0;

cout<<"============================================"<<endl;
cout<<"CeR GEM track projection"<<endl;
cout<<"Module: "<<watchModule<<endl;
cout<<"Strip direction: "<<(useUStrips?"U":"V")<<endl;
cout<<"Selected strip: "<<watchStrip<<endl;
cout<<"Reference z: "<<zReference<<endl;
cout<<"Projection z: "<<zProjection<<endl;
cout<<"Events: "<<numberOfEvents<<endl;
cout<<"============================================"<<endl;

// Loop through every event.
for(Long64_t eventNumber=0;
eventNumber<numberOfEvents;
eventNumber++){

T->GetEntry(eventNumber);

// Protect the fixed-size arrays from overflowing.
if(nHitTrackIndex>MAXHITS||
nHitModule>MAXHITS||
nUStripLo>MAXHITS||
nUStripHi>MAXHITS||
nVStripLo>MAXHITS||
nVStripHi>MAXHITS||
nHitADCU>MAXHITS||
nHitU>MAXHITS||
nTrackX>MAXTRACKS||
nTrackY>MAXTRACKS||
nTrackXp>MAXTRACKS||
nTrackYp>MAXTRACKS){

cerr<<"WARNING: Event "<<eventNumber
<<" exceeds the array limits and was skipped."
<<endl;

oversizedEvents++;
continue;
}

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

// Find the number of complete track entries.
Int_t numberOfTracks=nTrackX;

if(nTrackY<numberOfTracks)
numberOfTracks=nTrackY;

if(nTrackXp<numberOfTracks)
numberOfTracks=nTrackXp;

if(nTrackYp<numberOfTracks)
numberOfTracks=nTrackYp;

// Loop through every GEM hit in the event.
for(Int_t hit=0;hit<numberOfHits;hit++){

Int_t module=(Int_t)hitModule[hit];

if(module!=watchModule)
continue;

Int_t stripLo=0;
Int_t stripHi=0;

if(useUStrips){
stripLo=(Int_t)uStripLo[hit];
stripHi=(Int_t)uStripHi[hit];
}
else{
stripLo=(Int_t)vStripLo[hit];
stripHi=(Int_t)vStripHi[hit];
}

// Correct reversed strip ranges if one occurs.
if(stripLo>stripHi){
Int_t temporaryStrip=stripLo;
stripLo=stripHi;
stripHi=temporaryStrip;
}

// The selected strip must lie inside the cluster.
if(watchStrip<stripLo||watchStrip>stripHi)
continue;

selectedHits++;

Int_t trackIndex=(Int_t)hitTrackIndex[hit];

// Negative values mean the hit is not assigned to a track.
if(trackIndex<0){
cout<<"Event "<<eventNumber
<<", hit "<<hit
<<", module "<<module
<<", strip range "<<stripLo<<"-"<<stripHi
<<", no associated track"
<<endl;

continue;
}

// Verify that the track index exists.
if(trackIndex>=numberOfTracks){
cout<<"Event "<<eventNumber
<<", hit "<<hit
<<", invalid track index "<<trackIndex
<<", available tracks "<<numberOfTracks
<<endl;

invalidTrackIndices++;
continue;
}

// Read the reconstructed track.
Double_t x0=trackX[trackIndex];
Double_t y0=trackY[trackIndex];
Double_t xp=trackXp[trackIndex];
Double_t yp=trackYp[trackIndex];

// Project the straight-line track.
Double_t deltaZ=zProjection-zReference;
Double_t projectedX=x0+xp*deltaZ;
Double_t projectedY=y0+yp*deltaZ;

hProjectedXY->Fill(projectedX,projectedY);
associatedTracks++;

// Print information about the matched hit and track.
cout<<"Event "<<eventNumber
<<", hit "<<hit
<<", module "<<module
<<", strip range "<<stripLo<<"-"<<stripHi
<<", track "<<trackIndex;

if(hit<nHitADCU)
cout<<", ADCU "<<hitADCU[hit];

if(hit<nHitU)
cout<<", hit u "<<hitU[hit];

cout<<", x "<<x0
<<", y "<<y0
<<", xp "<<xp
<<", yp "<<yp
<<", projected x "<<projectedX
<<", projected y "<<projectedY
<<endl;
}
}

// Create and draw the canvas.
TCanvas *canvas=new TCanvas(
"channelTrackCanvas",
"Specific GEM channel tracks",
900,
750);

canvas->SetRightMargin(0.15);
hProjectedXY->Draw("COLZ");

// Create output names based on the selected module and strip.
TString outputBase;

outputBase.Form(
"gemCeR_module_%d_%cstrip_%d",
watchModule,
useUStrips?'U':'V',
watchStrip
);

canvas->SaveAs(outputBase+".png");
canvas->SaveAs(outputBase+".pdf");

// Save the histogram and canvas in a ROOT file.
TFile *outputFile=new TFile(
outputBase+".root",
"RECREATE"
);

hProjectedXY->Write();
canvas->Write();
outputFile->Close();

cout<<"============================================"<<endl;
cout<<"Finished"<<endl;
cout<<"Selected hits: "<<selectedHits<<endl;
cout<<"Associated tracks: "<<associatedTracks<<endl;
cout<<"Invalid track indices: "<<invalidTrackIndices<<endl;
cout<<"Oversized events skipped: "<<oversizedEvents<<endl;
cout<<"Histogram entries: "<<hProjectedXY->GetEntries()<<endl;
cout<<"Created "<<outputBase<<".png"<<endl;
cout<<"Created "<<outputBase<<".pdf"<<endl;
cout<<"Created "<<outputBase<<".root"<<endl;
cout<<"============================================"<<endl;

file->Close();
}
