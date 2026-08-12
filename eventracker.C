// //--------------------------------------------------------------
// // Includes
// //--------------------------------------------------------------

// #include <iostream>
// #include <vector>
// #include <algorithm>
// #include <fstream>
// #include <TPolyMarker3D.h>
// #include <TPolyLine3D.h>
// #include <TView.h>
// #include <TMarker.h>
// #include <TLatex.h>
// #include <TH2F.h>
// #include <TLine.h>
// #include <TColor.h>

// using namespace std;

// //----------------------------------------------------------
// // Proton identification parameters
// //----------------------------------------------------------

// // ADC calibration factor
// // Replace with your measured calibration

// Double_t adcGain = 1.0;

// // Proton energy window

// Double_t protonEnergyMin = 5.0;   // MeV
// Double_t protonEnergyMax = 200.0; // MeV

// //--------------------------------------------------------------
// // Structure used to hold one detector hit
// //--------------------------------------------------------------

// struct Hit
// {
//     Int_t    channel;   // Electronics channel (0-31)
//     Double_t adc;       // ADC value
//     Double_t tdc;       // TDC value
// };

// //--------------------------------------------------------------
// // Structure for detector geometry map
// //--------------------------------------------------------------

// struct ChannelPosition
// {
//     Int_t channel;

//     Double_t x;   // physical x position
//     Double_t y;   // physical y position
//     Double_t z;   // physical z position
// };

// //--------------------------------------------------------------
// // Define channel geometry
// //
// // Replace these values with your actual detector geometry
// //--------------------------------------------------------------

// ChannelPosition geometry[32] =
// {

//     {0,  0, 0, 0},
//     {1,  1, 0, 0},
//     {2,  2, 0, 0},
//     {3,  3, 0, 0},
//     {4,  4, 0, 0},
//     {5,  5, 0, 0},
//     {6,  6, 0, 0},
//     {7,  7, 0, 0},

//     {8,  0, 1, 0},
//     {9,  1, 1, 0},
//     {10, 2, 1, 0},
//     {11, 3, 1, 0},
//     {12, 4, 1, 0},
//     {13, 5, 1, 0},
//     {14, 6, 1, 0},
//     {15, 7, 1, 0},

//     {16, 0, 2, 0},
//     {17, 1, 2, 0},
//     {18, 2, 2, 0},
//     {19, 3, 2, 0},
//     {20, 4, 2, 0},
//     {21, 5, 2, 0},
//     {22, 6, 2, 0},
//     {23, 7, 2, 0},

//     {24, 0, 3, 0},
//     {25, 1, 3, 0},
//     {26, 2, 3, 0},
//     {27, 3, 3, 0},
//     {28, 4, 3, 0},
//     {29, 5, 3, 0},
//     {30, 6, 3, 0},
//     {31, 7, 3, 0}

// };

// //--------------------------------------------------------------
// // Function to retrieve channel position
// //--------------------------------------------------------------

// ChannelPosition GetPosition(Int_t channel)
// {
//     return geometry[channel];
// }

// //==============================================================
// // Main macro
// //==============================================================

// void eventracker(Int_t watchChannel)
// {

//     //----------------------------------------------------------
//     // Open ROOT file
//     //----------------------------------------------------------

//     TFile *f = TFile::Open(
//         "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root"
//     );

//     if (!f || f->IsZombie()) {

//         cout << "Error opening file!" << endl;
//         return;

//     }

//     //----------------------------------------------------------
//     // Get tree
//     //----------------------------------------------------------

//     TTree *T = (TTree*)f->Get("T");

//     if (!T) {

//         cout << "Error: TTree not found!" << endl;
//         return;

//     }

// //----------------------------------------------------------
// // Branch variables
// //----------------------------------------------------------

// Int_t adc_mult;
// Int_t tdc_mult;

// // ADC values

// Double_t adc_value[128];

// // ADC element IDs are Double_t in your tree

// Double_t adc_channel[128];

// // TDC values

// Double_t tdc_value[128];

// // TDC element IDs are Double_t in your tree

// Double_t tdc_channel[128];

