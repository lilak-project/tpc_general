#include "LKPulseShapeAnalysisTask.h"
#include "TTEventHeader.h"
#include "GETChannel.h"
#include "LKHit.h"

ClassImp(LKPulseShapeAnalysisTask);

LKPulseShapeAnalysisTask::LKPulseShapeAnalysisTask()
{
    fName = "LKPulseShapeAnalysisTask";
}

bool LKPulseShapeAnalysisTask::Init()
{
    lk_info << "Initializing LKPulseShapeAnalysisTask" << std::endl;

    fDetector = fRun -> GetDetector();
    fDetectorPlane = fRun -> GetDetectorPlane();
    fChannelAnalyzer = fDetector -> GetChannelAnalyzer();

    fChannelArray = fRun -> GetBranchA("RawData");
    fHitArrayCenter = fRun -> RegisterBranchA("HitCenter","LKHit",100);
    fEventHeaderHolder = fRun -> KeepBranchA("EventHeader");

    return true;
}

void LKPulseShapeAnalysisTask::Exec(Option_t *option)
{
    fHitArrayCenter -> Clear("C");

    //auto eventHeader = (TTEventHeader*) fEventHeaderHolder -> At(0);
    //if (eventHeader->IsGoodEvent()==false)
    //    return;

    int countHits = 0;

    double buffer[512];

    double xPos;
    double yPos;
    double zPos;
    double xErr;
    double yErr;
    double zErr;

    int numChannel = fChannelArray -> GetEntriesFast();
    for (int iChannel = 0; iChannel < numChannel; ++iChannel)
    {
        auto channel = (GETChannel *) fChannelArray -> At(iChannel);
        auto chDetType = channel -> GetDetType();
        auto cobo = channel -> GetCobo();
        auto asad = channel -> GetAsad();
        auto aget = channel -> GetAget();
        auto chan = channel -> GetChan();
        auto padID = channel -> GetChan2();
        auto data = channel -> GetWaveformY();
        if (padID<0)
            padID = fDetectorPlane -> FindPadID(cobo,asad,aget,chan);

        for (auto tb=0; tb<512; ++tb)
            buffer[tb] = double(data[tb]);
        fChannelAnalyzer -> Analyze(buffer);

        auto pad = fDetectorPlane -> GetPad(padID);

        auto numRecoHits = fChannelAnalyzer -> GetNumHits();
        lk_debug << iChannel << " " << padID << " " << numRecoHits << endl;
        for (auto iHit=0; iHit<numRecoHits; ++iHit)
        {
            auto tb        = fChannelAnalyzer -> GetTbHit(iHit);
            auto amplitude = fChannelAnalyzer -> GetAmplitude(iHit);
            auto chi2NDF   = fChannelAnalyzer -> GetChi2NDF(iHit);
            auto ndf       = fChannelAnalyzer -> GetNDF(iHit);
            auto pedestal  = fChannelAnalyzer -> GetPedestal();
            LKHit* hit = nullptr;
            hit = (LKHit*) fHitArrayCenter -> ConstructedAt(countHits++);

            hit -> SetHitID(countHits);
            hit -> SetChannelID(padID);
            hit -> SetPosition(xPos,tb,zPos);
            hit -> SetPositionError(xErr,1,zErr);
            hit -> SetCharge(amplitude);
            hit -> SetPedestal(pedestal);

            countHits++;
        }
    }

    lk_info << "Found " << countHits << " hits" << endl;
}

bool LKPulseShapeAnalysisTask::EndOfRun()
{
    return true;
}
