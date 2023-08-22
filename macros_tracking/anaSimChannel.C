#include "LKChannelSimulator.cpp"
#include "LKChannelAnalyzer.cpp"
#include "ejungwooA.h"

int fSimNumChannels = 1000000;
bool fSetDraw = true;
bool fSimIntegerTbHit = true;
bool fSimIntegerAmplitude = false;
bool fSimFluctuatingPedestal = false;
double fSimPulseErrorScale = 0;
double fSimPedestalErrorScale = 0;
int fSimNumHits = 1;
int fSimNumHitsMax = 2;
double fSimTbHit = -1;
double fSimAmplitude = -1;
double fSimTbHitRange1 = 0;
double fSimTbHitRange2 = 350;
double fSimAmplitudeRange1 = 100;
double fSimAmplitudeRange2 = 10000;
const char *fSimName = "test";
void SetIdealPulseSimulation();
void SetCleanPulseSimulation();
void SetMessyPulseSimulation();
void JustForDrawing();

void anaSimChannel(int setIdx=2)
{
    if (setIdx==0) SetIdealPulseSimulation();
    if (setIdx==1) SetCleanPulseSimulation();
    if (setIdx==2) SetMessyPulseSimulation();
    //JustForDrawing();

    //int seed = time(0);
    int seed = 1692677674;
    e_test << seed << endl;
    gRandom -> SetSeed(seed);
    gStyle -> SetOptStat(0);
    double anaThreshold = fSimAmplitudeRange1*0.8;

    int divX = 3;
    int divY = 2;
    if (fSimNumChannels==1) { divX = 1; divY = 1; }
    if (fSimNumChannels >1) { divX = 2; divY = 2; }
    if (fSimNumChannels >4) { divX = 3; divY = 2; }
    if (fSimNumChannels >6) { divX = 3; divY = 3; }
    if (fSimNumChannels >9) { divX = 4; divY = 3; }

    int    RecNumHitsCorrectr = false;
    int    bNumSimHits = 0;
    int    bNumRecHits = 0;
    int    bIndexHit = 0;
    double bTbHitSim = 0;
    double bAmplitudeSim = 0;
    double bTbHitRec = 0;
    double bAmplitudeRec = 0;
    double bChi2NDF = 0;
    double bNDF = 0;

    const char* pulseFile = "dataExample/pulseReference_MMCenter1.root";
    //const char* pulseFile = "data/pulseReference_MMCenter1.root";

    auto sim = new LKChannelSimulator();
    sim -> SetPulse(pulseFile);
    sim -> SetYMax(4096);
    sim -> SetTbMax(350);
    sim -> SetNumSmoothing(2);
    sim -> SetSmoothingLength(2);
    sim -> SetPedestalFluctuationLength(5);
    sim -> SetPedestalFluctuationScale(fSimPedestalErrorScale);
    sim -> SetPulseErrorScale(fSimPulseErrorScale);
    sim -> SetBackGroundLevel(0);
    sim -> SetCutBelow0(false);

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
    ana -> SetScaleTbStep(0.2);
    ana -> SetTbStepCut(0.01);

    auto pulse = ana -> GetPulse();

    int buffer[350] = {0};
    double tbHitArray[5] = {0};
    double amplitudeArray[5] = {0};

    const char *fileName = Form("data/simulationSummary_%s.root",fSimName);
    e_info << fileName << endl;

    auto file = new TFile(fileName,"recreate");
    (new TParameter<double>("pedestalFluctuationLevel",sim->GetPedestalFluctuationLevel())) -> Write();

    auto treeEvent = new TTree("event","");
    treeEvent -> Branch("correctN",&RecNumHitsCorrectr);
    treeEvent -> Branch("numSimHits",&bNumSimHits);
    treeEvent -> Branch("numRecHits",&bNumRecHits);

    auto treeHit = new TTree("hit","");
    treeHit -> Branch("correctN",&RecNumHitsCorrectr);
    treeHit -> Branch("tbSim", &bTbHitSim);
    treeHit -> Branch("ampSim", &bAmplitudeSim);
    treeHit -> Branch("tbRec", &bTbHitRec);
    treeHit -> Branch("ampRec", &bAmplitudeRec);
    treeHit -> Branch("chi2NDF", &bChi2NDF);
    treeHit -> Branch("ndf", &bNDF);

    int countSimForCvs = divX*divY+2;
    TCanvas* cvsGroup = nullptr;

    int xxx = fSimNumChannels/10;
    for (auto iSim=0; iSim<fSimNumChannels; ++iSim)
    {
        if (iSim%xxx==0) e_test << iSim << endl;

        memset(buffer, 0, sizeof(buffer));

        if (fSimFluctuatingPedestal) sim -> SetFluctuatingPedestal(buffer);
        else sim -> SetPedestal(buffer);

        int numSimHits;
        double tbHitSim;
        double amplitude;

        if (fSimNumHits>0) numSimHits = fSimNumHits;
        else 
            while (numSimHits>0)
                numSimHits = int(gRandom -> Integer(fSimNumHitsMax));

        for (auto iHit=0; iHit<numSimHits; ++iHit)
        {
            if (fSimTbHit>0) tbHitSim = fSimTbHit;
            else {
                tbHitSim = gRandom -> Uniform(fSimTbHitRange1,fSimTbHitRange2);
                if (fSimIntegerTbHit) tbHitSim = int(tbHitSim);
            }

            if (fSimAmplitude>0) amplitude = fSimAmplitude;
            else {
                amplitude = gRandom -> Uniform(fSimAmplitudeRange1,fSimAmplitudeRange2);
                if (fSimIntegerAmplitude) amplitude = int(amplitude);
            }

            tbHitArray[iHit] = tbHitSim;
            amplitudeArray[iHit] = amplitude;

            sim -> AddHit(buffer,tbHitSim,amplitude);
        }

        if (0)
        if (iSim==5895)
        {
            const char* fileName = "data/buffer_sim.dat";
            ofstream file_out(fileName);
            e_info << fileName << endl;
            for (auto tb=0; tb<350; ++tb)
                file_out << buffer[tb] << endl;
            return;
        }

        ana -> Clear();
        ana -> Analyze(buffer);
        auto numRecHits = ana -> GetNumHits();
#ifdef DEBUG_CHANA_FITPULSE
        auto cvsDebug = e_cvs(Form("cvsDebug_sim%d",iSim),"",3500,2000,3,2);
        int iCvs = 1;
        for (auto graph : { ana->dGraph_tb_chi2, ana->dGraph_tb_slope, ana->dGraph_it_slope, ana->dGraph_it_tbStep, ana->dGraph_it_tb, })
        {
            cvsDebug -> cd(++iCvs);
            auto frame = e_hist(graph,Form("frame%d%d",iSim,iCvs),Form("ex%d) %s",iSim,graph->GetTitle()));
            frame -> Draw();
            graph -> Draw("samepl");

            continue;
            if (graph==ana->dGraph_tb_chi2) {
                cvsDebug -> cd(iCvs);
                auto fit = new TF1(Form("fitTbC2_%d",iSim),"pol2",0,350);
                fit -> SetLineColor(kRed);
                fit -> SetLineStyle(2);
                graph -> Fit(fit,"QN0");
                fit -> Draw("samel");
            }
        }
        cvsDebug -> cd(1);

        cvsDebug -> Modified();
        cvsDebug -> Update();
#endif

        RecNumHitsCorrectr = false;
        bNumSimHits = numSimHits;
        bNumRecHits = numRecHits;
        if (numSimHits==numRecHits)
        {
            RecNumHitsCorrectr = true;
            for (bIndexHit=0; bIndexHit<numSimHits; ++bIndexHit)
            {
                bTbHitSim = tbHitArray[bIndexHit];
                bAmplitudeSim = amplitudeArray[bIndexHit];
                bTbHitRec = ana -> GetTbHit(bIndexHit);
                bAmplitudeRec = ana -> GetAmplitude(bIndexHit);
                bChi2NDF = ana -> GetChi2NDF(bIndexHit);
                bNDF = ana -> GetNDF(bIndexHit);
                treeHit -> Fill();
            }
        }
        treeEvent -> Fill();

        if (fSetDraw) {
            countSimForCvs++;
            if (countSimForCvs>divX*divY) {
                cvsGroup = e_cvs(Form("cvsSim%d",iSim),"",3000,2000,divX,divY);
                cvsGroup -> cd(1);
                countSimForCvs = 1;
            }
            else
                cvsGroup -> cd(countSimForCvs);

            auto histOriginal = new TH1D(Form("histSimulation_%d",iSim),Form("%d) sim/rec-hits = %d/%d;tb;y",iSim, numSimHits, numRecHits),350,0,350);
            for (auto tb=0; tb<350; ++tb)
                histOriginal -> SetBinContent(tb+1,buffer[tb]);
            histOriginal -> SetMaximum( histOriginal->GetBinContent(histOriginal->GetMaximumBin())*1.15);
            histOriginal -> SetMinimum(-histOriginal->GetBinContent(histOriginal->GetMaximumBin())*0.15);

            auto histSubtracted = new TH1D(Form("histSubtracted_%d",iSim),Form("%d) sim/rec-hits = %d/%d;tb;y",iSim, numSimHits, numRecHits),350,0,350);
            auto bufferSubtracted = ana -> GetBuffer();
            for (auto tb=0; tb<350; ++tb)
                histSubtracted -> SetBinContent(tb+1,bufferSubtracted[tb]);
            histSubtracted -> SetLineColor(kGreen+1);
            histSubtracted -> SetLineWidth(2);
            histSubtracted -> SetMaximum( histOriginal->GetBinContent(histOriginal->GetMaximumBin())*1.15);
            histSubtracted -> SetMinimum(-histOriginal->GetBinContent(histOriginal->GetMaximumBin())*0.15);

            auto line0 = new TLine(0,0,350,0);
            line0 -> SetLineStyle(2);

            histSubtracted -> Draw("same");
            histOriginal -> Draw("same");
            line0 -> Draw();

            for (auto iHit=0; iHit<numRecHits; ++iHit)
            {
                auto tbHit = ana -> GetTbHit(iHit);
                auto amplitude = ana -> GetAmplitude(iHit);
                auto graphFitted = ana -> GetPulse() -> GetPulseGraph(tbHit, amplitude);
                graphFitted -> Draw("samelx");
            }
        }
    }

    file -> cd();
    treeHit -> Write();
    treeEvent -> Write();
}