//     //----------------------------------------------------------
//     // Connect branches
//     //----------------------------------------------------------

//     T->SetBranchAddress("Ndata.sbs.activeAna_adc.a_mult", &adc_mult);
//     T->SetBranchAddress("sbs.activeAna_adc.a", adc_value);
//     T->SetBranchAddress("sbs.activeAna_adc.adcelemID", adc_channel);
//     T->SetBranchAddress("Ndata.sbs.activeAna_tdc.tdc_mult", &tdc_mult);
//     T->SetBranchAddress( "sbs.activeAna_tdc.hits.t", tdc_value);
//     T->SetBranchAddress("sbs.activeAna_tdc.tdcelemID", tdc_channel);

//     ChannelPosition watchPosition = GetPosition(watchChannel);

//     cout << "Tracking channel "
//          << watchChannel
//          << " at position ("
//          << watchPosition.x << ", "
//          << watchPosition.y << ", "
//          << watchPosition.z << ")"
//          << endl;

//     //----------------------------------------------------------
//     // Create CSV file
//     //----------------------------------------------------------

//     ofstream csv(Form("channel_%02d_results.csv", watchChannel));

//     csv << "Event, "
//         << "CurrentChannel, "
//         << "CurrentX, "
//         << "CurrentY, "
//         << "CurrentZ, "
//         << "PreviousChannel, "
//         << "PreviousX, "
//         << "PreviousY, "
//         << "PreviousZ, "
//         << "NextChannel, "
//         << "NextX, "
//         << "NextY, "
//         << "NextZ, "
//         << "ADC, "
//         << "TDC, "
//         << "PID"
//         << endl;


// TCanvas *c1 = new TCanvas("c1", "Detector Event Display",
//         800, 600);

// TH2F *detector = new TH2F( "detector", "Detector;X;Y",
//         8, -0.5, 7.5,
//         4, -0.5, 3.5);
// detector->SetStats(0);

// //----------------------------------------------------------
// // Draw all detector channels
// //----------------------------------------------------------

// for(int ch=0; ch<32; ch++)
//     {
//         ChannelPosition p = GetPosition(ch);

//         m->SetMarkerColor(kGray+2);
//         m->SetMarkerSize(1.5);
//         m->Draw("same");

//         TLatex *label = new TLatex(
//             p.x+0.1,
//             p.y+0.1,
//             Form("%d", ch));

//         label->SetTextSize(0.03);
//         label->Draw("same");
//     }

// TH2D *pid = new TH2D( "pid", "PID;TDC;ADC",
//     100, 0, 2000,
//     100, -100, 500);

// //----------------------------------------------------------
// // PID Canvas
// //----------------------------------------------------------

//     TCanvas *cpid =
//         new TCanvas("cpid", "ADC vs TDC PID",
//             800, 700);
//     pid->Draw("COLZ");

// //----------------------------------------------------------
// // Event loop
// //----------------------------------------------------------

// Long64_t nEntries = T->GetEntries();

//     cout << "Total Events = " << nEntries << endl;
//     // Statistics variables
//     double adcSum = 0.0;
//     double adcSum2 = 0.0;

//     double tdcSum = 0.0;
//     double tdcSum2 = 0.0;

//     long long count = 0;
// //------------------------------------------------------
// // First pass: determine ADC/TDC statistics
// //------------------------------------------------------

//     for(Long64_t event=0; event<nEntries; event++)
//     {
//         T->GetEntry(event);

// //------------------------------------------------------
// // Clear detector display
// //------------------------------------------------------

// c1->cd();
// c1->Modified();
// c1->Update();
// c1->SaveAs(Form("Channel%02d_Event%06lld.png",watchChannel, event));
// detector->Reset();
// detector->SetTitle( Form("Event %lld Channel %d", event, watchChannel));
// detector->Draw();

// // Draw every detector channel

// for(int ch=0; ch<32; ch++)
// {
//     ChannelPosition p = GetPosition(ch);

//     TMarker *m =
//         new TMarker(
//             p.x,
//             p.y,
//             20);

