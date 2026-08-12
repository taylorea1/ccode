//===============================================================
// File: track_to_analyzer_channel.C
//
// Loops over every event and every reconstructed CeR GEM track.
// Projects each track to the active-analyzer z-position.
// Converts the projected position into analyzer channels 0-31.
//
// useProjectedX=true:
//     Analyzer channels are determined from projected x.
//
// useProjectedX=false:
//     Analyzer channels are determined from projected y.
//
// analyzerMinimum and analyzerMaximum must be replaced with the
// real active coordinate limits of the analyzer.
//===============================================================

#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>
#include <TStyle.h>
#include <iostream>

using namespace std;

void track_analyzer(
const char *inputFile="/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root",
Double_t zReference=0.0,
Double_t analyzerZ=-1.0,
Double_t analyzerMinimum=-0.40,
Double_t analyzerMaximum=0.40,
Bool_t useProjectedX=true,
Bool_t reverseChannelOrder=false)
{
const Int_t MAXTRACKS=100;
const Int_t NUMBEROFCHANNELS=32;

// Open the input ROOT file.
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

// Check that the required track branches exist.
const char *requiredBranches[]={
"Ndata.sbs.gemCeR.track.x",
"Ndata.sbs.gemCeR.track.y",
"Ndata.sbs.gemCeR.track.xp",
"Ndata.sbs.gemCeR.track.yp",
"sbs.gemCeR.track.x",
"sbs.gemCeR.track.y",
"sbs.gemCeR.track.xp",
"sbs.gemCeR.track.yp"
};

Int_t requiredCount=
sizeof(requiredBranches)/sizeof(requiredBranches[0]);

for(Int_t i=0;i<requiredCount;i++){
if(!T->GetBranch(requiredBranches[i])){
cerr<<"ERROR: Missing branch "
<<requiredBranches[i]<<endl;
file->Close();
return;
}
}

// Number of values in each track array.
Int_t nTrackX=0;
Int_t nTrackY=0;
Int_t nTrackXp=0;
Int_t nTrackYp=0;

// Reconstructed track arrays.
Double_t trackX[MAXTRACKS];
Double_t trackY[MAXTRACKS];
Double_t trackXp[MAXTRACKS];
Double_t trackYp[MAXTRACKS];

// Connect track-count branches.
T->SetBranchAddress(
"Ndata.sbs.gemCeR.track.x",
&nTrackX
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.track.y",
&nTrackY
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.track.xp",
&nTrackXp
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.track.yp",
&nTrackYp
);

// Connect reconstructed-track branches.
T->SetBranchAddress(
"sbs.gemCeR.track.x",
trackX
);

T->SetBranchAddress(
"sbs.gemCeR.track.y",
trackY
);

T->SetBranchAddress(
"sbs.gemCeR.track.xp",
trackXp
);

T->SetBranchAddress(
"sbs.gemCeR.track.yp",
trackYp
);

// Check the analyzer coordinate range.
if(analyzerMaximum<=analyzerMinimum){
cerr<<"ERROR: analyzerMaximum must be greater than "
<<"analyzerMinimum."<<endl;
file->Close();
return;
}

// Calculate the width of one analyzer channel.
Double_t channelWidth=
(analyzerMaximum-analyzerMinimum)/NUMBEROFCHANNELS;

// Histogram of matched analyzer channels.
TH1D *hChannel=new TH1D(
"hChannel",
"Projected CeR tracks matched to active analyzer;"
"Active-analyzer channel;"
"Projected tracks",
NUMBEROFCHANNELS,
-0.5,
31.5
);

// Projected coordinate versus matched channel.
TH2D *hCoordinateVsChannel=new TH2D(
"hCoordinateVsChannel",
"Projected coordinate versus analyzer channel;"
"Active-analyzer channel;"
"Projected coordinate",
NUMBEROFCHANNELS,
-0.5,
31.5,
200,
analyzerMinimum,
analyzerMaximum
);

// Event number versus matched channel.
TH2D *hEventVsChannel=new TH2D(
"hEventVsChannel",
"Analyzer channel matched by each event;"
"Tree entry;"
"Active-analyzer channel",
500,
0,
T->GetEntries(),
NUMBEROFCHANNELS,
-0.5,
31.5
);

Long64_t numberOfEvents=T->GetEntries();
Long64_t totalTracks=0;
Long64_t matchedTracks=0;
Long64_t outsideAnalyzer=0;
Long64_t oversizedEvents=0;

cout<<"=================================================="<<endl;
cout<<"CeR track to active-analyzer channel matching"<<endl;
cout<<"Events: "<<numberOfEvents<<endl;
cout<<"Analyzer z: "<<analyzerZ<<endl;
cout<<"Analyzer coordinate: "
<<(useProjectedX?"projected x":"projected y")<<endl;
cout<<"Analyzer minimum: "<<analyzerMinimum<<endl;
cout<<"Analyzer maximum: "<<analyzerMaximum<<endl;
cout<<"Channel width: "<<channelWidth<<endl;
cout<<"Channel range: 0-31"<<endl;
cout<<"=================================================="<<endl;

// Loop through every event.
for(Long64_t event=0;event<numberOfEvents;event++){
T->GetEntry(event);

// Protect the fixed-size track arrays.
if(nTrackX>MAXTRACKS||
nTrackY>MAXTRACKS||
nTrackXp>MAXTRACKS||
nTrackYp>MAXTRACKS){
cerr<<"WARNING: Entry "<<event
<<" exceeds MAXTRACKS and was skipped."<<endl;

oversizedEvents++;
continue;
}

// Find the number of complete tracks.
Int_t numberOfTracks=nTrackX;

if(nTrackY<numberOfTracks)
numberOfTracks=nTrackY;

if(nTrackXp<numberOfTracks)
numberOfTracks=nTrackXp;

if(nTrackYp<numberOfTracks)
numberOfTracks=nTrackYp;

// Loop over every reconstructed track in the event.
for(Int_t track=0;track<numberOfTracks;track++){
totalTracks++;

// Read the reconstructed track parameters.
Double_t x0=trackX[track];
Double_t y0=trackY[track];
Double_t xp=trackXp[track];
Double_t yp=trackYp[track];

// Project the track to the analyzer plane.
Double_t deltaZ=analyzerZ-zReference;
Double_t projectedX=x0+xp*deltaZ;
Double_t projectedY=y0+yp*deltaZ;

// Choose the coordinate that determines the channel.
Double_t analyzerCoordinate;

if(useProjectedX)
analyzerCoordinate=projectedX;
else
analyzerCoordinate=projectedY;

// Reject tracks outside the active analyzer area.
if(analyzerCoordinate<analyzerMinimum||
analyzerCoordinate>=analyzerMaximum){
outsideAnalyzer++;

cout<<"Entry "<<event
<<", track "<<track
<<", projected x="<<projectedX
<<", projected y="<<projectedY
<<", outside analyzer"<<endl;

continue;
}

// Convert the projected coordinate into channel 0-31.
Int_t channel=
(Int_t)((analyzerCoordinate-analyzerMinimum)/
channelWidth);

// Protect against floating-point boundary effects.
if(channel<0||channel>=NUMBEROFCHANNELS){
outsideAnalyzer++;
continue;
}

// Reverse channel numbering when necessary.
if(reverseChannelOrder)
channel=31-channel;

// Fill the output histograms.
hChannel->Fill(channel);
hCoordinateVsChannel->Fill(
channel,
analyzerCoordinate
);

hEventVsChannel->Fill(
event,
channel
);

matchedTracks++;

// Print the matched event, track, and analyzer channel.
cout<<"Entry "<<event
<<", track "<<track
<<", x="<<x0
<<", y="<<y0
<<", xp="<<xp
<<", yp="<<yp
<<", projected x="<<projectedX
<<", projected y="<<projectedY
<<", analyzer channel="<<channel
<<endl;
}
}

// Remove the statistics boxes.
gStyle->SetOptStat(0);

// Draw the channel-count histogram.
TCanvas *channelCanvas=new TCanvas(
"channelCanvas",
"Active-analyzer channel matches",
1000,
700
);

channelCanvas->SetGrid();
hChannel->GetXaxis()->SetNdivisions(32);
hChannel->Draw("HIST TEXT");

channelCanvas->SaveAs(
"active_analyzer_channel_matches.png"
);

channelCanvas->SaveAs(
"active_analyzer_channel_matches.pdf"
);

// Draw projected coordinate versus channel.
TCanvas *coordinateCanvas=new TCanvas(
"coordinateCanvas",
"Projected coordinate versus channel",
1000,
700
);

coordinateCanvas->SetRightMargin(0.15);
hCoordinateVsChannel->Draw("COLZ");

coordinateCanvas->SaveAs(
"active_analyzer_coordinate_vs_channel.png"
);

coordinateCanvas->SaveAs(
"active_analyzer_coordinate_vs_channel.pdf"
);

// Save all histograms and canvases.
TFile *outputFile=new TFile(
"active_analyzer_channel_matches.root",
"RECREATE"
);

hChannel->Write();
hCoordinateVsChannel->Write();
hEventVsChannel->Write();
channelCanvas->Write();
coordinateCanvas->Write();
outputFile->Close();

// Print the final summary.
cout<<"=================================================="<<endl;
cout<<"Finished"<<endl;
cout<<"Total reconstructed tracks: "<<totalTracks<<endl;
cout<<"Tracks inside analyzer: "<<matchedTracks<<endl;
cout<<"Tracks outside analyzer: "<<outsideAnalyzer<<endl;
cout<<"Oversized events skipped: "<<oversizedEvents<<endl;
cout<<"=================================================="<<endl;

file->Close();
}