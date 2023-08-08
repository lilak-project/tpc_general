#include "LKPulseAnalyzer.cpp"
#include "LKPulseAnalyzer.h"

#ifndef USING_EJUNGWOO
void e_add(TObject *,const char* opt="") {}
void e_save(TObject *,const char* opt="") {}
#endif

int fNumCvsGroup = 1;
int fPulseHeightMin = 800;
int fPulseHeightMax = 4000;
int fPulseTbMin = 10;
int fPulseTbMax = 50;

const int fNumTypes = 11;
int eMMCenter      = 0;
int eMMLeftSide    = 1;
int eMMLeftCenter  = 2;
int eMMRightSide   = 3;
int eMMRightCenter = 4;
int efSiJunction   = 5;
int efSiOhmic      = 6;
int efCsI          = 7;
int eX6Ohmic       = 8;
int eX6Junction    = 9;
int eCsICT         = 10;
const char *fTypeNames[fNumTypes] = {"MMCenter", "MMLeftSide", "MMLeftCenter", "MMRightSide", "MMRightCenter", "fSiJunction", "fSiOhmic", "fCsI", "X6Ohmic", "X6Junction", "CsICT"};
bool fInvertChannel[fNumTypes] = {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1};
int fTbStart[] = {0,0,0,0,0,2,2,2,0,0,0};
int fThresholds[] = {700,700,700,700,700,450,600,400,650,500,500};
int fPulseHeightCuts[fNumTypes][2] = {{800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}};
int fPulseWidthCuts[fNumTypes][2] = {{20,40}, {20,40}, {20,40}, {20,40}, {20,40}, {20,40}, {20,40}, {2,10}, {20,40}, {10,25}, {10,30}};
//int fSelTypes[] = {eMMCenter};
//int fSelTypes[] = {eMMCenter, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter};
int fSelTypes[] = {eMMCenter, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter, efSiJunction, efSiOhmic, efCsI, eX6Ohmic, eX6Junction, eCsICT};


/*
const int fNumTypes = 7;
int eMMCenter      = 0;
int eMMLeftSide    = 0;
int eMMLeftCenter  = 0;
int eMMRightSide   = 0;
int eMMRightCenter = 0;
int efSiJunction   = 1;
int efSiOhmic      = 2;
int efCsI          = 3;
int eX6Ohmic       = 4;
int eX6Junction    = 5;
int eCsICT         = 6;
const char *fTypeNames[fNumTypes] = {"MicroMegas", "fSiJunction", "fSiOhmic", "fCsI", "X6Ohmic", "X6Junction", "CsICT"};
bool fInvertChannel[fNumTypes] = {0, 1, 0, 1, 0, 1, 1};
int fTbStart[] = {0,2,2,2,0,0,0};
int fThresholds[] = {700,450,600,400,650,500,500};
int fPulseHeightCuts[fNumTypes][2] = {{800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}, {800,4000}};
int fPulseWidthCuts[fNumTypes][2] = {{20,40}, {20,40}, {20,40}, {2,10}, {20,40}, {10,25}, {10,30}};
int fSelTypes[] = {eMMCenter, efSiJunction, efSiOhmic, efCsI, eX6Ohmic, eX6Junction, eCsICT};
//int fSelTypes[] = {eMMCenter, efCsI};
//int fSelTypes[] = {efCsI};
*/

int GetType(int cobo, int asad, int aget, int chan);

