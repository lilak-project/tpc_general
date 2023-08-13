#include "LKPulse.cpp"
#include "LKPulse.h"
#include "LKPulseAnalyzer.cpp"
#include "LKPulseAnalyzer.h"

#ifndef USING_EJUNGWOO
void e_add(TObject *,const char* opt="") {}
void e_save(TObject *,const char* opt="") {}
#endif

TObjArray *fCvsList;
void e_add_cvs(TCanvas *cvs, const char* opt="");
void e_end();

int fCvsDX = 1200;
int fCvsDY = 750;
int fCvsGroupDX = 2;
int fCvsGroupDY = 3;
int fNumCvsGroup = 1;

bool fDrawChannel = true;

bool fDrawAverage = false;
bool fDrawAccumulate = true;

bool fDrawMean  = false;
bool fDrawProjY = false;

bool fDrawHeightWidth = true;

bool fDrawWidth = false;
bool fDrawHeight = false;
bool fDrawPulseTb = false;
bool fDrawPedestal = false;
bool fDrawResidual = false;
bool fDrawReference = false;
bool fWriteReferencePulse = false;
int fCAAC = 0;
//int fCAAC = 22002;

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
//int fSelTypes[] = {eMMCenter};
int fSelTypes[] = {eMMCenter, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter};
//int fSelTypes[] = {eMMCenter, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter, efSiJunction, efSiOhmic, efCsI, eX6Ohmic, eX6Junction, eCsICT};
const char *fTypeNames[fNumTypes] = {"MMCenter", "MMLeftSide", "MMLeftCenter", "MMRightSide", "MMRightCenter", "fSiJunction", "fSiOhmic", "fCsI", "X6Ohmic", "X6Junction", "CsICT"};
bool fInvertChannel[fNumTypes] = {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1};
int fTbStart[] = {0,0,0,0,0,2,2,2,0,0,0};
int fPulseCuts[fNumTypes][7] = { // threshold, height1, height2, width1, width2, tb1, tb2
    {700,  700,4000, 20,40,  10,100},  // eMMCenter      = 0;
    {650,  650,4000, 20,40, 100,300},  // eMMLeftSide    = 1;
    {650,  650,4000, 20,40, 100,300},  // eMMLeftCenter  = 2;
    {750,  750,4000, 20,40, 100,300},  // eMMRightSide   = 3;
    {750,  750,4000, 20,40, 100,300},  // eMMRightCenter = 4;
    {450,  450,4000, 25,40,  10, 50},  // efSiJunction   = 5;
    {600,  600,4000, 25,40,  10, 50},  // efSiOhmic      = 6;
    {600,  800,4000,  2,10,  20,100},  // efCsI          = 7;
    {650,  800,4000, 20,40,  10, 50},  // eX6Ohmic       = 8;
    {600,  700,4000, 15,40,  10, 50},  // eX6Junction    = 9;
    {600,  700,4000, 15,40,  10, 50}   // eCsICT         = 10;
};

int GetType(int cobo, int asad, int aget, int chan);

