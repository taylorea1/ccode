//===============================================================
// File: all_track_channel_matches.C
//
// Reads every TTree entry once.
//
// For each event:
//   1. Reads all active-analyzer ADC and TDC hits.
//   2. Matches ADC and TDC entries by analyzer channel.
//   3. Reads all reconstructed CeR GEM tracks.
//   4. Projects each track to the analyzer plane.
//   5. Converts each projected position to channel 0-31.
//   6. Compares the predicted channel with measured channels.
//   7. Records exact, adjacent, unmatched, and outside results.
//   8. Saves matched event/track rows and a 32-channel summary.
//
// All output files are saved in:
// /home/amarit/rootdisplay/pdfs/track_channel_results
//
// IMPORTANT:
// Replace the analyzer geometry defaults with the real analyzer
// position, dimensions, and orientation.
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
#include <TSystem.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

// Number of active-analyzer channels.
const Int_t NUMBEROFCHANNELS=32;

// Maximum number of reconstructed tracks in one event.
const Int_t MAXTRACKS=100;

// Value used when an ADC hit has no matching TDC hit.
const Double_t MISSINGTDC=-999999.0;

// Stores one measured active-analyzer hit.
struct AnalyzerHit{
Int_t channel;
Double_t adc;
Double_t tdc;
};

// Return the first usable leaf from a branch.
TLeaf *GetDataLeafAllMatches(TTree *T,const char *branchName){
TLeaf *leaf=T->GetLeaf(branchName);

if(leaf)
return leaf;

TBranch *branch=T->GetBranch(branchName);

if(!branch||
!branch->GetListOfLeaves()||
branch->GetListOfLeaves()->GetEntries()==0)
return nullptr;

return (TLeaf *)branch->GetListOfLeaves()->At(0);
}

// Match ADC and TDC hits by analyzer channel.
//
// Multiple ADC entries are allowed for one channel.
// Each TDC entry is assigned to the first unmatched ADC entry
// from the same analyzer channel.
vector<AnalyzerHit> BuildAnalyzerHitsAllMatches(
TLeaf *adcValues,
TLeaf *adcChannels,
TLeaf *tdcValues,
TLeaf *tdcChannels)
{
Int_t adcCount=min(
adcValues->GetNdata(),
adcChannels->GetNdata()
);

Int_t tdcCount=min(
tdcValues->GetNdata(),
tdcChannels->GetNdata()
);

vector<AnalyzerHit> hits;
hits.reserve(adcCount);

// Store the hit indices belonging to each ADC channel.
unordered_map<Int_t,vector<size_t>> adcIndices;

// Read all valid ADC hits.
for(Int_t adcIndex=0;adcIndex<adcCount;adcIndex++){
Int_t channel=(Int_t)lround(
adcChannels->GetValue(adcIndex)
);

if(channel<0||channel>=NUMBEROFCHANNELS)
continue;

AnalyzerHit hit;
hit.channel=channel;
hit.adc=adcValues->GetValue(adcIndex);
hit.tdc=MISSINGTDC;

adcIndices[channel].push_back(hits.size());
hits.push_back(hit);
}

// Track the next unmatched ADC entry for each channel.
unordered_map<Int_t,size_t> nextADCIndex;

// Match each TDC hit with an ADC hit from the same channel.
for(Int_t tdcIndex=0;tdcIndex<tdcCount;tdcIndex++){
Int_t channel=(Int_t)lround(
tdcChannels->GetValue(tdcIndex)
);

auto found=adcIndices.find(channel);

if(found==adcIndices.end())
continue;

size_t &offset=nextADCIndex[channel];

if(offset<found->second.size()){
size_t hitIndex=found->second[offset];

hits[hitIndex].tdc=
tdcValues->GetValue(tdcIndex);

offset++;
}
}

// Remove ADC hits that did not receive a matching TDC hit.
hits.erase(
remove_if(
hits.begin(),
hits.end(),
[](const AnalyzerHit &hit){
return hit.tdc==MISSINGTDC;
}
),
hits.end()
);

// Sort the measured hits by TDC time.
sort(
hits.begin(),
hits.end(),
[](const AnalyzerHit &a,const AnalyzerHit &b){
return a.tdc<b.tdc;
}
);

return hits;
}

