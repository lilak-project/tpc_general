#include "TTPulseAnalysisTask.h"
#include "MMChannel.h"
#include "LKHit.h"

ClassImp(TTPulseAnalysisTask);

TTPulseAnalysisTask::TTPulseAnalysisTask()
{
    fName = "TTPulseAnalysisTask";
}

bool TTPulseAnalysisTask::Init()
{
    lk_info << "Initializing TTPulseAnalysisTask" << std::endl;

    fDetector = (TexAT2 *) fRun -> GetDetector();
    fDetector -> InitChannelAnalyzer();
    fChannelArray = fRun -> GetBranchA("RawData");
    fHitArray = fRun -> RegisterBranchA("Hit", "LKHit", 200);
    fEventHeader = (TTEventHeader*) fRun -> KeepBranch("EventHeader");

    return true;
}

void TTPulseAnalysisTask::Exec(Option_t *option)
{
    fHitArray -> Clear("C");

    int countHits = 0;
    double buffer[350];

    int numChannel = fChannelArray -> GetEntriesFast();
    for (int iChannel = 0; iChannel < numChannel; ++iChannel)
    {
        auto channel = (MMChannel *) fChannelArray -> At(iChannel);
        auto cobo = channel -> GetCobo();
        auto asad = channel -> GetAsad();
        auto aget = channel -> GetAget();
        auto chan = channel -> GetChan();
        auto dchan = channel -> GetDChan();
        auto data = channel -> GetWaveformY();
        auto electronicsID = fDetector -> GetElectronicsID(cobo, asad, aget, chan);
        auto x = fDetector -> Getmmpx(asad, aget, dchan);
        auto z = fDetector -> Getmmpy(asad, aget, dchan);

        if (cobo!=0)
            continue;

        for (auto tb=0; tb<350; ++tb)
            buffer[tb] = double(data[tb]);
        auto ana = fDetector -> GetChannelAnalyzer(electronicsID);
        ana -> Analyze(buffer);

        auto numRecoHits = ana -> GetNumHits();
        for (auto iHit=0; iHit<numRecoHits; ++iHit)
        {
            auto tb        = ana -> GetTbHit(iHit);
            auto amplitude = ana -> GetAmplitude(iHit);
            auto chi2NDF   = ana -> GetChi2NDF(iHit);
            auto ndf       = ana -> GetNDF(iHit);

            auto hit = (LKHit*) fHitArray -> ConstructedAt(countHits);
            hit -> SetHitID(countHits);
            hit -> SetPosition(x,tb,z);
            hit -> SetCharge(amplitude);
            hit -> SetChi2NDF(chi2NDF);
            //hit -> SetPositionError();

            countHits++;
        }
    }

    lk_info << "Found " << countHits << " hits" << endl;
}

bool TTPulseAnalysisTask::EndOfRun()
{
    return true;
}

