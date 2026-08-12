#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <TBranch.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TLeaf.h>
#include <TLatex.h>
#include <TLine.h>
#include <TMarker.h>
#include <TString.h>
#include <TTree.h>

namespace {

constexpr int kNChannels = 32;
constexpr double kMissingTdc = -999999.0;
constexpr const char *kCodeVersion = "2026-07-28-v5";

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

const std::array<ChannelPosition, kNChannels> geometry = {{
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
    return geometry.at(static_cast<std::size_t>(channel));
}

bool IsValidChannel(int channel)
{
    return channel >= 0 && channel < kNChannels;
}

std::string EntryText(Long64_t value)
{
    return std::to_string(static_cast<long long>(value));
}

std::vector<Hit> BuildHits(
    TLeaf *adcValues,
    TLeaf *adcChannels,
    TLeaf *tdcValues,
    TLeaf *tdcChannels)
{
    const int adcMult = std::min(adcValues->GetNdata(), adcChannels->GetNdata());
    const int tdcMult = std::min(tdcValues->GetNdata(), tdcChannels->GetNdata());
    std::vector<Hit> hits;
    hits.reserve(static_cast<std::size_t>(adcMult));

    // There can be more than one ADC hit for a channel. Store indices so TDC
    // hits can be paired with the first still-unmatched ADC hit on that channel.
    std::unordered_map<int, std::vector<std::size_t>> adcIndices;

    for (int i = 0; i < adcMult; ++i) {
        const int channel =
            static_cast<int>(std::lround(adcChannels->GetValue(i)));
        if (!IsValidChannel(channel)) {
            continue;
        }

        adcIndices[channel].push_back(hits.size());
        hits.push_back({channel, adcValues->GetValue(i), kMissingTdc});
    }

    std::unordered_map<int, std::size_t> nextAdcIndex;
    for (int i = 0; i < tdcMult; ++i) {
        const int channel =
            static_cast<int>(std::lround(tdcChannels->GetValue(i)));
        const auto found = adcIndices.find(channel);
        if (found == adcIndices.end()) {
            continue;
        }

        std::size_t &offset = nextAdcIndex[channel];
        if (offset < found->second.size()) {
            hits[found->second[offset]].tdc = tdcValues->GetValue(i);
            ++offset;
        }
    }

    hits.erase(
        std::remove_if(
            hits.begin(),
            hits.end(),
            [](const Hit &hit) { return hit.tdc == kMissingTdc; }),
        hits.end());

    std::sort(
        hits.begin(),
        hits.end(),
        [](const Hit &a, const Hit &b) { return a.tdc < b.tdc; });

    return hits;
}

TLeaf *GetDataLeaf(TTree *tree, const char *branchName)
{
    if (TLeaf *leaf = tree->GetLeaf(branchName)) {
        return leaf;
    }

    TBranch *branch = tree->GetBranch(branchName);
    if (!branch || !branch->GetListOfLeaves() ||
        branch->GetListOfLeaves()->GetEntries() == 0) {
        return nullptr;
    }

    return static_cast<TLeaf *>(branch->GetListOfLeaves()->At(0));
}

void DrawGeometry()
{
    for (int channel = 0; channel < kNChannels; ++channel) {
        const ChannelPosition &p = GetPosition(channel);

        auto *marker = new TMarker(p.x, p.y, 20);
        marker->SetMarkerColor(kGray + 2);
        marker->SetMarkerSize(1.3);
        marker->Draw();
        marker->SetBit(kCanDelete);

        auto *label = new TLatex(p.x + 0.08, p.y + 0.08, Form("%d", channel));
        label->SetTextSize(0.025);
        label->Draw();
        label->SetBit(kCanDelete);
    }
}

} // namespace

void eventracker(
    int watchChannel,
    const char *inputFile =
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root")
{
    std::cout << "eventracker code version: " << kCodeVersion << '\n';

    if (!IsValidChannel(watchChannel)) {
        std::cerr << "Error: watchChannel must be between 0 and "
                  << (kNChannels - 1) << ".\n";
        return;
    }

    TFile *file = TFile::Open(inputFile, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: could not open ROOT file: " << inputFile << '\n';
        if (file) {
            file->Close();
            delete file;
        }
        return;
    }

    auto *tree = dynamic_cast<TTree *>(file->Get("T"));
    if (!tree) {
        std::cerr << "Error: TTree \"T\" was not found.\n";
        file->Close();
        delete file;
        return;
    }

    const std::array<const char *, 4> branchNames = {{
        "sbs.activeAna_adc.a",
        "sbs.activeAna_adc.adcelemID",
        "sbs.activeAna_tdc.hits.t",
        "sbs.activeAna_tdc.tdcelemID"
    }};

    std::array<TLeaf *, 4> leaves = {{nullptr, nullptr, nullptr, nullptr}};
    std::size_t leafIndex = 0;
    for (const char *name : branchNames) {
        leaves[leafIndex] = GetDataLeaf(tree, name);
        if (!leaves[leafIndex]) {
            std::cerr << "Error: required data leaf not found for branch: "
                      << name << '\n';
            file->Close();
            delete file;
            return;
        }
        ++leafIndex;
    }

    const ChannelPosition &watchPosition = GetPosition(watchChannel);
    const Long64_t nEntries = tree->GetEntries();

    std::cout << "Tracking channel " << watchChannel << " at position ("
              << watchPosition.x << ", " << watchPosition.y << ", "
              << watchPosition.z << ")\n"
              << "Total events = " << EntryText(nEntries) << '\n';

    double adcSum = 0.0;
    double adcSum2 = 0.0;
    double tdcSum = 0.0;
    double tdcSum2 = 0.0;
    Long64_t count = 0;

    // First pass: calculate the watched channel's ADC/TDC statistics.
    for (Long64_t event = 0; event < nEntries; ++event) {
        if (tree->GetEntry(event) <= 0) {
            std::cerr << "Warning: could not read event "
                      << EntryText(event) << ".\n";
            continue;
        }

        const std::vector<Hit> hits = BuildHits(
            leaves[0], leaves[1], leaves[2], leaves[3]);

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
        std::cerr << "Error: no matched ADC/TDC hits were found for channel "
                  << watchChannel << ".\n";
        file->Close();
        delete file;
        return;
    }

    const double adcMean = adcSum / static_cast<double>(count);
    const double tdcMean = tdcSum / static_cast<double>(count);
    const double adcVariance =
        std::max(0.0, adcSum2 / static_cast<double>(count) - adcMean * adcMean);
    const double tdcVariance =
        std::max(0.0, tdcSum2 / static_cast<double>(count) - tdcMean * tdcMean);
    const double adcSigma = std::sqrt(adcVariance);
    const double tdcSigma = std::sqrt(tdcVariance);
    const double adcLow = adcMean - 2.0 * adcSigma;
    const double adcHigh = adcMean + 2.0 * adcSigma;
    const double tdcLow = tdcMean - 2.0 * tdcSigma;
    const double tdcHigh = tdcMean + 2.0 * tdcSigma;

    std::cout << "\nAutomatic PID limits\n"
              << "ADC: " << adcLow << " to " << adcHigh << '\n'
              << "TDC: " << tdcLow << " to " << tdcHigh << '\n';

    const TString csvName = Form("channel_%02d_results.csv", watchChannel);
    std::ofstream csv(csvName.Data());
    if (!csv) {
        std::cerr << "Error: could not create " << csvName << ".\n";
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

    const TString combinedPdf =
        Form("channel_%02d_events_and_PID.pdf", watchChannel);
    eventCanvas->Print(combinedPdf + "[");

    // Second pass: fill output, draw tracks, and save watched-channel events.
    for (Long64_t event = 0; event < nEntries; ++event) {
        if (tree->GetEntry(event) <= 0) {
            continue;
        }

        const std::vector<Hit> hits = BuildHits(
            leaves[0], leaves[1], leaves[2], leaves[3]);

        const auto watched = std::find_if(
            hits.begin(),
            hits.end(),
            [watchChannel](const Hit &hit) {
                return hit.channel == watchChannel;
            });

        if (watched == hits.end()) {
            continue;
        }

        eventCanvas->cd();
        eventCanvas->Clear();
        detector->Reset();
        detector->SetTitle(
            Form("Event %s, channel %d",
                 EntryText(event).c_str(), watchChannel));
        detector->Draw();
        DrawGeometry();

        for (std::size_t i = 0; i < hits.size(); ++i) {
            const ChannelPosition &p = GetPosition(hits[i].channel);
            auto *marker = new TMarker(p.x, p.y, 20);
            marker->SetMarkerColor(
                i == 0 ? kBlue :
                (i + 1 == hits.size() ? kRed : kGreen + 2));
            marker->SetMarkerSize(2.0);
            marker->Draw();
            marker->SetBit(kCanDelete);
        }

        for (std::size_t i = 1; i < hits.size(); ++i) {
            const ChannelPosition &p1 = GetPosition(hits[i - 1].channel);
            const ChannelPosition &p2 = GetPosition(hits[i].channel);
            auto *line = new TLine(p1.x, p1.y, p2.x, p2.y);
            line->SetLineWidth(2);
            line->SetLineColor(kBlue);
            line->Draw();
            line->SetBit(kCanDelete);
        }

        const std::size_t i =
            static_cast<std::size_t>(std::distance(hits.begin(), watched));
        const Hit &hit = hits[i];

        auto *watchMarker =
            new TMarker(watchPosition.x, watchPosition.y, 29);
        watchMarker->SetMarkerColor(kMagenta + 2);
        watchMarker->SetMarkerSize(2.5);
        watchMarker->Draw();
        watchMarker->SetBit(kCanDelete);

        int previousChannel = -1;
        int nextChannel = -1;
        double previousX = std::numeric_limits<double>::quiet_NaN();
        double previousY = std::numeric_limits<double>::quiet_NaN();
        double previousZ = std::numeric_limits<double>::quiet_NaN();
        double nextX = std::numeric_limits<double>::quiet_NaN();
        double nextY = std::numeric_limits<double>::quiet_NaN();
        double nextZ = std::numeric_limits<double>::quiet_NaN();

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
            marker->SetBit(kCanDelete);
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
            marker->SetBit(kCanDelete);
        }

        const bool protonCandidate =
            hit.tdc > tdcLow && hit.tdc < tdcHigh &&
            hit.adc > adcLow && hit.adc < adcHigh;

        pid->Fill(hit.tdc, hit.adc);

        csv << EntryText(event) << ',' << watchChannel << ','
            << watchPosition.x << ',' << watchPosition.y << ','
            << watchPosition.z << ',' << previousChannel << ','
            << previousX << ',' << previousY << ',' << previousZ << ','
            << nextChannel << ',' << nextX << ',' << nextY << ',' << nextZ
            << ',' << hit.adc << ',' << hit.tdc << ','
            << (protonCandidate ? "proton_candidate" : "not_proton") << '\n';

        eventCanvas->Modified();
        eventCanvas->Update();
        eventCanvas->Print(combinedPdf);

        std::cout << "Event " << EntryText(event)
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
    pidCanvas->Print(combinedPdf);
    pidCanvas->Print(combinedPdf + "]");

    csv.close();
    file->Close();
    delete file;

    std::cout << "\nAnalysis complete.\n"
              << "Saved CSV: " << csvName << '\n'
              << "Saved PID plot: " << pidName << '\n'
              << "Saved combined event and PID document: "
              << combinedPdf << '\n';
}

// ROOT's `root -q "eventracker_corrected.C++g(27)"` syntax automatically
// calls a function whose name matches the macro filename. Keep this wrapper
// so both invocation styles work:
//   eventracker(27);
//   root -l -b -q "eventracker_corrected.C++g(27)"
void eventracker_corrected(
    int watchChannel,
    const char *inputFile =
        "/home/amarit/rootdisplay/genrp_replayed_1071_20k_events.root")
{
    eventracker(watchChannel, inputFile);
}