//     m->SetMarkerColor(kGray+2);
//     m->SetMarkerSize(1.5);
//     m->Draw("same");

//     TLatex *t =
//         new TLatex(
//             p.x+0.05,
//             p.y+0.05,
//             Form("%d", ch));

//     t->SetTextSize(0.03);
//     t->Draw("same");
// }

//         vector<Hit> hits;

//         // Read ADC hits
//         for(int i=0; i<adc_mult; i++)
//         {
//             Hit h;

//             h.channel = (Int_t)adc_channel[i];
//             h.adc = adc_value[i];
//             h.tdc = -999999;

//             hits.push_back(h);
//         }

//         // Match TDC values
//         for(int i=0; i<tdc_mult; i++)
//         {
//             for(size_t j=0; j<hits.size(); j++)
//             {
//                 if(hits[j].channel == (Int_t)tdc_channel[i])
//                 {
//                     hits[j].tdc = tdc_value[i];
//                     break;
//                 }
//             }
//         }

//         // Keep only valid hits
//         hits.erase(
//             remove_if(
//                 hits.begin(),
//                 hits.end(),
//                 [](const Hit &h)
//                 {
//                     return h.tdc < -1000;
//                 }),
//             hits.end());

//         // Only examine the watched channel
//         for(size_t i=0; i<hits.size(); i++)
//         {
//             if(hits[i].channel != watchChannel)
//                 continue;

//             adcSum += hits[i].adc;
//             adcSum2 += hits[i].adc * hits[i].adc;

//             tdcSum += hits[i].tdc;
//             tdcSum2 += hits[i].tdc * hits[i].tdc;

//             count++;
//         }
//     }

//     double adcMean = adcSum / count;
//     double tdcMean = tdcSum / count;

//     double adcSigma = sqrt(adcSum2 / count - adcMean * adcMean);

//     double tdcSigma = sqrt(tdcSum2 / count - tdcMean * tdcMean);

//     double adcLow  = adcMean - 2.0 * adcSigma;
//     double adcHigh = adcMean + 2.0 * adcSigma;

//     double tdcLow  = tdcMean - 2.0 * tdcSigma;
//     double tdcHigh = tdcMean + 2.0 * tdcSigma;

//     cout << "\nAutomatic PID limits\n";
//     cout << "ADC: " << adcLow << "  to  " << adcHigh << endl;
//     cout << "TDC: " << tdcLow << "  to  " << tdcHigh << endl;

// for(Long64_t event=0; event<nEntries; event++)
// {
//     T->GetEntry(event);

//     vector<Hit> hits;

//     //------------------------------------------------------
//     // Read ADC hits
//     //------------------------------------------------------

//     for(int i=0; i<adc_mult; i++)
//     {
//         Hit h;

//         h.channel = (Int_t)adc_channel[i];
//         h.adc = adc_value[i];
//         h.tdc = -999999;

//         hits.push_back(h);
//     }

//     //------------------------------------------------------
//     // Match TDC values
//     //------------------------------------------------------

//     for(int i=0; i<tdc_mult; i++)
//     {
//         for(size_t j=0; j<hits.size(); j++)
//         {
//             if(hits[j].channel == (Int_t)tdc_channel[i])
//             {
//                 hits[j].tdc = tdc_value[i];
//                 break;
//             }
//         }
//     }

//     //------------------------------------------------------
//     // Remove invalid hits
//     //------------------------------------------------------

//     hits.erase(
//         remove_if(
//             hits.begin(),
//             hits.end(),
//             [](const Hit &h)
//             {
//                 return h.tdc < -1000;
//             }
//         ),
//         hits.end()
//     );

//     //------------------------------------------------------
//     // Sort hits by time
//     //------------------------------------------------------

//     sort(
//         hits.begin(),
//         hits.end(),
//         [](const Hit &a, const Hit &b)
//         {
//             return a.tdc < b.tdc;
//         }
//     );

// //------------------------------------------------------
// // Draw detector geometry
// //------------------------------------------------------

// for(int ch=0; ch<32; ch++)
// {
//     ChannelPosition p = GetPosition(ch);

