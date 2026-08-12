//===============================================================
// File: verify_track_channel.C
//
// Checks whether measured events in one active-analyzer channel
// agree with reconstructed CeR GEM tracks.
//
// For each TTree entry:
//   1. Match active-analyzer ADC and TDC hits by channel.
//   2. Check whether watchChannel has a measured hit.
//   3. Project every CeR GEM track to the analyzer plane.
//   4. Convert the projected x/y position to channel 0-31.
//   5. Classify the result as exact, adjacent, or no_match.
//
// IMPORTANT:
// analyzerXMin, analyzerXMax, analyzerYMin, analyzerYMax,
// analyzerZ, and zReference must use the real detector geometry.
//===============================================================

#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TLeaf.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>
#include <TStyle.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

struct AnalyzerHit{
Int_t channel;
Double_t adc;
Double_t tdc;
};

// Get a TLeaf using either its leaf name or branch name.
TLeaf *GetLeafFromTree(TTree *T,const char *name){
TLeaf *leaf=T->GetLeaf(name);

if(leaf)
return leaf;

TBranch *branch=T->GetBranch(name);

if(!branch||
!branch->GetListOfLeaves()||
branch->GetListOfLeaves()->GetEntries()==0)
return nullptr;

return (TLeaf *)branch->GetListOfLeaves()->At(0);
}

// Build the same kind of matched ADC/TDC hit list used by
// eventracker_corrected.
vector<AnalyzerHit> BuildAnalyzerHits(
TLeaf *adcValues,
TLeaf *adcChannels,
TLeaf *tdcValues,
TLeaf *tdcChannels)
{
vector<AnalyzerHit> hits;

Int_t adcCount=min(
adcValues->GetNdata(),
adcChannels->GetNdata()
);

Int_t tdcCount=min(
tdcValues->GetNdata(),
tdcChannels->GetNdata()
);

// Loop over all ADC entries.
for(Int_t adcIndex=0;adcIndex<adcCount;adcIndex++){
Int_t channel=(Int_t)lround(
adcChannels->GetValue(adcIndex)
);

if(channel<0||channel>=32)
continue;

// Find the first unused TDC hit from the same channel.
Bool_t tdcFound=false;
Double_t matchedTDC=0.0;

for(Int_t tdcIndex=0;tdcIndex<tdcCount;tdcIndex++){
Int_t tdcChannel=(Int_t)lround(
tdcChannels->GetValue(tdcIndex)
);

if(tdcChannel!=channel)
continue;

matchedTDC=tdcValues->GetValue(tdcIndex);
tdcFound=true;
break;
}

// Keep only ADC hits that have a matching TDC channel.
if(tdcFound){
AnalyzerHit hit;
hit.channel=channel;
hit.adc=adcValues->GetValue(adcIndex);
hit.tdc=matchedTDC;
hits.push_back(hit);
}
}

return hits;
}

// Convert a channel number into its 8-by-4 grid column.
Int_t ChannelColumn(Int_t channel){
return channel%8;
}

// Convert a channel number into its 8-by-4 grid row.
Int_t ChannelRow(Int_t channel){
return channel/8;
}

// Determine whether two analyzer channels touch each other.
// Diagonal neighbors are included.
Bool_t ChannelsAreAdjacent(Int_t channel1,Int_t channel2){
Int_t columnDifference=abs(
ChannelColumn(channel1)-ChannelColumn(channel2)
);

Int_t rowDifference=abs(
ChannelRow(channel1)-ChannelRow(channel2)
);

return columnDifference<=1&&
rowDifference<=1&&
channel1!=channel2;
}

