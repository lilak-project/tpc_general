#ifndef USING_EJUNGWOO
void e_add(TObject *,const char* opt="") {}
void e_save(TObject *,const char* opt="") {}
#endif

TObjArray *fCvsList;
void e_cvs(TCanvas *cvs, const char* opt="");
void e_cvs_end();

int fNumCvsGroup = 10;
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
    run -> AddPar("config.mac");
    auto detector = new TexAT2();
    run -> AddDetector(detector);
    //run -> AddInputFile("/home/ejungwoo/lilak/texat_reco/macros_tracking/data/texat_0824.conv.root");
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

    LKPulseAnalyzer *ana[fNumTypes];
    for (auto iType : fSelTypes)
    {
        ana[iType] = new LKPulseAnalyzer(fTypeNames[iType]);
        ana[iType] -> SetTbStart(fTbStart[iType]);
        ana[iType] -> SetTbMax(350);
        ana[iType] -> SetThreshold(fThresholds[iType]);
        ana[iType] -> SetPulseHeightCuts(fPulseHeightMin,fPulseHeightMax);
        ana[iType] -> SetPulseTbCuts(fPulseTbMin,fPulseTbMax);
        ana[iType] -> SetPulseWidthCuts(fPulseWidthCuts[iType][0],fPulseWidthCuts[iType][1]);
        //ana[iType] -> SetCvsGroup(1000,800,4,5);
        ana[iType] -> SetCvsGroup(2800,2000,4,5);
        //ana[iType] -> SetCvsAverage(2800,2000,4,5);
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
            if (ana[type]->GetNumHistChannel()<20*fNumCvsGroup)
            //if (ana[type]->IsGoodChannel() && ana[type]->GetNumGoodChannels()<=20*fNumCvsGroup)
            {
                bool cvsIsNew = ana[type] -> DrawChannel();
                auto cvs = ana[type] -> GetGroupCanvas();
                if (cvsIsNew) {
                    e_cvs(cvs,fTypeNames[type]);
                }
            }
        }
    }

    int divX = 3;
    int divY = 3;
    if (fNumTypes>9) divX = 4;

    int dxCvs = 2800;
    int dyCvs = 2000;


    auto cvsAverageAll = new TCanvas("cvsAverageAll","cvsAverageAll",dxCvs,dyCvs);
    cvsAverageAll -> Divide(divX,divY);
    e_cvs(cvsAverageAll,"Summary");
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

    auto cvsAccumulateAll = new TCanvas("cvsAccumulateAll","cvsAccumulateAll",dxCvs,dyCvs);
    cvsAccumulateAll -> Divide(divX,divY);
    e_cvs(cvsAccumulateAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawAccumulate(cvsAccumulateAll->cd(iType+1));

    auto cvsWidthAll = new TCanvas("cvsWidthAll","cvsWidthAll",dxCvs,dyCvs);
    cvsWidthAll -> Divide(divX,divY);
    e_cvs(cvsWidthAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawWidth(cvsWidthAll->cd(iType+1));

    auto cvsHeightAll = new TCanvas("cvsHeightAll","cvsHeightAll",dxCvs,dyCvs);
    cvsHeightAll -> Divide(divX,divY);
    e_cvs(cvsHeightAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawHeight(cvsHeightAll->cd(iType+1));

    auto cvsPulseTbAll = new TCanvas("cvsPulseTbAll","cvsPulseTbAll",dxCvs,dyCvs);
    cvsPulseTbAll -> Divide(divX,divY);
    e_cvs(cvsPulseTbAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawPulseTb(cvsPulseTbAll->cd(iType+1));

    auto cvsPedestalAll = new TCanvas("cvsPedestalAll","cvsPedestalAll",dxCvs,dyCvs);
    cvsPedestalAll -> Divide(divX,divY);
    e_cvs(cvsPedestalAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawPedestal(cvsPedestalAll->cd(iType+1));

    auto cvsResidualAll = new TCanvas("cvsResidualAll","cvsResidualAll",dxCvs,dyCvs);
    cvsResidualAll -> Divide(divX,divY);
    e_cvs(cvsResidualAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawResidual(cvsResidualAll->cd(iType+1));

    for (auto iType : fSelTypes)
        ana[iType] -> WriteReferecePulse(0,20,"data");

    auto cvsReferenceAll = new TCanvas("cvsReferenceAll","cvsReferenceAll",dxCvs,dyCvs);
    cvsReferenceAll -> Divide(divX,divY);
    e_cvs(cvsReferenceAll,"Summary");
    for (auto iType : fSelTypes)
        ana[iType] -> DrawReference(cvsReferenceAll->cd(iType+1));

    e_cvs_end();
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

void e_cvs(TCanvas *cvs, const char* opt="")
{
    //e_add(cvs,opt);
    if (fCvsList==nullptr)
        fCvsList = new TObjArray();
    fCvsList -> Add(cvs);
}

void e_cvs_end()
{
    if (fCvsList!=nullptr)  {
        TIter next(fCvsList);
        while (auto cvs = (TCanvas *) next()) {
            cvs -> Update();
            cvs -> Modified();
            cvs -> SaveAs(Form("figures2/%s.png",cvs->GetName()));
        }
    }
}