void SetIdealPulseSimulation()
{
    fSimName = "IdealPulse";
    //fSimNumChannels = 100000;
    fSetDraw = false;
    fSimIntegerTbHit = true;
    fSimIntegerAmplitude = false;
    fSimFluctuatingPedestal = false;
    fSimPulseErrorScale = 0;
    fSimPedestalErrorScale = 0;
    fSimNumHits = 1;
    fSimNumHitsMax = 2;
    fSimTbHit = -1;
    fSimAmplitude = -1;
    fSimTbHitRange1 = 0;
    fSimTbHitRange2 = 350;
    fSimAmplitudeRange1 = 100;
    fSimAmplitudeRange2 = 10000;
}

void SetCleanPulseSimulation()
{
    fSimName = "CleanPulse";
    //fSimNumChannels = 100000;
    fSetDraw = false;
    fSimIntegerTbHit = false;
    fSimIntegerAmplitude = false;
    fSimFluctuatingPedestal = true;
    fSimPulseErrorScale = 0.1;
    fSimPedestalErrorScale = 0.2;
    fSimNumHits = 1;
    fSimNumHitsMax = 2;
    fSimTbHit = -1;
    fSimAmplitude = -1;
    fSimTbHitRange1 = 0;
    fSimTbHitRange2 = 350;
    fSimAmplitudeRange1 = 100;
    fSimAmplitudeRange2 = 10000;
}

void SetMessyPulseSimulation()
{
    fSimName = "MessyPulse";
    //fSimNumChannels = 100000;
    fSetDraw = false;
    fSimIntegerTbHit = false;
    fSimIntegerAmplitude = false;
    fSimFluctuatingPedestal = true;
    fSimPulseErrorScale = 0.2;
    fSimPedestalErrorScale = 0.5;
    fSimNumHits = 1;
    fSimNumHitsMax = 2;
    fSimTbHit = -1;
    fSimAmplitude = -1;
    fSimTbHitRange1 = 0;
    fSimTbHitRange2 = 350;
    fSimAmplitudeRange1 = 100;
    fSimAmplitudeRange2 = 10000;
}

void JustForDrawing()
{
    fSimName = Form("%s_JD",fSimName);
    fSimNumChannels = 24;
    fSetDraw = true;
}
