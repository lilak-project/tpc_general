#include "TTPulseAnalysisTask.h"
#include "TTEventHeader.h"
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

    fHitArrayCenter = fRun -> RegisterBranchA("HitCenter","LKHit",100);
    fHitArrayLStrip = fRun -> RegisterBranchA("HitLStrip","LKHit",100);
    fHitArrayLChain = fRun -> RegisterBranchA("HitLChain","LKHit",100);
    fHitArrayRStrip = fRun -> RegisterBranchA("HitRStrip","LKHit",100);
    fHitArrayRChain = fRun -> RegisterBranchA("HitRChain","LKHit",100);
    fHitArrayOthers = fRun -> RegisterBranchA("HitOthers","LKHit",100);

    fITypeLStrip = fDetector -> GetTypeNumber(TexAT2::eType::kLeftStrip);
    fITypeRStrip = fDetector -> GetTypeNumber(TexAT2::eType::kRightStrip);
    fITypeLChain = fDetector -> GetTypeNumber(TexAT2::eType::kLeftChain);
    fITypeRChain = fDetector -> GetTypeNumber(TexAT2::eType::kRightChain);
    fITypeLCenter = fDetector -> GetTypeNumber(TexAT2::eType::kLowCenter);
    fITypeHCenter = fDetector -> GetTypeNumber(TexAT2::eType::kHighCenter);

    fEventHeaderHolder = fRun -> KeepBranchA("EventHeader");

    return true;
}

void TTPulseAnalysisTask::Exec(Option_t *option)
{
    fHitArrayCenter -> Clear("C");
    fHitArrayLStrip -> Clear("C");
    fHitArrayLChain -> Clear("C");
    fHitArrayRStrip -> Clear("C");
    fHitArrayRChain -> Clear("C");
    fHitArrayOthers -> Clear("C");

    auto eventHeader = (TTEventHeader*) fEventHeaderHolder -> At(0);
    if (eventHeader->IsGoodEvent()==false)
        return;

    int countHits = 0;
    int countHitCenter = 0;
    int countHitLStrip = 0;
    int countHitLChain = 0;
    int countHitRStrip = 0;
    int countHitRChain = 0;
    int countHitOthers = 0;

    double buffer[350];

    double xPos;
    double yPos;
    double zPos;
    double xErr;
    double yErr;
    double zErr;

    int numChannel = fChannelArray -> GetEntriesFast();
    for (int iChannel = 0; iChannel < numChannel; ++iChannel)
    {
        auto channel = (MMChannel *) fChannelArray -> At(iChannel);
        auto chDetType = channel -> GetDetType();
        auto cobo = channel -> GetCobo();
        auto asad = channel -> GetAsad();
        auto aget = channel -> GetAget();
        auto chan = channel -> GetChan();
        auto dchan = channel -> GetDChan();
        auto data = channel -> GetWaveformY();
        auto electronicsID = fDetector -> GetElectronicsID(cobo, asad, aget, chan);
        auto x = fDetector -> Getmmpx(asad, aget, dchan);
        auto z = fDetector -> Getmmpy(asad, aget, dchan);
        auto caac = cobo*10000 + asad*1000 + aget*100 + chan;

        if (cobo!=0)
            continue;

        for (auto tb=0; tb<350; ++tb)
            buffer[tb] = double(data[tb]);
        auto ana = fDetector -> GetChannelAnalyzer(electronicsID);
        ana -> Analyze(buffer);

        if(cobo==0) {
            if(chan<11) chan = chan;
            else if(chan<21) chan = chan +1;
            else if(chan<43) chan = chan +2;
            else if(chan<53) chan = chan +3;
            else if(chan<64) chan = chan +4;
        }

        fDetector -> CAACToGlobalPosition(cobo,asad,aget,chan, xPos,yPos,zPos,xErr,yErr,zErr);

        double alpha = 0;
        auto detType = fDetector -> GetType(cobo,asad,aget,chan);
             if (detType==TexAT2::eType::kLeftStrip)  alpha = -100;
        else if (detType==TexAT2::eType::kRightStrip) alpha = +100;
        else if (detType==TexAT2::eType::kLeftChain)  alpha = -200;
        else if (detType==TexAT2::eType::kRightChain) alpha = +200;

        auto numRecoHits = ana -> GetNumHits();
        for (auto iHit=0; iHit<numRecoHits; ++iHit)
        {
            auto tb        = ana -> GetTbHit(iHit);
            auto amplitude = ana -> GetAmplitude(iHit);
            auto chi2NDF   = ana -> GetChi2NDF(iHit);
            auto ndf       = ana -> GetNDF(iHit);
            auto pedestal  = ana -> GetPedestal();
            LKHit* hit = nullptr;
                 if (chDetType==fITypeLCenter) hit = (LKHit*) fHitArrayCenter -> ConstructedAt(countHitCenter++);
            else if (chDetType==fITypeHCenter) hit = (LKHit*) fHitArrayCenter -> ConstructedAt(countHitCenter++);
            else if (chDetType==fITypeLStrip ) hit = (LKHit*) fHitArrayLStrip -> ConstructedAt(countHitLStrip++);
            else if (chDetType==fITypeLChain ) hit = (LKHit*) fHitArrayLChain -> ConstructedAt(countHitLChain++);
            else if (chDetType==fITypeRStrip ) hit = (LKHit*) fHitArrayRStrip -> ConstructedAt(countHitRStrip++);
            else if (chDetType==fITypeRChain ) hit = (LKHit*) fHitArrayRChain -> ConstructedAt(countHitRChain++);
            else                               hit = (LKHit*) fHitArrayOthers -> ConstructedAt(countHitOthers++);

            hit -> SetHitID(countHits);
            hit -> SetChannelID(caac);
            hit -> SetPosition(xPos,tb,zPos);
            hit -> SetPositionError(xErr,1,zErr);
            hit -> SetCharge(amplitude);
            hit -> SetPedestal(pedestal);
            hit -> SetAlpha(alpha);

            countHits++;
        }
    }

    lk_info << "Found " << countHits << " hits" << endl;
}

bool TTPulseAnalysisTask::EndOfRun()
{
    return true;
}
