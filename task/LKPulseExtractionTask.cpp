#include "LKPulseExtractionTask.h"

ClassImp(LKPulseExtractionTask);

LKPulseExtractionTask::LKPulseExtractionTask()
{
    fName = "LKPulseExtractionTask";
}

bool LKPulseExtractionTask::Init()
{
    fChannelArray = fRun -> GetBranchA("RawData");

    if (fPar -> CheckPar("LKPulseExtraction/analysisName"))      fAnalysisName      = fPar -> GetParInt("LKPulseExtraction/analysisName");
    if (fPar -> CheckPar("LKPulseExtraction/threshold"))         fThreshold         = fPar -> GetParInt("LKPulseExtraction/threshold");
    if (fPar -> CheckPar("LKPulseExtraction/fixPedestal"))       fFixPedestal       = fPar -> GetParInt("LKPulseExtraction/fixPedestal");
    if (fPar -> CheckPar("LKPulseExtraction/channelIsInverted")) fChannelIsInverted = fPar -> GetParBool("LKPulseExtraction/channelIsInverted");
    if (fPar -> CheckPar("LKPulseExtraction/tbRange")) {
        fTbRange1 = fPar -> GetParInt("LKPulseExtraction/tbRange",0);
        fTbRange2 = fPar -> GetParInt("LKPulseExtraction/tbRange",1);
    }
    if (fPar -> CheckPar("LKPulseExtraction/tbRangeCut")) {
        fTbRangeCut1 = fPar -> GetParInt("LKPulseExtraction/tbRangeCut",0);
        fTbRangeCut2 = fPar -> GetParInt("LKPulseExtraction/tbRangeCut",1);
    }
    if (fPar -> CheckPar("LKPulseExtraction/tbHeightCut")) {
        fPulseHeightCut1 = fPar -> GetParInt("LKPulseExtraction/tbHeightCut",0);
        fPulseHeightCut2 = fPar -> GetParInt("LKPulseExtraction/tbHeightCut",1);
    }
    if (fPar -> CheckPar("LKPulseExtraction/tbWidthCut")) {
        fPulseWidthCut1 = fPar -> GetParInt("LKPulseExtraction/tbWidthCut",0);
        fPulseWidthCut2 = fPar -> GetParInt("LKPulseExtraction/tbWidthCut",1);
    }
    
    fPulseAnalyzer = new LKPulseAnalyzer(fAnalysisName,fRun->GetDataPath());
    fPulseAnalyzer -> SetThreshold(fThreshold);
    fPulseAnalyzer -> SetTbRange(fTbRange1,fTbRange2);
    fPulseAnalyzer -> SetPulseTbCuts(fTbRangeCut1,fTbRangeCut2);
    fPulseAnalyzer -> SetPulseWidthCuts(fPulseWidthCut1,fPulseWidthCut2);
    fPulseAnalyzer -> SetPulseHeightCuts(fPulseHeightCut1,fPulseHeightCut2);
    fPulseAnalyzer -> SetInvertChannel(fChannelIsInverted);
    fPulseAnalyzer -> SetFixPedestal(fFixPedestal);

    return true;
}

void LKPulseExtractionTask::Exec(Option_t *option)
{
    int numChannels = fChannelArray -> GetEntries();
    for (int iChannel=0; iChannel<numChannels; iChannel++)
    {
        auto channel = (GETChannel *) fChannelArray -> At(iChannel);
        auto data = channel -> GetWaveformY();
        fPulseAnalyzer -> AddChannel(data);
    }

    lk_info << "Added " << numChannels << " channels, PA holding " << fPulseAnalyzer->GetNumGoodChannels() << endl;
    
}

bool LKPulseExtractionTask::EndOfRun()
{
    auto runHeader = fRun -> GetRunHeader();

    auto file1 = fPulseAnalyzer -> WriteReferencePulse(20,40);
    file1 -> cd();
    runHeader -> Write(runHeader->GetName(),TObject::kSingleKey);

    auto file2 = fPulseAnalyzer -> WriteTree();
    file2 -> cd();
    runHeader -> Write(runHeader->GetName(),TObject::kSingleKey);

    return true;
}