//     TMarker *m = new TMarker(
//         p.x,
//         p.y,
//         20);

//     m->SetMarkerColor(kGray+2);
//     m->SetMarkerSize(1.3);
//     m->Draw("same");

//     TLatex *lab =
//         new TLatex(
//             p.x+0.08,
//             p.y+0.08,
//             Form("%d", ch));

//     lab->SetTextSize(0.025);
//     lab->Draw("same");
// }
// //------------------------------------------------------
// // Draw event hits
// //------------------------------------------------------

// for(size_t j=0; j<hits.size(); j++)
// {
//     ChannelPosition p = GetPosition(hits[j].channel);

//     TMarker *m =
//         new TMarker(
//             p.x,
//             p.y,
//             20);

//     if(j==0)
//         m->SetMarkerColor(kBlue);

//     else if(j==hits.size()-1)
//         m->SetMarkerColor(kRed);

//     else
//         m->SetMarkerColor(kGreen+2);

//     m->SetMarkerSize(2.0);

//     m->Draw("same");
// }

// //------------------------------------------------------
// // Draw track
// //------------------------------------------------------

// for(size_t j=1; j<hits.size(); j++)
// {
//     ChannelPosition p1 = GetPosition(hits[j-1].channel);
//     ChannelPosition p2 = GetPosition(hits[j].channel);
//     TLine *line = new TLine( p1.x, p1.y, p2.x, p2.y);

//     line->SetLineWidth(2);
//     line->SetLineColor(kBlue);

//     line->Draw("same");
// }

//     //------------------------------------------------------
//     // Search selected channel only
//     //------------------------------------------------------

//     for(size_t i=0; i<hits.size(); i++)
//     {

//         if(hits[i].channel != watchChannel)
//             continue;
//     //--------------------------------------------------
//     // Highlight watched channel
//     //--------------------------------------------------

//     c1->cd();

//     TMarker *watch = new TMarker(watchPosition.x, watchPosition.y, 29);

//     watch->SetMarkerColor(kRed);
//     watch->SetMarkerSize(2.5);
//     watch->Draw("same");

//         //--------------------------------------------------
//         // Fill PID histogram
//         //--------------------------------------------------

//         pid->Fill(hits[i].tdc, hits[i].adc);
//         //--------------------------------------------------
//         // Proton PID cut
//         //--------------------------------------------------

//         bool protonCandidate = false;

//         if
//         (hits[i].tdc > tdcLow &&
//         hits[i].tdc < tdcHigh &&
//         hits[i].adc > adcLow &&
//         hits[i].adc < adcHigh)
//         {
//             protonCandidate = true;
//         }

//         //--------------------------------------------------
//         // Find previous and next channels
//         //--------------------------------------------------

//         int previousChannel = -1;
//         int nextChannel = -1;

//         double previousX=-999;
//         double previousY=-999;
//         double previousZ=-999;

//         double nextX=-999;
//         double nextY=-999;
//         double nextZ=-999;

//         if(i>0)
//         {
//     ChannelPosition prev = GetPosition(hits[i-1].channel);

//     TMarker *prevMarker = new TMarker(prev.x, prev.y, 21);

//     prevMarker->SetMarkerColor(kBlue);
//     prevMarker->SetMarkerSize(2.0);
//     prevMarker->Draw("same");

//             previousChannel = hits[i-1].channel;

//             previousX = prev.x;
//             previousY = prev.y;
//             previousZ = prev.z;
//         }

//         if(i<hits.size()-1)
//         {
//      ChannelPosition next = GetPosition(hits[i+1].channel);
//     TMarker *nextMarker = new TMarker(next.x, next.y, 22);
//     nextMarker->SetMarkerColor(kGreen+2);
//     nextMarker->SetMarkerSize(2.0);
//     nextMarker->Draw("same");

//             nextChannel = hits[i+1].channel;

//             nextX = next.x;
//             nextY = next.y;
//             nextZ = next.z;
//         }

//         //--------------------------------------------------
//         // Save CSV
//         //--------------------------------------------------