void anaExtractPulse()
{
    auto run = new LKRun();
    run -> AddPar("config.mac");
    auto detector = new TexAT2();
    run -> AddDetector(detector);
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_801.27.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_801.28.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_801.36.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_806.28.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_806.29.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_806.3.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_806.30.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_821.25.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_821.3.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_821.30.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_821.31.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_821.32.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_821.4.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_821.7.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_822.3.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_822.4.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_823.10.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_823.17.root");
    run -> AddInputFile("/home/ejungwoo/data/texat/conv/texat_823.2.root");
    run -> SetTag("read");
    run -> Init();
    auto channelArray = run -> GetBranchA("RawData");

    LKPulse *pulse[fNumTypes];
    LKPulseAnalyzer *ana[fNumTypes];
    for (auto iType : fSelTypes)
    {
        pulse[iType] = new LKPulse(Form("data/pulseReference_%s.root",fTypeNames[iType]));

        ana[iType] = new LKPulseAnalyzer(fTypeNames[iType]);
        ana[iType] -> SetTbStart(fTbStart[iType]);
        ana[iType] -> SetTbMax(350);
        ana[iType] -> SetThreshold(fPulseCuts[iType][0]);
        ana[iType] -> SetPulseHeightCuts(fPulseCuts[iType][1],fPulseCuts[iType][2]);
        ana[iType] -> SetPulseWidthCuts(fPulseCuts[iType][3],fPulseCuts[iType][4]);
        ana[iType] -> SetPulseTbCuts(fPulseCuts[iType][5],fPulseCuts[iType][6]);
        ana[iType] -> SetCvsGroup(fCvsDX,fCvsDY,fCvsGroupDX,fCvsGroupDY);
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
            auto type = GetType(cobo,asad,aget,chan);
            auto caac = iEvent*1000000 + cobo*100000 + asad*10000 + aget*1000 + chan;

            bool selected = false;
            for (auto type0 : fSelTypes)
                if (type==type0)
                    selected = true;

            if (selected==false)
                continue;

            ana[type] -> AddChannel(caac,data);

            if (fDrawChannel)
            {
                if (ana[type]->GetNumHistChannel()<fCvsGroupDX*fCvsGroupDY*fNumCvsGroup)
                //if (ana[type]->IsGoodChannel() && ana[type]->GetNumGoodChannels()<=fCvsGroupDX*fCvsGroupDY*fNumCvsGroup)
                {
                    bool cvsIsNew = ana[type] -> DrawChannel();
                    auto cvs = ana[type] -> GetGroupCanvas();
                    if (cvsIsNew) {
                        e_add_cvs(cvs,fTypeNames[type]);
                    }
                }
            }

            if (fCAAC>0)
            {
                if (caac == fCAAC) {
                    cout << "double data[] = {";
                    for (auto tb=0; tb<350; ++tb) {
                        cout << data[tb] - ana[type]->GetPedestalPry() << ", ";
                    }
                    cout << "};" << endl;
                }
            }

        }
    }

    int divX = 1;
    int divY = 1;
    int divAX = 1;
    int divAY = 2;
    int fNumSel = 0; for (auto iType : fSelTypes) fNumSel++;
    if (fNumSel>1)  { divX = 1; divY = 2; }
    if (fNumSel>2)  { divX = 2; divY = 2; }
    if (fNumSel>4)  { divX = 3; divY = 2; }
    if (fNumSel>6)  { divX = 3; divY = 3; }
    if (fNumSel>9)  { divX = 4; divY = 3; }
    if (fNumSel>12) { divX = 5; divY = 3; }
    if (fNumSel>=1)  { divAX = 1; divAY = 2; }
    if (fNumSel>=2)  { divAX = 2; divAY = 2; }
    if (fNumSel>=4)  { divAX = 3; divAY = 2; }
    if (fNumSel>=6)  { divAX = 3; divAY = 3; }
    if (fNumSel>=9)  { divAX = 4; divAY = 3; }
    if (fNumSel>=12) { divAX = 5; divAY = 3; }

    if (fDrawAverage) {
        auto cvsAverageAll = new TCanvas("cvsAverageAll","cvsAverageAll",fCvsDX,fCvsDY);
        cvsAverageAll -> Divide(divAX,divAY);
        e_add_cvs(cvsAverageAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawAverage(cvsAverageAll->cd(iType+1));

        cvsAverageAll -> cd(divAX*divAY);
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
    }

    if (fDrawAccumulate) {
        auto cvsAccumulateAll = new TCanvas("cvsAccumulateAll","cvsAccumulateAll",fCvsDX,fCvsDY);
        cvsAccumulateAll -> Divide(divX,divY);
        e_add_cvs(cvsAccumulateAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawAccumulate(cvsAccumulateAll->cd(iType+1));
    }

    if (fDrawWidth) {
        auto cvsWidthAll = new TCanvas("cvsWidthAll","cvsWidthAll",fCvsDX,fCvsDY);
        cvsWidthAll -> Divide(divX,divY);
        e_add_cvs(cvsWidthAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawWidth(cvsWidthAll->cd(iType+1));
    }

    if (fDrawHeight) {
        auto cvsHeightAll = new TCanvas("cvsHeightAll","cvsHeightAll",fCvsDX,fCvsDY);
        cvsHeightAll -> Divide(divX,divY);
        e_add_cvs(cvsHeightAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawHeight(cvsHeightAll->cd(iType+1));
    }

    if (fDrawPulseTb) {
        auto cvsPulseTbAll = new TCanvas("cvsPulseTbAll","cvsPulseTbAll",fCvsDX,fCvsDY);
        cvsPulseTbAll -> Divide(divX,divY);
        e_add_cvs(cvsPulseTbAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawPulseTb(cvsPulseTbAll->cd(iType+1));
    }

    if (fDrawPedestal) {
        auto cvsPedestalAll = new TCanvas("cvsPedestalAll","cvsPedestalAll",fCvsDX,fCvsDY);
        cvsPedestalAll -> Divide(divX,divY);
        e_add_cvs(cvsPedestalAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawPedestal(cvsPedestalAll->cd(iType+1));
    }

    if (fDrawResidual) {
        auto cvsResidualAll = new TCanvas("cvsResidualAll","cvsResidualAll",fCvsDX,fCvsDY);
        cvsResidualAll -> Divide(divX,divY);
        e_add_cvs(cvsResidualAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawResidual(cvsResidualAll->cd(iType+1));
    }

    if (fDrawReference) {
        auto cvsReferenceAll = new TCanvas("cvsReferenceAll","cvsReferenceAll",fCvsDX,fCvsDY);
        cvsReferenceAll -> Divide(divX,divY);
        e_add_cvs(cvsReferenceAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawReference(cvsReferenceAll->cd(iType+1));
    }

    if (fDrawHeightWidth) {
        auto cvsHeightWidthAll = new TCanvas("cvsHeightWidthAll","cvsHeightWidthAll",fCvsDX,fCvsDY);
        cvsHeightWidthAll -> Divide(divX,divY);
        e_add_cvs(cvsHeightWidthAll,"Summary");
        for (auto iType : fSelTypes)
            ana[iType] -> DrawHeightWidth(cvsHeightWidthAll->cd(iType+1));
    }

    if (fDrawMean || fDrawProjY) {
        auto cvsMeanAll = new TCanvas("cvsMeanAll","cvsMeanAll",fCvsDX,fCvsDY);
        cvsMeanAll -> Divide(divX,divY);
        e_add_cvs(cvsMeanAll,"Summary");
        for (auto iType : fSelTypes) {
            ana[iType] -> DrawMean(cvsMeanAll->cd(iType+1));

            auto histArray = ana[iType] -> GetHistArray();
            auto numHists = histArray -> GetEntries(); 
            auto cvsProjYAll = new TCanvas(Form("cvsProjYAll_%s",fTypeNames[iType]),Form("cvsProjYAll_%s",fTypeNames[iType]),fCvsDX,fCvsDY);
            //cvsProjYAll -> Divide(4,5); if (numHists>20) numHists = 20;
            //cvsProjYAll -> Divide(2,2); if (numHists>4) numHists = 4;
            //cvsProjYAll -> Divide(4,3); if (numHists>12) numHists = 12;
            cvsProjYAll -> Divide(5,4); if (numHists>20) numHists = 20;
            for (auto iHist=0; iHist<numHists; ++iHist)
            {
                cvsProjYAll -> cd(iHist+1);
                auto hist = (TH1D*) histArray -> At(iHist+100);
                hist -> Draw();
            }
            e_add_cvs(cvsProjYAll,"Projection");
        }
    }

    if (fWriteReferencePulse) {
        for (auto iType : fSelTypes)
            ana[iType] -> WriteReferecePulse(0,20,"data");
    }

    e_end();
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
    if (cobo==1 && asad==1
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

void e_add_cvs(TCanvas *cvs, const char* opt="")
{
    return;
    e_add(cvs,opt);
    return;
    if (fCvsList==nullptr)
        fCvsList = new TObjArray();
    fCvsList -> Add(cvs);
}

void e_end()
{
    return;
    if (fCvsList!=nullptr)  {
        TIter next(fCvsList);
        while (auto cvs = (TCanvas *) next()) {
            cvs -> Update();
            cvs -> Modified();
            cvs -> SaveAs(Form("figures2/%s.png",cvs->GetName()));
        }
    }
}