void anaExtractPulse()
{
    auto run = new LKRun();
    auto detector = new TexAT2();
    run -> AddDetector(detector);
    run -> AddInputFile("/home/ejungwoo/lilak/texat_reco/macros_tracking/data/texat_0824.conv.root");
    run -> SetTag("read");
    run -> Init();
    auto channelArray = run -> GetBranchA("RawData");

    LKPulseAnalyzer *ana[fNumTypes];
    for (auto iType : fSelTypes)
    {
        ana[iType] = new LKPulseAnalyzer(fTypeNames[iType]);
        ana[iType] -> SetTbStart(fTbStart[iType]);
        ana[iType] -> SetTbMax(350);
        ana[iType] -> SetThreshold(fThresholds[iType]);
        ana[iType] -> SetPulseHeightCuts(fPulseHeightMin,fPulseHeightMax);
        ana[iType] -> SetPulseTbCuts(fPulseTbMin,fPulseTbMax);//10,200);
        ana[iType] -> SetPulseWidthCuts(fPulseWidthCuts[iType][0],fPulseWidthCuts[iType][1]);
        ana[iType] -> SetCvsGroup(1000,800,4,5);
        ana[iType] -> SetInvertChannel(fInvertChannel[iType]);
    }

    auto numEvents = run -> GetNumEvents();
    for (auto iEvent=0; iEvent<numEvents; ++iEvent)
    {
        run -> GetEntry(iEvent);

        int numChannels = channelArray -> GetEntries();
        for (int iChannel=0; iChannel<numChannels; iChannel++)
        {
            auto channel = (MMChannel *) channelArray -> At(iChannel);
            auto cobo = channel -> GetCobo();
            auto asad = channel -> GetAsad();
            auto aget = channel -> GetAget();
            auto chan = channel -> GetChan();
            auto data = channel -> GetWaveformY();
            auto iall = 90000000 + cobo*100000 + asad*10000 + aget*1000 + chan;
            auto type = GetType(cobo,asad,aget,chan);

            bool selected = false;
            for (auto type0 : fSelTypes)
                if (type==type0)
                    selected = true;

            if (selected==false)
                continue;

            ana[type] -> AddChannel(iall,data);
            //if (ana[type]->GetNumHistChannel()<64)
            if (ana[type]->IsGoodChannel() && ana[type]->GetNumGoodChannels()<20*fNumCvsGroup)
            {
                bool cvsIsNew = ana[type] -> DrawChannel();
                auto cvs = ana[type] -> GetGroupCanvas();
                if (cvsIsNew) {
                    e_add(cvs,fTypeNames[type]);
                }
            }
        }
    }

    int divX = 3;
    int divY = 3;
    if (fNumTypes>9) divX = 4;

    auto cvsAverageAll = new TCanvas("cvsAverageAll","cvsAverageAll",1000,800);
    cvsAverageAll -> Divide(divX,divY);
    e_add(cvsAverageAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawAverage(cvsAverageAll->cd(iType+1));

    cvsAverageAll -> cd(divX*divY);
    auto frame = new TH2D("frameWidthAll",";y;width",100,0,100,100,0,60);
    frame -> SetStats(0);
    frame -> Draw();
    auto legend = new TLegend(0.5,0.5,0.85,0.95);
    for (auto iType : fSelTypes)
    {
        auto graph = new TGraph();
        graph -> SetMarkerStyle(20+iType);
        graph -> SetMarkerSize(0.6);
        graph -> SetMarkerColor(iType+1);
        graph -> SetLineColor  (iType+1);
        if (iType==4) {
            graph -> SetMarkerColor(1);
            graph -> SetLineColor  (1);
            graph -> SetLineStyle(2);
        }
        if (iType>8) {
            graph -> SetMarkerColor(iType+1-8);
            graph -> SetLineColor  (iType+1-8);
            graph -> SetLineStyle(2);
        }
        for (auto ratio : {0.05, 0.25, 0.50, 0.75})
        {
            double x0,x1,error;
            auto width = ana[iType]->FullWidthRatioMaximum(ana[iType]->GetHistAverage(),ratio,4,x0,x1,error);
            graph -> SetPoint(graph->GetN(),ratio*100,width);
        }
        graph -> Draw("samelp");
        legend -> AddEntry(graph,fTypeNames[iType],"lp");
    }
    legend -> Draw();

    auto cvsAccumulateAll = new TCanvas("cvsAccumulateAll","cvsAccumulateAll",1000,800);
    cvsAccumulateAll -> Divide(divX,divY);
    e_add(cvsAccumulateAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawAccumulate(cvsAccumulateAll->cd(iType+1));

    auto cvsWidthAll = new TCanvas("cvsWidthAll","cvsWidthAll",1000,800);
    cvsWidthAll -> Divide(divX,divY);
    e_add(cvsWidthAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawWidth(cvsWidthAll->cd(iType+1));

    auto cvsHeightAll = new TCanvas("cvsHeightAll","cvsHeightAll",1000,800);
    cvsHeightAll -> Divide(divX,divY);
    e_add(cvsHeightAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawHeight(cvsHeightAll->cd(iType+1));

    auto cvsPulseTbAll = new TCanvas("cvsPulseTbAll","cvsPulseTbAll",1000,800);
    cvsPulseTbAll -> Divide(divX,divY);
    e_add(cvsPulseTbAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawPulseTb(cvsPulseTbAll->cd(iType+1));

    auto cvsPedestalAll = new TCanvas("cvsPedestalAll","cvsPedestalAll",1000,800);
    cvsPedestalAll -> Divide(divX,divY);
    e_add(cvsPedestalAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawPedestal(cvsPedestalAll->cd(iType+1));

    auto cvsResidualAll = new TCanvas("cvsResidualAll","cvsResidualAll",1000,800);
    cvsResidualAll -> Divide(divX,divY);
    e_add(cvsResidualAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawResidual(cvsResidualAll->cd(iType+1));

    for (auto iType : fSelTypes)
        ana[iType] -> WriteReferecePulse(0,20,"data");

    auto cvsReferenceAll = new TCanvas("cvsReferenceAll","cvsReferenceAll",1000,800);
    cvsReferenceAll -> Divide(divX,divY);
    e_add(cvsReferenceAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawReference(cvsReferenceAll->cd(iType+1));
}

int GetType(int cobo, int asad, int aget, int chan) 
{
    if (cobo==0 && (asad==0 || asad==1)) return eMMCenter;
    if (cobo==0 && asad==2 && (aget==0 || aget==1)) return eMMLeftSide;
    if (cobo==0 && asad==2 && (aget==2 || aget==3)) return eMMLeftCenter;
    if (cobo==0 && asad==3 && (aget==0 || aget==1)) return eMMRightSide;
    if (cobo==0 && asad==3 && (aget==2 || aget==3)) return eMMRightCenter;
    if (cobo==1 && asad==0 && aget==0) return efSiJunction;
    if (cobo==1 && asad==0 && aget==1) return efSiOhmic;
    if (cobo==1 && asad==1) return efCsI;
    if (cobo==2 && aget==0) return eX6Ohmic;
    if (cobo==2 && (aget==1 || aget==2)) return eX6Junction;
    if (cobo==2 && aget==3) return eCsICT;

    return -1;
}