//         csv << event << ", "
//             << watchChannel << ", "
//             << watchPosition.x << ", "
//             << watchPosition.y << ", "
//             << watchPosition.z << ", "

//             << previousChannel << ", "
//             << previousX << ", "
//             << previousY << ", "
//             << previousZ << ", "

//             << nextChannel << ", "
//             << nextX << ", "
//             << nextY << ", "
//             << nextZ << ", "

//             << hits[i].adc << ", "
//             << hits[i].tdc << ", "
//             << protonCandidate

//             << endl;

//         //--------------------------------------------------
//         // Print results
//         //--------------------------------------------------

//         cout << "\n==============================" << endl;
//         cout << "Event: " << event << endl;
//         cout << "Channel: " << watchChannel << endl;
//         cout << "ADC: " << hits[i].adc << endl;
//         cout << "TDC: " << hits[i].tdc << endl;

//         if(protonCandidate)
//             cout << "Particle ID: Proton candidate" << endl;
//         else
//             cout << "Particle ID: Not proton" << endl;

//         cout << "Previous Channel: "
//              << previousChannel << endl;

//         cout << "Next Channel: "
//              << nextChannel << endl;

//         cout << "==============================" << endl;

//     }
// }

//     //----------------------------------------------------------
//     // Save PDF and close files
//     //----------------------------------------------------------

   
//     cpid->SaveAs(Form("channel_%02d_PID.pdf",watchChannel));
//     csv.close();
//     f->Close();


//     cout << "\nAnalysis complete." << endl;

//     cout << "Saved:"
//          << "\n  event_tracker_results.csv"
//          << "\n  active_analyzer_channel_map.pdf"
//          << endl;

// }



#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TLine.h>
#include <TMarker.h>
#include <TString.h>
#include <TTree.h>

namespace {

constexpr int kNChannels = 32;
constexpr int kMaxHits = 128;
constexpr double kMissingTdc = -999999.0;

struct Hit {
    int channel = -1;
    double adc = 0.0;
    double tdc = kMissingTdc;
};

struct ChannelPosition {
    int channel;
    double x;
    double y;
    double z;
};

const array<ChannelPosition, kNChannels> geometry = {{
    { 0, 0, 0, 0}, { 1, 1, 0, 0}, { 2, 2, 0, 0}, { 3, 3, 0, 0},
    { 4, 4, 0, 0}, { 5, 5, 0, 0}, { 6, 6, 0, 0}, { 7, 7, 0, 0},
    { 8, 0, 1, 0}, { 9, 1, 1, 0}, {10, 2, 1, 0}, {11, 3, 1, 0},
    {12, 4, 1, 0}, {13, 5, 1, 0}, {14, 6, 1, 0}, {15, 7, 1, 0},
    {16, 0, 2, 0}, {17, 1, 2, 0}, {18, 2, 2, 0}, {19, 3, 2, 0},
    {20, 4, 2, 0}, {21, 5, 2, 0}, {22, 6, 2, 0}, {23, 7, 2, 0},
    {24, 0, 3, 0}, {25, 1, 3, 0}, {26, 2, 3, 0}, {27, 3, 3, 0},
    {28, 4, 3, 0}, {29, 5, 3, 0}, {30, 6, 3, 0}, {31, 7, 3, 0}
}};

const ChannelPosition &GetPosition(int channel)
{
    return geometry.at(static_cast<size_t>(channel));
}

bool IsValidChannel(int channel)
{
    return channel >= 0 && channel < kNChannels;
}

vector<Hit> BuildHits(
    int adcMult,
    const double *adcValues,
    const double *adcChannels,
    int tdcMult,
    const double *tdcValues,
    const double *tdcChannels)
{
    vector<Hit> hits;
    hits.reserve(static_cast<size_t>(max(0, adcMult)));

    // There can be more than one ADC hit for a channel. Store indices so TDC
    // hits can be paired with the first still-unmatched ADC hit on that channel.
    unordered_map<int, vector<size_t>> adcIndices;

    for (int i = 0; i < adcMult; ++i) {
        const int channel = static_cast<int>(lround(adcChannels[i]));
        if (!IsValidChannel(channel)) {
            continue;
        }

        adcIndices[channel].push_back(hits.size());
        hits.push_back({channel, adcValues[i], kMissingTdc});
    }

    unordered_map<int, size_t> nextAdcIndex;
    for (int i = 0; i < tdcMult; ++i) {
        const int channel = static_cast<int>(lround(tdcChannels[i]));
        const auto found = adcIndices.find(channel);
        if (found == adcIndices.end()) {
            continue;
        }

        size_t &offset = nextAdcIndex[channel];
        if (offset < found->second.size()) {
            hits[found->second[offset]].tdc = tdcValues[i];
            ++offset;
        }
    }

    hits.erase(
        remove_if(
            hits.begin(),
            hits.end(),
            [](const Hit &hit) { return hit.tdc == kMissingTdc; }),
        hits.end());

    sort(
        hits.begin(),
        hits.end(),
        [](const Hit &a, const Hit &b) { return a.tdc < b.tdc; });

    return hits;
}

void DrawGeometry()
{
    for (int channel = 0; channel < kNChannels; ++channel) {
        const ChannelPosition &p = GetPosition(channel);

        auto *marker = new TMarker(p.x, p.y, 20);
        marker->SetMarkerColor(kGray + 2);
        marker->SetMarkerSize(1.3);
        marker->Draw();

        auto *label = new TLatex(p.x + 0.08, p.y + 0.08, Form("%d", channel));
        label->SetTextSize(0.025);
        label->Draw();
    }
}

} // namespace

