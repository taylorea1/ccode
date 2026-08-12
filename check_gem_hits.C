// #include <TFile.h>
// #include <TTree.h>
// #include <TTreeReader.h>
// #include <TTreeReaderArray.h>
// #include <iostream>

// using namespace std;

// void check_gem_hits(
// const char *inputFile="/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root")
// {
// TFile *file=TFile::Open(inputFile,"READ");

// if(!file||file->IsZombie()){
// cerr<<"Could not open file."<<endl;
// return;
// }

// TTree *T=nullptr;
// file->GetObject("T",T);

// if(!T){
// cerr<<"Could not find tree T."<<endl;
// return;
// }

// TTreeReader reader(T);

// TTreeReaderArray<Double_t> module(reader,"sbs.gemCeR.hit.module");
// TTreeReaderArray<Double_t> trackindex(reader,"sbs.gemCeR.hit.trackindex");
// TTreeReaderArray<Double_t> ustriphi(reader,"sbs.gemCeR.hit.ustriphi");

// cout<<"module status: "<<module.GetSetupStatus()<<endl;
// cout<<"trackindex status: "<<trackindex.GetSetupStatus()<<endl;
// cout<<"ustriphi status: "<<ustriphi.GetSetupStatus()<<endl;

// Long64_t event=0;

// while(reader.Next()&&event<30){
// cout<<"Event "<<event
// <<": modules="<<module.GetSize()
// <<", trackindices="<<trackindex.GetSize()
// <<", ustriphi="<<ustriphi.GetSize()
// <<endl;

// int numberOfHits=module.GetSize();

// if((int)trackindex.GetSize()<numberOfHits)
// numberOfHits=trackindex.GetSize();

// if((int)ustriphi.GetSize()<numberOfHits)
// numberOfHits=ustriphi.GetSize();

// for(int hit=0;hit<numberOfHits;hit++){
// cout<<"  hit "<<hit
// <<", module="<<module[hit]
// <<", trackindex="<<trackindex[hit]
// <<", ustriphi="<<ustriphi[hit]
// <<endl;
// }

// event++;
// }

// file->Close();
// }



//===============================================================
// File: check_gem_hits.C
//
// Checks whether the CeR GEM hit arrays contain readable data.
// These branches are stored as variable-length Double_t arrays.
//===============================================================

#include <TFile.h>
#include <TTree.h>
#include <iostream>

using namespace std;

void check_gem_hits(
const char *inputFile="/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root")
{
const int MAXHITS=100;

TFile *file=TFile::Open(inputFile,"READ");

if(!file||file->IsZombie()){
cerr<<"ERROR: Could not open "<<inputFile<<endl;
return;
}

TTree *T=nullptr;
file->GetObject("T",T);

if(!T){
cerr<<"ERROR: Could not find tree T."<<endl;
file->Close();
return;
}

// Ndata branches contain the number of values in each array.
Int_t nModule=0;
Int_t nTrackIndex=0;
Int_t nUStripHi=0;

// Actual hit arrays.
// The branch definitions end in /D, so use Double_t.
Double_t module[MAXHITS];
Double_t trackindex[MAXHITS];
Double_t ustriphi[MAXHITS];

// Connect the array counters.
T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.module",
&nModule
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.trackindex",
&nTrackIndex
);

T->SetBranchAddress(
"Ndata.sbs.gemCeR.hit.ustriphi",
&nUStripHi
);

// Connect the actual arrays.
T->SetBranchAddress(
"sbs.gemCeR.hit.module",
module
);

T->SetBranchAddress(
"sbs.gemCeR.hit.trackindex",
trackindex
);

T->SetBranchAddress(
"sbs.gemCeR.hit.ustriphi",
ustriphi
);

Long64_t numberOfEvents=T->GetEntries();
Int_t printedEvents=0;

for(Long64_t event=0;
event<numberOfEvents&&printedEvents<30;
event++){
    T->GetEntry(event);

    if(nModule<=0)continue;

    printedEvents++;

    Int_t numberOfHits=nModule;

    if(nTrackIndex<numberOfHits)
        numberOfHits=nTrackIndex;

    if(nUStripHi<numberOfHits)
        numberOfHits=nUStripHi;

    if(numberOfHits>MAXHITS){
        cerr<<"ERROR: Event "<<event
            <<" exceeds MAXHITS."<<endl;
        continue;
    }

    cout<<"Event "<<event
        <<": modules="<<nModule
        <<", trackindices="<<nTrackIndex
        <<", ustriphi="<<nUStripHi
        <<endl;

    for(Int_t hit=0;hit<numberOfHits;hit++){
        cout<<"  hit "<<hit
            <<", module="<<module[hit]
            <<", trackindex="<<trackindex[hit]
            <<", ustriphi="<<ustriphi[hit]
            <<endl;
    }
}

file->Close();
}