// Convert a projected x/y position into analyzer channel 0-31.
Int_t PositionToChannel(
Double_t projectedX,
Double_t projectedY,
Double_t analyzerXMin,
Double_t analyzerXMax,
Double_t analyzerYMin,
Double_t analyzerYMax,
Bool_t reverseX,
Bool_t reverseY)
{
const Int_t numberOfColumns=8;
const Int_t numberOfRows=4;

// Reject projections outside the analyzer.
if(projectedX<analyzerXMin||
projectedX>=analyzerXMax||
projectedY<analyzerYMin||
projectedY>=analyzerYMax)
return -1;

Double_t channelWidth=
(analyzerXMax-analyzerXMin)/numberOfColumns;

Double_t channelHeight=
(analyzerYMax-analyzerYMin)/numberOfRows;

Int_t column=(Int_t)(
(projectedX-analyzerXMin)/channelWidth
);

Int_t row=(Int_t)(
(projectedY-analyzerYMin)/channelHeight
);

// Reverse channel directions when required by detector geometry.
if(reverseX)
column=numberOfColumns-1-column;

if(reverseY)
row=numberOfRows-1-row;

if(column<0||column>=numberOfColumns||
row<0||row>=numberOfRows)
return -1;

return row*numberOfColumns+column;
}

void verifytrack(
Int_t watchChannel=0,
const char *inputFile="/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root",
Double_t zReference=0.0,
Double_t analyzerZ=-1.0,
Double_t analyzerXMin=-0.40,
Double_t analyzerXMax=0.40,
Double_t analyzerYMin=-0.20,
Double_t analyzerYMax=0.20,
Bool_t reverseX=false,
Bool_t reverseY=false)
{
const Int_t MAXTRACKS=100;

// Check the requested channel.
if(watchChannel<0||watchChannel>31){
cerr<<"ERROR: watchChannel must be between 0 and 31."<<endl;
return;
}

// Check the analyzer geometry limits.
if(analyzerXMax<=analyzerXMin||
analyzerYMax<=analyzerYMin){
cerr<<"ERROR: Invalid analyzer coordinate limits."<<endl;
return;
}

// Open the ROOT file.
TFile *file=TFile::Open(inputFile,"READ");

if(!file||file->IsZombie()){
cerr<<"ERROR: Could not open "<<inputFile<<endl;
return;
}

// Get the TTree.
TTree *T=nullptr;
file->GetObject("T",T);

if(!T){
cerr<<"ERROR: Could not find TTree T."<<endl;
file->Close();
return;
}

// Get active-analyzer data leaves.
TLeaf *adcValues=
GetLeafFromTree(T,"sbs.activeAna_adc.a");

TLeaf *adcChannels=
GetLeafFromTree(T,"sbs.activeAna_adc.adcelemID");

TLeaf *tdcValues=
GetLeafFromTree(T,"sbs.activeAna_tdc.hits.t");

TLeaf *tdcChannels=
GetLeafFromTree(T,"sbs.activeAna_tdc.tdcelemID");

if(!adcValues||
!adcChannels||
!tdcValues||
!tdcChannels){
cerr<<"ERROR: Missing active-analyzer ADC or TDC branches."<<endl;
file->Close();
return;
}

// CeR GEM track-array counts.
Int_t nTrackX=0;
Int_t nTrackY=0;
Int_t nTrackXp=0;
Int_t nTrackYp=0;

// CeR GEM track arrays.
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

// Connect track-data branches.
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

// Create the output CSV.
TString csvName;

csvName.Form(
"channel_%02d_track_verification.csv",
watchChannel
);

ofstream csv(csvName.Data());

if(!csv){
cerr<<"ERROR: Could not create "<<csvName<<endl;
file->Close();
return;
}

csv<<"Entry,MeasuredChannel,ADC,TDC,Track,"
<<"TrackX,TrackY,TrackXp,TrackYp,"
<<"ProjectedX,ProjectedY,PredictedChannel,"
<<"ColumnDifference,RowDifference,Match\n";

// Predicted channel versus measured channel.
TH2D *hPredictedVsMeasured=new TH2D(
"hPredictedVsMeasured",
"Predicted versus measured analyzer channel;"
"Projected GEM channel;"
"Measured analyzer channel",
32,-0.5,31.5,
32,-0.5,31.5
);

// Count predicted channels for events containing watchChannel.
TH1D *hPredictedChannel=new TH1D(
"hPredictedChannel",
"Projected channels for selected measured events;"
"Projected analyzer channel;"
"Tracks",
32,-0.5,31.5
);

// Count match classifications.
TH1D *hMatchType=new TH1D(
"hMatchType",
"Track alignment classification;"
"Classification;"
"Tracks",
3,0.5,3.5
);

hMatchType->GetXaxis()->SetBinLabel(1,"Exact");
hMatchType->GetXaxis()->SetBinLabel(2,"Adjacent");
hMatchType->GetXaxis()->SetBinLabel(3,"No match");

// Track counters.
Long64_t totalEntries=T->GetEntries();
Long64_t measuredEvents=0;
Long64_t projectedTracks=0;
Long64_t exactMatches=0;
Long64_t adjacentMatches=0;
Long64_t noMatches=0;
Long64_t eventsWithoutTracks=0;
Long64_t oversizedEvents=0;

// Loop over every TTree entry.
for(Long64_t entry=0;entry<totalEntries;entry++){
if(T->GetEntry(entry)<=0)
continue;

// Build measured ADC/TDC hits for this entry.
vector<AnalyzerHit> analyzerHits=BuildAnalyzerHits(
adcValues,
adcChannels,
tdcValues,
tdcChannels
);

// Find all measured hits in the selected analyzer channel.
vector<AnalyzerHit> selectedAnalyzerHits;

for(size_t hitIndex=0;
hitIndex<analyzerHits.size();
hitIndex++){
if(analyzerHits[hitIndex].channel==watchChannel)
selectedAnalyzerHits.push_back(analyzerHits[hitIndex]);
}

// Skip entries where the selected channel did not fire.
if(selectedAnalyzerHits.empty())
continue;

measuredEvents++;

// Protect the fixed-size track arrays.
if(nTrackX>MAXTRACKS||
nTrackY>MAXTRACKS||
nTrackXp>MAXTRACKS||
nTrackYp>MAXTRACKS){
oversizedEvents++;
continue;
}

// Determine the number of complete reconstructed tracks.
Int_t numberOfTracks=nTrackX;

if(nTrackY<numberOfTracks)
numberOfTracks=nTrackY;

if(nTrackXp<numberOfTracks)
numberOfTracks=nTrackXp;

if(nTrackYp<numberOfTracks)
numberOfTracks=nTrackYp;

if(numberOfTracks<=0){
eventsWithoutTracks++;

// Record measured hits even when there is no GEM track.
for(size_t measuredIndex=0;
measuredIndex<selectedAnalyzerHits.size();
measuredIndex++){
csv<<entry<<","
<<watchChannel<<","
<<selectedAnalyzerHits[measuredIndex].adc<<","
<<selectedAnalyzerHits[measuredIndex].tdc<<","
<<-1<<","
<<"nan,nan,nan,nan,"
<<"nan,nan,-1,"
<<"nan,nan,no_track\n";
}

continue;
}

// Project every reconstructed track.
for(Int_t track=0;track<numberOfTracks;track++){
projectedTracks++;

Double_t x0=trackX[track];
Double_t y0=trackY[track];
Double_t xp=trackXp[track];
Double_t yp=trackYp[track];

Double_t deltaZ=analyzerZ-zReference;

Double_t projectedX=x0+xp*deltaZ;
Double_t projectedY=y0+yp*deltaZ;

Int_t predictedChannel=PositionToChannel(
projectedX,
projectedY,
analyzerXMin,
analyzerXMax,
analyzerYMin,
analyzerYMax,
reverseX,
reverseY
);

// Start with no measured match.
TString matchType="no_match";
Int_t columnDifference=-1;
Int_t rowDifference=-1;

// Compare the projected track with watchChannel.
if(predictedChannel==watchChannel){
matchType="exact";
columnDifference=0;
rowDifference=0;
exactMatches++;
hMatchType->Fill(1);
}
else if(predictedChannel>=0&&
ChannelsAreAdjacent(predictedChannel,watchChannel)){
matchType="adjacent";

columnDifference=abs(
ChannelColumn(predictedChannel)-
ChannelColumn(watchChannel)
);

rowDifference=abs(
ChannelRow(predictedChannel)-
ChannelRow(watchChannel)
);

adjacentMatches++;
hMatchType->Fill(2);
}
else{
noMatches++;
hMatchType->Fill(3);
}

// Fill histograms for valid projected channels.
if(predictedChannel>=0){
hPredictedChannel->Fill(predictedChannel);

hPredictedVsMeasured->Fill(
predictedChannel,
watchChannel
);
}

// Write one CSV row for every selected measured hit.
for(size_t measuredIndex=0;
measuredIndex<selectedAnalyzerHits.size();
measuredIndex++){
csv<<entry<<","
<<watchChannel<<","
<<selectedAnalyzerHits[measuredIndex].adc<<","
<<selectedAnalyzerHits[measuredIndex].tdc<<","
<<track<<","
<<x0<<","
<<y0<<","
<<xp<<","
<<yp<<","
<<projectedX<<","
<<projectedY<<","
<<predictedChannel<<","
<<columnDifference<<","
<<rowDifference<<","
<<matchType<<'\n';
}

// Print exact and adjacent matches.
if(matchType!="no_match"){
cout<<"Entry "<<entry
<<", track "<<track
<<", measured channel "<<watchChannel
<<", predicted channel "<<predictedChannel
<<", result "<<matchType
<<endl;
}
}
}

csv.close();
gStyle->SetOptStat(0);

// Draw projected-channel distribution.
TCanvas *channelCanvas=new TCanvas(
"channelCanvas",
"Projected analyzer channels",
1000,
700
);

channelCanvas->SetGrid();
hPredictedChannel->Draw("HIST TEXT");

TString channelPdf;

channelPdf.Form(
"channel_%02d_projected_tracks.pdf",
watchChannel
);

channelCanvas->SaveAs(channelPdf);

// Draw predicted-versus-measured comparison.
TCanvas *comparisonCanvas=new TCanvas(
"comparisonCanvas",
"Predicted versus measured channels",
900,
800
);

comparisonCanvas->SetRightMargin(0.15);
comparisonCanvas->SetGrid();
hPredictedVsMeasured->Draw("COLZ TEXT");

TString comparisonPdf;

comparisonPdf.Form(
"channel_%02d_track_comparison.pdf",
watchChannel
);

comparisonCanvas->SaveAs(comparisonPdf);

// Draw match summary.
TCanvas *matchCanvas=new TCanvas(
"matchCanvas",
"Track match summary",
900,
700
);

matchCanvas->SetGrid();
hMatchType->Draw("HIST TEXT");

TString matchPdf;

matchPdf.Form(
"channel_%02d_match_summary.pdf",
watchChannel
);

matchCanvas->SaveAs(matchPdf);

// Save histograms and canvases.
TString rootName;

rootName.Form(
"channel_%02d_track_verification.root",
watchChannel
);

TFile *outputFile=new TFile(
rootName,
"RECREATE"
);

hPredictedChannel->Write();
hPredictedVsMeasured->Write();
hMatchType->Write();
channelCanvas->Write();
comparisonCanvas->Write();
matchCanvas->Write();
outputFile->Close();

// Print the final summary.
cout<<endl;
cout<<"=================================================="<<endl;
cout<<"Track verification finished for channel "
<<watchChannel<<endl;
cout<<"Entries containing measured channel: "
<<measuredEvents<<endl;
cout<<"Projected tracks tested: "
<<projectedTracks<<endl;
cout<<"Exact matches: "
<<exactMatches<<endl;
cout<<"Adjacent matches: "
<<adjacentMatches<<endl;
cout<<"No matches: "
<<noMatches<<endl;
cout<<"Measured entries without tracks: "
<<eventsWithoutTracks<<endl;
cout<<"Oversized entries skipped: "
<<oversizedEvents<<endl;
cout<<"Created "<<csvName<<endl;
cout<<"Created "<<channelPdf<<endl;
cout<<"Created "<<comparisonPdf<<endl;
cout<<"Created "<<matchPdf<<endl;
cout<<"Created "<<rootName<<endl;
cout<<"=================================================="<<endl;

file->Close();
}