// Return the detector-grid column for channel 0-31.
Int_t ChannelColumnAllMatches(Int_t channel){
return channel%8;
}

// Return the detector-grid row for channel 0-31.
Int_t ChannelRowAllMatches(Int_t channel){
return channel/8;
}

// Determine whether two channels are neighboring blocks.
//
// Diagonal neighbors are included.
Bool_t ChannelsAreAdjacentAllMatches(
Int_t channel1,
Int_t channel2)
{
if(channel1<0||channel1>=NUMBEROFCHANNELS||
channel2<0||channel2>=NUMBEROFCHANNELS)
return false;

Int_t columnDifference=abs(
ChannelColumnAllMatches(channel1)-
ChannelColumnAllMatches(channel2)
);

Int_t rowDifference=abs(
ChannelRowAllMatches(channel1)-
ChannelRowAllMatches(channel2)
);

return columnDifference<=1&&
rowDifference<=1&&
channel1!=channel2;
}

// Convert a projected x/y position into analyzer channel 0-31.
//
// Analyzer layout:
//   8 columns
//   4 rows
//
// Channel numbering:
//   row 0: 0-7
//   row 1: 8-15
//   row 2: 16-23
//   row 3: 24-31
Int_t PositionToChannelAllMatches(
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

// Reject tracks outside the active analyzer.
if(projectedX<analyzerXMin||
projectedX>=analyzerXMax||
projectedY<analyzerYMin||
projectedY>=analyzerYMax)
return -1;

// Calculate the physical size of one channel.
Double_t channelWidth=
(analyzerXMax-analyzerXMin)/numberOfColumns;

Double_t channelHeight=
(analyzerYMax-analyzerYMin)/numberOfRows;

// Determine the projected detector column and row.
Int_t column=(Int_t)(
(projectedX-analyzerXMin)/channelWidth
);

Int_t row=(Int_t)(
(projectedY-analyzerYMin)/channelHeight
);

// Reverse detector numbering when required.
if(reverseX)
column=numberOfColumns-1-column;

if(reverseY)
row=numberOfRows-1-row;

// Reject invalid channel indices.
if(column<0||column>=numberOfColumns||
row<0||row>=numberOfRows)
return -1;

return row*numberOfColumns+column;
}