void eventracker(
    int watchChannel,
    const char *inputFile =
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root")
{
    if (!IsValidChannel(watchChannel)) {
        cerr << "Error: watchChannel must be between 0 and "
                  << (kNChannels - 1) << ".\n";
        return;
    }

    TFile *file = TFile::Open(inputFile, "READ");
    if (!file || file->IsZombie()) {
        cerr << "Error: could not open ROOT file: " << inputFile << '\n';
        if (file) {
            file->Close();
            delete file;
        }
        return;
    }

    auto *tree = dynamic_cast<TTree *>(file->Get("T"));
    if (!tree) {
        cerr << "Error: TTree \"T\" was not found.\n";
        file->Close();
        delete file;
        return;
    }

    int adcMult = 0;
    int tdcMult = 0;
    double adcValue[kMaxHits] = {};
    double adcChannel[kMaxHits] = {};
    double tdcValue[kMaxHits] = {};
    double tdcChannel[kMaxHits] = {};

    const array<const char *, 6> branchNames = {{
        "Ndata.sbs.activeAna_adc.a_mult",
        "sbs.activeAna_adc.a",
        "sbs.activeAna_adc.adcelemID",
        "Ndata.sbs.activeAna_tdc.tdc_mult",
        "sbs.activeAna_tdc.hits.t",
        "sbs.activeAna_tdc.tdcelemID"
    }};

    for (const char *name : branchNames) {
        if (!tree->GetBranch(name)) {
            cerr << "Error: required branch not found: " << name << '\n';
            file->Close();
            delete file;
            return;
        }
    }

    tree->SetBranchAddress(branchNames[0], &adcMult);
    tree->SetBranchAddress(branchNames[1], adcValue);
    tree->SetBranchAddress(branchNames[2], adcChannel);
    tree->SetBranchAddress(branchNames[3], &tdcMult);
    tree->SetBranchAddress(branchNames[4], tdcValue);
    tree->SetBranchAddress(branchNames[5], tdcChannel);

    const ChannelPosition &watchPosition = GetPosition(watchChannel);
    const Long64_t nEntries = tree->GetEntries();

    cout << "Tracking channel " << watchChannel << " at position ("
              << watchPosition.x << ", " << watchPosition.y << ", "
              << watchPosition.z << ")\n"
              << "Total events = " << nEntries << '\n';

    double adcSum = 0.0;
    double adcSum2 = 0.0;
    double tdcSum = 0.0;
    double tdcSum2 = 0.0;
    Long64_t count = 0;

    // First pass: calculate the watched channel's ADC/TDC statistics.
    for (Long64_t event = 0; event < nEntries; ++event) {
        tree->GetEntry(event);

        if (adcMult < 0 || tdcMult < 0 ||
            adcMult > kMaxHits || tdcMult > kMaxHits) {
            cerr << "Warning: event " << event
                      << " has an invalid multiplicity; skipping it.\n";
            continue;
        }

        const vector<Hit> hits = BuildHits(
            adcMult, adcValue, adcChannel,
            tdcMult, tdcValue, tdcChannel);

        for (const Hit &hit : hits) {
            if (hit.channel != watchChannel) {
                continue;
            }
            adcSum += hit.adc;
            adcSum2 += hit.adc * hit.adc;
            tdcSum += hit.tdc;
            tdcSum2 += hit.tdc * hit.tdc;
            ++count;
        }
    }

    if (count == 0) {
        cerr << "Error: no matched ADC/TDC hits were found for channel "
                  << watchChannel << ".\n";
        file->Close();
        delete file;
        return;
    }

    const double adcMean = adcSum / static_cast<double>(count);
    const double tdcMean = tdcSum / static_cast<double>(count);
    const double adcVariance =
        max(0.0, adcSum2 / static_cast<double>(count) - adcMean * adcMean);
    const double tdcVariance =
        max(0.0, tdcSum2 / static_cast<double>(count) - tdcMean * tdcMean);
    const double adcSigma = sqrt(adcVariance);
    const double tdcSigma = sqrt(tdcVariance);
    const double adcLow = adcMean - 2.0 * adcSigma;
    const double adcHigh = adcMean + 2.0 * adcSigma;
    const double tdcLow = tdcMean - 2.0 * tdcSigma;
    const double tdcHigh = tdcMean + 2.0 * tdcSigma;

    cout << "\nAutomatic PID limits\n"
              << "ADC: " << adcLow << " to " << adcHigh << '\n'
              << "TDC: " << tdcLow << " to " << tdcHigh << '\n';

    const TString csvName = Form("channel_%02d_results.csv", watchChannel);
    ofstream csv(csvName.Data());
    if (!csv) {
        cerr << "Error: could not create " << csvName << ".\n";
        file->Close();
        delete file;
        return;
    }

    csv << "Event,CurrentChannel,CurrentX,CurrentY,CurrentZ,"
           "PreviousChannel,PreviousX,PreviousY,PreviousZ,"
           "NextChannel,NextX,NextY,NextZ,ADC,TDC,PID\n";

    auto *eventCanvas =
        new TCanvas("eventCanvas", "Detector Event Display", 800, 600);
    auto *detector =
        new TH2F("detector", "Detector;X;Y", 8, -0.5, 7.5, 4, -0.5, 3.5);
    detector->SetStats(false);

    auto *pidCanvas =
        new TCanvas("pidCanvas", "ADC vs TDC PID", 800, 700);
    auto *pid =
        new TH2D("pid", "PID;TDC;ADC", 100, 0, 2000, 100, -100, 500);
    pid->SetStats(false);

    // Second pass: fill output, draw tracks, and save watched-channel events.
    for (Long64_t event = 0; event < nEntries; ++event) {
        tree->GetEntry(event);

        if (adcMult < 0 || tdcMult < 0 ||
            adcMult > kMaxHits || tdcMult > kMaxHits) {
            continue;
        }

        const vector<Hit> hits = BuildHits(
            adcMult, adcValue, adcChannel,
            tdcMult, tdcValue, tdcChannel);

        const auto watched = find_if(
            hits.begin(),
            hits.end(),
            [watchChannel](const Hit &hit) {
                return hit.channel == watchChannel;
            });

        if (watched == hits.end()) {
            continue;
        }

        eventCanvas->cd();
        detector->Reset();
        detector->SetTitle(Form("Event %lld, channel %d", event, watchChannel));
        detector->Draw();
        DrawGeometry();

        for (size_t i = 0; i < hits.size(); ++i) {
            const ChannelPosition &p = GetPosition(hits[i].channel);
            auto *marker = new TMarker(p.x, p.y, 20);
            marker->SetMarkerColor(
                i == 0 ? kBlue :
                (i + 1 == hits.size() ? kRed : kGreen + 2));
            marker->SetMarkerSize(2.0);
            marker->Draw();
        }

        for (size_t i = 1; i < hits.size(); ++i) {
            const ChannelPosition &p1 = GetPosition(hits[i - 1].channel);
            const ChannelPosition &p2 = GetPosition(hits[i].channel);
            auto *line = new TLine(p1.x, p1.y, p2.x, p2.y);
            line->SetLineWidth(2);
            line->SetLineColor(kBlue);
            line->Draw();
        }

        const size_t i =
            static_cast<size_t>(distance(hits.begin(), watched));
        const Hit &hit = hits[i];

        auto *watchMarker =
            new TMarker(watchPosition.x, watchPosition.y, 29);
        watchMarker->SetMarkerColor(kMagenta + 2);
        watchMarker->SetMarkerSize(2.5);
        watchMarker->Draw();

        int previousChannel = -1;
        int nextChannel = -1;
        double previousX = numeric_limits<double>::quiet_NaN();
        double previousY = numeric_limits<double>::quiet_NaN();
        double previousZ = numeric_limits<double>::quiet_NaN();
        double nextX = numeric_limits<double>::quiet_NaN();
        double nextY = numeric_limits<double>::quiet_NaN();
        double nextZ = numeric_limits<double>::quiet_NaN();

        if (i > 0) {
            previousChannel = hits[i - 1].channel;
            const ChannelPosition &previous = GetPosition(previousChannel);
            previousX = previous.x;
            previousY = previous.y;
            previousZ = previous.z;

            auto *marker = new TMarker(previous.x, previous.y, 21);
            marker->SetMarkerColor(kBlue);
            marker->SetMarkerSize(2.0);
            marker->Draw();
        }

        if (i + 1 < hits.size()) {
            nextChannel = hits[i + 1].channel;
            const ChannelPosition &next = GetPosition(nextChannel);
            nextX = next.x;
            nextY = next.y;
            nextZ = next.z;

            auto *marker = new TMarker(next.x, next.y, 22);
            marker->SetMarkerColor(kGreen + 2);
            marker->SetMarkerSize(2.0);
            marker->Draw();
        }

        const bool protonCandidate =
            hit.tdc > tdcLow && hit.tdc < tdcHigh &&
            hit.adc > adcLow && hit.adc < adcHigh;

        pid->Fill(hit.tdc, hit.adc);

        csv << event << ',' << watchChannel << ','
            << watchPosition.x << ',' << watchPosition.y << ','
            << watchPosition.z << ',' << previousChannel << ','
            << previousX << ',' << previousY << ',' << previousZ << ','
            << nextChannel << ',' << nextX << ',' << nextY << ',' << nextZ
            << ',' << hit.adc << ',' << hit.tdc << ','
            << (protonCandidate ? "proton_candidate" : "not_proton") << '\n';

        eventCanvas->Modified();
        eventCanvas->Update();
        eventCanvas->SaveAs(
            Form("channel_%02d_event_%06lld.png", watchChannel, event));

        cout << "Event " << event
                  << ": ADC=" << hit.adc
                  << ", TDC=" << hit.tdc
                  << ", previous=" << previousChannel
                  << ", next=" << nextChannel
                  << ", PID="
                  << (protonCandidate ? "proton candidate" : "not proton")
                  << '\n';
    }

    pidCanvas->cd();
    pid->Draw("COLZ");
    pidCanvas->Modified();
    pidCanvas->Update();
    const TString pidName = Form("channel_%02d_PID.pdf", watchChannel);
    pidCanvas->SaveAs(pidName);

    csv.close();
    file->Close();
    delete file;

    cout << "\nAnalysis complete.\n"
              << "Saved CSV: " << csvName << '\n'
              << "Saved PID plot: " << pidName << '\n'
              << "Saved one event PNG per watched-channel event.\n";
}