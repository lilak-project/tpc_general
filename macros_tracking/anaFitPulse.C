#include "ejungwooA.h"
#include "setChannels.h"
#include "LKPulseAnalyzer.h"
#include "LKChannelAnalyzer.h"
#include "LKPulse.h"

int fCvsDX = 1200;
int fCvsDY = 720;
int fCvsGroupDX = 3;
int fCvsGroupDY = 2;
int fNumCvsGroup = 0;
int fMaxInputFiles = 1;
const char* fDataPath = "data";
int fNumChannels;
int fIdxChannel = 0;
bool fEndOfEvent = true;
TClonesArray* fChannelArray = nullptr;
LKRun* fRun;
TexAT2* fDetector;
//LKChannelAnalyzer *fChannelAnalyzer[fNumTypes];
TCanvas *fCvs;
TCanvas *fCvsDebug;

void NextChannel(int pass=0);

void anaFitPulse(double scaleBeta = 0.7)
{
    fRun = new LKRun();
    fRun -> AddDetector(new TexAT2);
    fRun -> AddPar("config.mac");

    //ifstream file_list_conv("/home/ejungwoo/data/texat/conv/list_conv");
    ifstream file_list_conv("list_conv");
    int countFiles = 0;
    TString fileName;
    while (file_list_conv>>fileName) {
        if (countFiles>=fMaxInputFiles) break;
        fRun -> AddInputFile(fileName);
        countFiles++;
    }

    fRun -> SetTag("read");
    fRun -> Init();
    fDetector = (TexAT2*) fRun -> GetDetector();
    fDetector -> InitChannelAnalyzer();
    fChannelArray = fRun -> GetBranchA("RawData");

    for (auto type : fSelTypes)
    {
        TString pulseFile = Form("data100/pulseReference_%s.root",fTypeNames[type]);
        int anaThreshold = 150;

        /*
        auto ana = new LKChannelAnalyzer();
        ana -> SetPulse(pulseFile);
        ana -> SetTbMax(350);
        ana -> SetTbStart(1);
        ana -> SetTbStartCut(330);
        ana -> SetThreshold(anaThreshold);
        ana -> SetThresholdOneStep(2);
        ana -> SetNumTbAcendingCut(5);
        ana -> SetDynamicRange(4096);
        ana -> SetIterMax(15);
        //ana -> SetIterMax(2);
        ana -> SetScaleTbStep(0.2);
        //ana -> SetTbStepCut(0.01);
        ana -> SetTbStepCut(0.000001);
        */
        //fChannelAnalyzer[type] = ana;
    }

#ifdef DEBUG_CHANA_FITPULSE
    fCvsDebug = ejungwoo::Canvas("cvsd",0,0,3,2);
    fCvs = (TCanvas *) fCvsDebug -> cd(1);
#else
    fCvs = e_cvs("cvs");
#endif

    NextChannel();
    if (0) {
        lk_logger("fit11");
        NextChannel(11);
        NextChannel();
        NextChannel();
        NextChannel();
        NextChannel();
        NextChannel();
        NextChannel();
        NextChannel();
        NextChannel();
        NextChannel();
    }
    if (0) {
        lk_logger("fit20");
        NextChannel(20);
    }
}

void NextChannel(int pass)
{
    if (fIdxChannel==fNumChannels) {
        fEndOfEvent = true;
        fIdxChannel = 0;
    }
    if (fEndOfEvent) {
        fRun -> GetNextEvent();
        fEndOfEvent = false;
        fNumChannels = fChannelArray -> GetEntries();
    }

    if (pass>0) {
        pass--;
        fIdxChannel++;
        NextChannel(pass);
        return;
    }

    if (pass<0) {
        fIdxChannel = fIdxChannel + pass - 1;
        if (fIdxChannel<0) {
            e_warning << "Impossible job!" << endl;
            return;
        }
        NextChannel();
        return;
    }

    auto channel = (MMChannel *) fChannelArray -> At(fIdxChannel++);
    auto cobo = channel -> GetCobo();
    auto asad = channel -> GetAsad();
    auto aget = channel -> GetAget();
    auto chan = channel -> GetChan();
    auto data = channel -> GetWaveformY();
    //auto type = GetType(cobo,asad,aget,chan);
    auto caac = fRun->GetCurrentEventID()*1000000 + cobo*100000 + asad*10000 + aget*1000 + chan;
    bool selected = false;
    auto type = fDetector -> GetElectronicsID(cobo, asad, aget, chan);
    for (auto type0 : fSelTypes)
        if (type==type0)
            selected = true;
    if (selected==false)
        return;

    //auto ana = fChannelAnalyzer[type];
    auto ana = fDetector -> GetChannelAnalyzer(type);

    double data2[350];
    for (auto tb=0; tb<350; ++tb)
        data2[tb] = double(data[tb]);

    ana -> Analyze(data2);

#ifdef DEBUG_CHANA_FITPULSE
    {
        int iCvs = 1;
        for (auto graph : {ana->dGraph_tb_chi2, ana->dGraph_tb_slope, ana->dGraph_it_slope, ana->dGraph_it_tbStep, ana->dGraph_it_tb,})
        //for (auto graph : {ana->dGraph_it_slope})
        {
            fCvsDebug -> cd(++iCvs);
            TH2D* frame;
            if (graph==ana->dGraph_it_slope) {
                ejungwoo::Range rg0;
                ejungwoo::Range ry1(0, 0.01, false);
                ejungwoo::Range ry2(0, 0.01, true);
                frame = ejungwoo::Frame(graph, Form("frame%lld%d",caac,iCvs), Form("ex%lld) %s",caac,graph->GetTitle()), rg0, rg0, ry1, ry2);
            }
            else
                frame = e_frame(graph, Form("frame%lld%d",caac,iCvs), Form("ex%lld) %s",caac,graph->GetTitle()));
            frame -> Draw();
            graph -> Draw("samepl");

            //continue;
            if (graph==ana->dGraph_tb_chi2) {
                fCvsDebug -> cd(iCvs);
                auto fit = new TF1(Form("fitTbC2_%lld",caac),"pol2",0,350);
                fit -> SetLineColor(kRed);
                fit -> SetLineStyle(2);
                graph -> Fit(fit,"QN0");
                fit -> Draw("samel");
            }
        }
        fCvsDebug -> Modified();
        fCvsDebug -> Update();
    }
#endif

    TString nameCvs = Form("cvs_%s_%lld",fTypeNames[type],caac);
    TString nameHist = Form("hist_%s_%lld",fTypeNames[type],caac);
    TString namePedestal = Form("hists_%s_%lld",fTypeNames[type],caac);

    fCvs -> cd();
    auto hist = ejungwoo::MakeChannelHist(data2,350,nameHist);
    hist -> Draw();

    auto numHits = ana -> GetNumHits();
    e_test << numHits << endl;
    for (auto iHit=0; iHit<numHits; ++iHit)
    {
        auto tbHit = ana -> GetTbHit(iHit);
        auto amplitude = ana -> GetAmplitude(iHit);
        auto graph = ana -> GetPulse() -> GetPulseGraph(tbHit,amplitude);
        graph -> Draw("samelx");
    }

    auto histSubtracted = ejungwoo::MakeChannelHist(ana->GetBuffer(),350,namePedestal);
    histSubtracted -> SetLineColor(kBlack);
    histSubtracted -> SetLineStyle(2);
    histSubtracted -> Draw("same");

    fCvs -> Modified();
    fCvs -> Update();
}