void all_trackmatches(
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
// Folder where every output file will be saved.
TString outputDirectory=
"/home/amarit/rootdisplay/pdfs/track_channel_results";

// Create the output folder if it does not exist.
//
// The true argument creates missing parent folders too.
Int_t mkdirStatus=
gSystem->mkdir(outputDirectory,true);

// Check whether the directory is accessible.
if(mkdirStatus!=0&&
gSystem->AccessPathName(outputDirectory)){
cerr<<"ERROR: Could not create output directory "
<<outputDirectory<<endl;
return;
}

cout<<"Output directory: "
<<outputDirectory<<endl;

// Validate the analyzer geometry.
if(analyzerXMax<=analyzerXMin||
analyzerYMax<=analyzerYMin){
cerr<<"ERROR: Invalid analyzer coordinate limits."<<endl;
return;
}

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

// Get active-analyzer ADC and TDC leaves.
TLeaf *adcValues=
GetDataLeafAllMatches(
T,
"sbs.activeAna_adc.a"
);

TLeaf *adcChannels=
GetDataLeafAllMatches(
T,
"sbs.activeAna_adc.adcelemID"
);

TLeaf *tdcValues=
GetDataLeafAllMatches(
T,
"sbs.activeAna_tdc.hits.t"
);

TLeaf *tdcChannels=
GetDataLeafAllMatches(
T,
"sbs.activeAna_tdc.tdcelemID"
);

if(!adcValues||
!adcChannels||
!tdcValues||
!tdcChannels){
cerr<<"ERROR: Missing active-analyzer ADC or TDC branches."
<<endl;

file->Close();
return;
}

// Required CeR GEM track branches.
const char *trackBranches[]={
"Ndata.sbs.gemCeR.track.x",
"Ndata.sbs.gemCeR.track.y",
"Ndata.sbs.gemCeR.track.xp",
"Ndata.sbs.gemCeR.track.yp",
"sbs.gemCeR.track.x",
"sbs.gemCeR.track.y",
"sbs.gemCeR.track.xp",
"sbs.gemCeR.track.yp"
};

Int_t trackBranchCount=
sizeof(trackBranches)/sizeof(trackBranches[0]);

// Stop if any required track branch is missing.
for(Int_t branchIndex=0;
branchIndex<trackBranchCount;
branchIndex++){
if(!T->GetBranch(trackBranches[branchIndex])){
cerr<<"ERROR: Missing branch "
<<trackBranches[branchIndex]<<endl;

file->Close();
return;
}
}

// Number of values stored in each track array.
Int_t nTrackX=0;
Int_t nTrackY=0;
Int_t nTrackXp=0;
Int_t nTrackYp=0;

// Reconstructed CeR track arrays.
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

// Create complete output paths.
TString csvName=
outputDirectory+
"/all_track_channel_matches.csv";

TString rootName=
outputDirectory+
"/all_track_channel_matches.root";

TString comparisonPdf=
outputDirectory+
"/predicted_vs_measured_channels.pdf";

TString comparisonPng=
outputDirectory+
"/predicted_vs_measured_channels.png";

TString summaryPdf=
outputDirectory+
"/match_summary_by_channel.pdf";

TString summaryPng=
outputDirectory+
"/match_summary_by_channel.png";

TString channelPdf=
outputDirectory+
"/projected_measured_exact_by_channel.pdf";

TString channelPng=
outputDirectory+
"/projected_measured_exact_by_channel.png";

TString matchedEventsName=
outputDirectory+
"/matched_events.csv";

TString summaryCsvName=
outputDirectory+
"/channel_match_summary.csv";

TString summaryTextName=
outputDirectory+
"/channel_match_summary.txt";

// Create one CSV containing all channel comparisons.
ofstream csv(csvName.Data());

if(!csv){
cerr<<"ERROR: Could not create "
<<csvName<<endl;

file->Close();
return;
}

// Create a smaller file containing only exact and adjacent matches.
ofstream matchedEvents(matchedEventsName.Data());

if(!matchedEvents){
cerr<<"ERROR: Could not create "
<<matchedEventsName<<endl;

csv.close();
file->Close();
return;
}

// Create the per-channel CSV summary.
ofstream summaryCSV(summaryCsvName.Data());

if(!summaryCSV){
cerr<<"ERROR: Could not create "
<<summaryCsvName<<endl;

matchedEvents.close();
csv.close();
file->Close();
return;
}

// Create a human-readable text summary.
ofstream summaryText(summaryTextName.Data());

if(!summaryText){
cerr<<"ERROR: Could not create "
<<summaryTextName<<endl;

summaryCSV.close();
matchedEvents.close();
csv.close();
file->Close();
return;
}

// Write the CSV header.
csv<<"Entry,Track,"
<<"TrackX,TrackY,TrackXp,TrackYp,"
<<"ProjectedX,ProjectedY,"
<<"PredictedChannel,"
<<"MeasuredChannel,ADC,TDC,"
<<"ColumnDifference,RowDifference,"
<<"Match\n";

matchedEvents
<<"Entry,Track,PredictedChannel,MeasuredChannel,"
<<"ProjectedX,ProjectedY,ADC,TDC,"
<<"ColumnDifference,RowDifference,Match\n";

summaryCSV
<<"Channel,Projected,Measured,Exact,Adjacent,"
<<"ExactOrAdjacent,NoMatch,ExactPercent,"
<<"ExactOrAdjacentPercent\n";

// Predicted channel versus measured channel.
//
// Exact matches should form a diagonal.
TH2D *hPredictedVsMeasured=new TH2D(
"hPredictedVsMeasured",
"Predicted versus measured analyzer channels;"
"Projected GEM channel;"
"Measured active-analyzer channel",
32,-0.5,31.5,
32,-0.5,31.5
);

// Number of projected tracks landing in each channel.
TH1D *hProjectedTracks=new TH1D(
"hProjectedTracks",
"Projected GEM tracks per analyzer channel;"
"Projected analyzer channel;"
"Tracks",
32,-0.5,31.5
);

// Number of measured ADC/TDC hits in each channel.
TH1D *hMeasuredHits=new TH1D(
"hMeasuredHits",
"Measured active-analyzer hits per channel;"
"Measured analyzer channel;"
"ADC/TDC hits",
32,-0.5,31.5
);

// Number of exact matches in each predicted channel.
TH1D *hExactMatches=new TH1D(
"hExactMatches",
"Exact GEM-track matches per analyzer channel;"
"Analyzer channel;"
"Exact matches",
32,-0.5,31.5
);

// Number of adjacent matches in each predicted channel.
TH1D *hAdjacentMatches=new TH1D(
"hAdjacentMatches",
"Adjacent GEM-track matches per analyzer channel;"
"Projected analyzer channel;"
"Adjacent matches",
32,-0.5,31.5
);

// Number of tracks with no corresponding measured channel.
TH1D *hNoMatches=new TH1D(
"hNoMatches",
"Projected tracks without analyzer matches;"
"Projected analyzer channel;"
"No matches",
32,-0.5,31.5
);

// Overall match-category summary.
TH1D *hMatchSummary=new TH1D(
"hMatchSummary",
"All track-to-analyzer match results;"
"Match type;"
"Tracks",
4,0.5,4.5
);

hMatchSummary->GetXaxis()->SetBinLabel(1,"Exact");
hMatchSummary->GetXaxis()->SetBinLabel(2,"Adjacent");
hMatchSummary->GetXaxis()->SetBinLabel(3,"No match");
hMatchSummary->GetXaxis()->SetBinLabel(4,"Outside");

// Analysis counters.
Long64_t numberOfEntries=T->GetEntries();
Long64_t totalTracks=0;
Long64_t tracksInsideAnalyzer=0;
Long64_t tracksOutsideAnalyzer=0;
Long64_t exactMatches=0;
Long64_t adjacentMatches=0;
Long64_t noMatches=0;
Long64_t measuredHitsTotal=0;
Long64_t entriesWithoutTracks=0;
Long64_t oversizedEntries=0;

cout<<"=================================================="<<endl;
cout<<"All-channel GEM track verification"<<endl;
cout<<"Entries: "<<numberOfEntries<<endl;
cout<<"Analyzer z: "<<analyzerZ<<endl;
cout<<"Analyzer x range: "
<<analyzerXMin<<" to "<<analyzerXMax<<endl;
cout<<"Analyzer y range: "
<<analyzerYMin<<" to "<<analyzerYMax<<endl;
cout<<"=================================================="<<endl;

// Loop over every TTree entry once.
for(Long64_t entry=0;
entry<numberOfEntries;
entry++){
if(T->GetEntry(entry)<=0)
continue;

// Build all measured active-analyzer hits.
vector<AnalyzerHit> measuredHits=
BuildAnalyzerHitsAllMatches(
adcValues,
adcChannels,
tdcValues,
tdcChannels
);

// Count measured hits by channel.
for(size_t hitIndex=0;
hitIndex<measuredHits.size();
hitIndex++){
hMeasuredHits->Fill(
measuredHits[hitIndex].channel
);

measuredHitsTotal++;
}

// Protect the fixed-size track arrays.
if(nTrackX>MAXTRACKS||
nTrackY>MAXTRACKS||
nTrackXp>MAXTRACKS||
nTrackYp>MAXTRACKS){
cerr<<"WARNING: Entry "<<entry
<<" exceeds MAXTRACKS and was skipped."
<<endl;

oversizedEntries++;
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

// Count entries that have no reconstructed CeR tracks.
if(numberOfTracks<=0){
entriesWithoutTracks++;
continue;
}

// Loop over every reconstructed CeR track.
for(Int_t track=0;
track<numberOfTracks;
track++){
totalTracks++;

// Read reconstructed track parameters.
Double_t x0=trackX[track];
Double_t y0=trackY[track];
Double_t xp=trackXp[track];
Double_t yp=trackYp[track];

// Project the track to the active-analyzer plane.
Double_t deltaZ=
analyzerZ-zReference;

Double_t projectedX=
x0+xp*deltaZ;

Double_t projectedY=
y0+yp*deltaZ;

// Convert the projected position into channel 0-31.
Int_t predictedChannel=
PositionToChannelAllMatches(
projectedX,
projectedY,
analyzerXMin,
analyzerXMax,
analyzerYMin,
analyzerYMax,
reverseX,
reverseY
);

// Record tracks that miss the active analyzer.
if(predictedChannel<0){
tracksOutsideAnalyzer++;
hMatchSummary->Fill(4);

csv<<entry<<","
<<track<<","
<<x0<<","
<<y0<<","
<<xp<<","
<<yp<<","
<<projectedX<<","
<<projectedY<<","
<<-1<<","
<<-1<<","
<<"nan,nan,"
<<-1<<","
<<-1<<","
<<"outside_analyzer\n";

continue;
}

tracksInsideAnalyzer++;
hProjectedTracks->Fill(predictedChannel);

// Store the best measured match for this track.
Int_t bestMeasuredChannel=-1;
Int_t bestHitIndex=-1;
Int_t bestColumnDifference=999;
Int_t bestRowDifference=999;
Int_t bestDistance=999;

TString matchType="no_match";

// Compare this projected track with every measured hit.
for(size_t hitIndex=0;
hitIndex<measuredHits.size();
hitIndex++){
Int_t measuredChannel=
measuredHits[hitIndex].channel;

Int_t columnDifference=abs(
ChannelColumnAllMatches(predictedChannel)-
ChannelColumnAllMatches(measuredChannel)
);

Int_t rowDifference=abs(
ChannelRowAllMatches(predictedChannel)-
ChannelRowAllMatches(measuredChannel)
);

Int_t distance=
columnDifference+rowDifference;

// Exact match has highest priority.
if(measuredChannel==predictedChannel){
bestMeasuredChannel=measuredChannel;
bestHitIndex=(Int_t)hitIndex;
bestColumnDifference=0;
bestRowDifference=0;
bestDistance=0;
matchType="exact";
break;
}

// Store the nearest adjacent measured channel.
if(ChannelsAreAdjacentAllMatches(
predictedChannel,
measuredChannel
)&&distance<bestDistance){
bestMeasuredChannel=measuredChannel;
bestHitIndex=(Int_t)hitIndex;
bestColumnDifference=columnDifference;
bestRowDifference=rowDifference;
bestDistance=distance;
matchType="adjacent";
}
}

// Process the final match classification.
if(matchType=="exact"){
exactMatches++;
hExactMatches->Fill(predictedChannel);
hMatchSummary->Fill(1);
}
else if(matchType=="adjacent"){
adjacentMatches++;
hAdjacentMatches->Fill(predictedChannel);
hMatchSummary->Fill(2);
}
else{
noMatches++;
hNoMatches->Fill(predictedChannel);
hMatchSummary->Fill(3);
}

// Fill predicted-versus-measured channel histogram.
if(bestMeasuredChannel>=0){
hPredictedVsMeasured->Fill(
predictedChannel,
bestMeasuredChannel
);
}

// Write this track comparison to the CSV.
csv<<entry<<","
<<track<<","
<<x0<<","
<<y0<<","
<<xp<<","
<<yp<<","
<<projectedX<<","
<<projectedY<<","
<<predictedChannel<<","
<<bestMeasuredChannel<<",";

if(bestHitIndex>=0){
csv<<measuredHits[bestHitIndex].adc<<","
<<measuredHits[bestHitIndex].tdc<<",";
}
else{
csv<<"nan,nan,";
}

csv<<bestColumnDifference<<","
<<bestRowDifference<<","
<<matchType<<'\n';

// Save exact and adjacent matches in a smaller lookup file.
if((matchType=="exact"||matchType=="adjacent")&&
bestHitIndex>=0){
matchedEvents
<<entry<<","
<<track<<","
<<predictedChannel<<","
<<bestMeasuredChannel<<","
<<projectedX<<","
<<projectedY<<","
<<measuredHits[bestHitIndex].adc<<","
<<measuredHits[bestHitIndex].tdc<<","
<<bestColumnDifference<<","
<<bestRowDifference<<","
<<matchType<<'\n';
}

// Print exact matches while the macro runs.
if(matchType=="exact"){
cout<<"Entry "<<entry
<<", track "<<track
<<", channel "<<predictedChannel
<<", exact match"<<endl;
}
}
}

csv.close();
matchedEvents.close();
gStyle->SetOptStat(0);

cout<<endl;
cout<<"===================================================================="<<endl;
cout<<"                     CHANNEL MATCH SUMMARY"<<endl;
cout<<"===================================================================="<<endl;
cout<<"Channel  Projected  Measured  Exact  Adjacent  Exact+Adj  NoMatch"<<endl;
cout<<"--------------------------------------------------------------------"<<endl;

summaryText
<<"====================================================================\n"
<<"                     CHANNEL MATCH SUMMARY\n"
<<"====================================================================\n"
<<"Channel  Projected  Measured  Exact  Adjacent  Exact+Adj  NoMatch\n"
<<"--------------------------------------------------------------------\n";

for(Int_t channel=0;channel<NUMBEROFCHANNELS;channel++){
Long64_t projected=(Long64_t)hProjectedTracks->
GetBinContent(channel+1);

Long64_t measured=(Long64_t)hMeasuredHits->
GetBinContent(channel+1);

Long64_t exact=(Long64_t)hExactMatches->
GetBinContent(channel+1);

Long64_t adjacent=(Long64_t)hAdjacentMatches->
GetBinContent(channel+1);

Long64_t noMatch=(Long64_t)hNoMatches->
GetBinContent(channel+1);

Long64_t exactOrAdjacent=exact+adjacent;

Double_t exactPercent=
projected>0?100.0*exact/projected:0.0;

Double_t exactOrAdjacentPercent=
projected>0?100.0*exactOrAdjacent/projected:0.0;

cout.width(7);
cout<<channel;
cout.width(11);
cout<<projected;
cout.width(10);
cout<<measured;
cout.width(7);
cout<<exact;
cout.width(10);
cout<<adjacent;
cout.width(11);
cout<<exactOrAdjacent;
cout.width(9);
cout<<noMatch;
cout<<endl;

summaryText.width(7);
summaryText<<channel;
summaryText.width(11);
summaryText<<projected;
summaryText.width(10);
summaryText<<measured;
summaryText.width(7);
summaryText<<exact;
summaryText.width(10);
summaryText<<adjacent;
summaryText.width(11);
summaryText<<exactOrAdjacent;
summaryText.width(9);
summaryText<<noMatch;
summaryText<<'\n';

summaryCSV
<<channel<<","
<<projected<<","
<<measured<<","
<<exact<<","
<<adjacent<<","
<<exactOrAdjacent<<","
<<noMatch<<","
<<exactPercent<<","
<<exactOrAdjacentPercent<<'\n';
}

cout<<"===================================================================="<<endl;

summaryText
<<"====================================================================\n";

summaryCSV.close();
summaryText.close();

// Draw predicted versus measured channels.
TCanvas *comparisonCanvas=new TCanvas(
"comparisonCanvas",
"Predicted versus measured channels",
900,
800
);

comparisonCanvas->SetRightMargin(0.15);
comparisonCanvas->SetGrid();
hPredictedVsMeasured->Draw("COLZ TEXT");

comparisonCanvas->SaveAs(comparisonPdf);
comparisonCanvas->SaveAs(comparisonPng);

// Draw overall match categories.
TCanvas *summaryCanvas=new TCanvas(
"summaryCanvas",
"Track match summary",
900,
700
);

summaryCanvas->SetGrid();
hMatchSummary->Draw("HIST TEXT");

summaryCanvas->SaveAs(summaryPdf);
summaryCanvas->SaveAs(summaryPng);

// Draw projected, measured, and exact-match counts.
TCanvas *channelCanvas=new TCanvas(
"channelCanvas",
"Channel comparison",
1100,
750
);

channelCanvas->SetGrid();

hProjectedTracks->SetLineColor(kBlue);
hMeasuredHits->SetLineColor(kRed);
hExactMatches->SetLineColor(kGreen+2);

hProjectedTracks->SetLineWidth(2);
hMeasuredHits->SetLineWidth(2);
hExactMatches->SetLineWidth(2);

hProjectedTracks->Draw("HIST");
hMeasuredHits->Draw("HIST SAME");
hExactMatches->Draw("HIST SAME");

channelCanvas->BuildLegend();

channelCanvas->SaveAs(channelPdf);
channelCanvas->SaveAs(channelPng);

// Save all histograms and canvases in one ROOT file.
TFile *outputFile=new TFile(
rootName,
"RECREATE"
);

hPredictedVsMeasured->Write();
hProjectedTracks->Write();
hMeasuredHits->Write();
hExactMatches->Write();
hAdjacentMatches->Write();
hNoMatches->Write();
hMatchSummary->Write();

comparisonCanvas->Write();
summaryCanvas->Write();
channelCanvas->Write();

outputFile->Close();

// Print final results.
cout<<endl;
cout<<"=================================================="<<endl;
cout<<"All-channel verification finished"<<endl;
cout<<"Total reconstructed tracks: "
<<totalTracks<<endl;
cout<<"Tracks inside analyzer: "
<<tracksInsideAnalyzer<<endl;
cout<<"Tracks outside analyzer: "
<<tracksOutsideAnalyzer<<endl;
cout<<"Measured ADC/TDC hits: "
<<measuredHitsTotal<<endl;
cout<<"Exact matches: "
<<exactMatches<<endl;
cout<<"Adjacent matches: "
<<adjacentMatches<<endl;
cout<<"No matches: "
<<noMatches<<endl;
cout<<"Entries without tracks: "
<<entriesWithoutTracks<<endl;
cout<<"Oversized entries skipped: "
<<oversizedEntries<<endl;
cout<<"Created "<<csvName<<endl;
cout<<"Created "<<matchedEventsName<<endl;
cout<<"Created "<<summaryCsvName<<endl;
cout<<"Created "<<summaryTextName<<endl;
cout<<"Created "<<rootName<<endl;
cout<<"Created "<<comparisonPdf<<endl;
cout<<"Created "<<summaryPdf<<endl;
cout<<"Created "<<channelPdf<<endl;
cout<<"=================================================="<<endl;

file->Close();
}
