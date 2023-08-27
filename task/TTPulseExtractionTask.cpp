#include "TTPulseExtractionTask.h"

ClassImp(TTPulseExtractionTask);

TTPulseExtractionTask::TTPulseExtractionTask()
{
    fName = "TTPulseExtractionTask";
}

bool TTPulseExtractionTask::Init()
{
    // Put intialization todos here which are not iterative job though event
    lk_info << "Initializing TTPulseExtractionTask" << std::endl;

    fChannelArray = fRun -> GetBranchA("RawData");

    for (auto type : fSelTypes) {
        TString parName = Form("TTPulseExtractionTask/Cuts/%s",fTypeNames[type]);
        if (fPar->CheckPar(parName)) {
            for (auto iCut=0; iCut<10; ++iCut) {
                fExtractionCuts[type][iCut] = fPar -> GetParInt(parName, iCut);
            }
        }
    }

    for (auto type : fSelTypes) {
        fPulseAnalyzer[type] = new LKPulseAnalyzer(fTypeNames[type],fRun->GetDataPath());
        fPulseAnalyzer[type] -> SetTbRange(fExtractionCuts[type][7],350);
        fPulseAnalyzer[type] -> SetThreshold(fExtractionCuts[type][0]);
        fPulseAnalyzer[type] -> SetPulseHeightCuts(fExtractionCuts[type][1],fExtractionCuts[type][2]);
        fPulseAnalyzer[type] -> SetPulseWidthCuts(fExtractionCuts[type][3],fExtractionCuts[type][4]);
        fPulseAnalyzer[type] -> SetPulseTbCuts(fExtractionCuts[type][5],fExtractionCuts[type][6]);
        fPulseAnalyzer[type] -> SetInvertChannel(fExtractionCuts[type][8]);
        fPulseAnalyzer[type] -> SetFixPedestal(fExtractionCuts[type][9]);
    }

    return true;
}

void TTPulseExtractionTask::Exec(Option_t *option)
{
    auto eventID = fRun -> GetCurrentEventID();

    int numChannels = fChannelArray -> GetEntries();
    for (int iChannel=0; iChannel<numChannels; iChannel++)
    {
        auto channel = (MMChannel *) fChannelArray -> At(iChannel);
        auto cobo = channel -> GetCobo();
        auto asad = channel -> GetAsad();
        auto aget = channel -> GetAget();
        auto chan = channel -> GetChan();
        auto data = channel -> GetWaveformY();
        auto type = GetType(cobo,asad,aget,chan);
        auto caac = eventID*1000000 + cobo*100000 + asad*10000 + aget*1000 + chan;

        bool selected = false;
        for (auto type0 : fSelTypes)
            if (type==type0)
                selected = true;
        if (selected==false)
            continue;

        fPulseAnalyzer[type] -> AddChannel(data, eventID, cobo, asad, aget, chan);
    }

    lk_info << "TTPulseExtractionTask" << std::endl;
}

bool TTPulseExtractionTask::EndOfRun()
{
    for (auto type : fSelTypes) {
        auto runHeader = fRun -> GetRunHeader();
        auto file1 = fPulseAnalyzer[type] -> WriteReferencePulse(20,40);
        file1 -> cd();
        runHeader -> Write(runHeader->GetName(),TObject::kSingleKey);
        auto file2 = fPulseAnalyzer[type] -> WriteTree();
        file2 -> cd();
        runHeader -> Write(runHeader->GetName(),TObject::kSingleKey);
    }

    return true;
}

int TTPulseExtractionTask::GetType(int cobo, int asad, int aget, int chan) 
{
    if (cobo==0 && (asad==0 || asad==1)) {
        int mchannel = chan;
        if (chan>11) mchannel -= 1;
        if (chan>22) mchannel -= 1;
        if (chan>45) mchannel -= 1;
        if (chan>57) mchannel -= 1;

        if ((int(mchannel+1)/2)%2==0)
        {
            if (aget==0) return eMMCenter00;
            else if (aget==1) return eMMCenter01;
            else if (aget==2) return eMMCenter02;
            else if (aget==3) return eMMCenter03;
        }
        else
        {
            if (aget==0) return eMMCenter10;
            else if (aget==1) return eMMCenter11;
            else if (aget==2) return eMMCenter12;
            else if (aget==3) return eMMCenter13;
        }
    }
    if (cobo==0 && asad==2 && (aget==0 || aget==1)) return eMMLeftSide;
    if (cobo==0 && asad==2 && (aget==2 || aget==3)) return eMMLeftCenter;
    if (cobo==0 && asad==3 && (aget==0 || aget==1)) return eMMRightSide;
    if (cobo==0 && asad==3 && (aget==2 || aget==3)) return eMMRightCenter;
    if (cobo==1 && asad==0 && aget==0) return efSiJunction;
    if (cobo==1 && asad==0 && aget==1) return efSiOhmic;
    if (cobo==1 && asad==1 && aget==0
            && (chan==2
                || chan==7
                || chan==10
                || chan==16
                || chan==19
                || chan==25
                || chan==28
                || chan==33
                || chan==36
                || chan==41)
       ) 
        return efCsI;
    if (cobo==2 && aget==0) return eX6Ohmic;
    if (cobo==2 && (aget==1 || aget==2)) return eX6Junction;
    if (cobo==2 && aget==3) return eCsICT;

    return -1;
}
