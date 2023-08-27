#include "LKChannelSimulator.cpp"
#include "LKChannelAnalyzer.cpp"
#include "ejungwooA.h"

vector<int> fSimIDArray;
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
//const char *fSimName = "test";
TString fSimName = "test";
void SetIdealPulseSimulation();
void SetCleanPulseSimulation();
void SetMessyPulseSimulation();
void SetTestPulseSimulation();
void JustForDrawing();
void DebugPulseFit(const char* fileName, int numHits);

void anaSimChannel(int setIdx=2)
{
    if (setIdx==0) SetIdealPulseSimulation();
    if (setIdx==1) SetCleanPulseSimulation();
    if (setIdx==2) SetMessyPulseSimulation();
    if (setIdx==3) SetTestPulseSimulation();
    JustForDrawing();
    //DebugPulseFit("data/simulationSelection_TestPulse.root",24);
    //DebugPulseFit("data/simulationSelection_TestPulse.cutt_ampRec_ampSim_split.root",24);
    //DebugPulseFit("data/simulationSelection_CleanPulse.cutt_ampRec_ampSim_split.root",24);
    //DebugPulseFit("data/simulationSelection_MessyPulse.cutt_tbRem_dTb_leftCross.root",24);

    //int seed = time(0);
    int seed = 1692677674;
    e_info << seed << endl;
    new TRandom3();
    gRandom -> SetSeed(seed);
    gStyle -> SetOptStat(0);
    double anaThreshold = fSimAmplitudeRange1*0.8;

    int divX = 3;
    int divY = 2;
    int numChannelsDraw = fSimNumChannels;
    if (fSimIDArray.size()>0) { numChannelsDraw = fSimIDArray.size(); }
    if (numChannelsDraw==1) { divX = 1; divY = 1; }
    if (numChannelsDraw >1) { divX = 2; divY = 2; }
    if (numChannelsDraw >4) { divX = 3; divY = 2; }
    //if (numChannelsDraw >6) { divX = 3; divY = 3; }
    //if (numChannelsDraw >9) { divX = 4; divY = 3; }

    int    bSimID = 0;
    int    bRecNumHitsCorrectr = false;
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

    const char *fileName = Form("data/simulationSummary_%s.root",fSimName.Data());
    e_info << fileName << endl;

    auto file = new TFile(fileName,"recreate");

    auto treeEvent = new TTree("event","");
    treeEvent -> Branch("correctN",&bRecNumHitsCorrectr);
    treeEvent -> Branch("numSimHits",&bNumSimHits);
    treeEvent -> Branch("numRecHits",&bNumRecHits);

    auto treeHit = new TTree("hit","");
    treeHit -> Branch("id", &bSimID);
    treeHit -> Branch("correctN", &bRecNumHitsCorrectr);
    treeHit -> Branch("tbSim", &bTbHitSim);
    treeHit -> Branch("ampSim", &bAmplitudeSim);
    treeHit -> Branch("tbRec", &bTbHitRec);
    treeHit -> Branch("ampRec", &bAmplitudeRec);
    treeHit -> Branch("chi2NDF", &bChi2NDF);
    treeHit -> Branch("ndf", &bNDF);

    int countSimForCvs = divX*divY+2;
    TCanvas* cvsGroup = nullptr;

    int fCountSimDebug = 0;

    int xxx = fSimNumChannels/10;
    if (xxx==0) xxx = 1;
    e_info << "numSimChannels: " << fSimNumChannels << " " << xxx << endl;
    for (auto iSim=0; iSim<fSimNumChannels; ++iSim)
    {
        bSimID = iSim;
        if (iSim%xxx==0)
            e_info << int(double(iSim)/fSimNumChannels*100) << " % " << endl;

        if (fSimIDArray.size()>0) {
            if (fCountSimDebug==fSimIDArray.size()) {
                break;
            }
            fSetDraw = false;
            for (auto selSim : fSimIDArray) {
                if (selSim==iSim) {
                    e_test << "Debugging " << iSim << endl;
                    fSetDraw = true;
                    fCountSimDebug++;
                    break;
                }
            }
        }

        memset(buffer, 0, sizeof(buffer));

        if (fSimFluctuatingPedestal) sim -> SetFluctuatingPedestal(buffer);
        else sim -> SetPedestal(buffer);

        int numSimHits;
        double tbHitSim;
        double amplitude;

        if (fSimNumHits>0) {
            numSimHits = fSimNumHits;
        }
        else 
            while (numSimHits>0) {
                numSimHits = int(gRandom -> Integer(fSimNumHitsMax));
            }

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
        //auto cvsDebug = e_cvs(Form("cvsDebug_sim%d",iSim),"",3500,2000,3,2);
        auto cvsDebug = e_cvs_full(Form("cvsDebug_sim%d",iSim));//,"",3500,2000,3,2);
        cvsDebug -> Divide(3,2);
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

        bRecNumHitsCorrectr = false;
        bNumSimHits = numSimHits;
        bNumRecHits = numRecHits;
        if (numSimHits==numRecHits)
        {
            bRecNumHitsCorrectr = true;
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

        e_test << fSimName << endl;

        if (fSetDraw)
        {
            countSimForCvs++;

            if (countSimForCvs>divX*divY) {
                const char* nameGroupCvs = Form("cvsSim_%s_%d",fSimName.Data(),iSim);
                cvsGroup = e_cvs_full(nameGroupCvs);//,"",3000,2000,divX,divY);
                cvsGroup -> Divide(divX,divY);
                cvsGroup -> cd(1);
                countSimForCvs = 1;
            }
            else
                cvsGroup -> cd(countSimForCvs);

            auto histSimulation = new TH1D(Form("histSimulation_%d",iSim),Form("%s-%d %d %d;tb;y",fSimName.Data(),iSim,numRecHits,numSimHits),350,0,350);
            auto histSubtracted = new TH1D(Form("histSubtracted_%d",iSim),Form("%s-%d %d %d;tb;y",fSimName.Data(),iSim,numRecHits,numSimHits),350,0,350);
            auto bufferSubtracted = ana -> GetBuffer();
            for (auto tb=0; tb<350; ++tb) {
                histSimulation -> SetBinContent(tb+1,buffer[tb]);
                histSubtracted -> SetBinContent(tb+1,bufferSubtracted[tb]);
            }

            histSimulation -> SetMaximum( histSimulation->GetBinContent(histSimulation->GetMaximumBin())*1.15);
            histSimulation -> SetMinimum(-histSimulation->GetBinContent(histSimulation->GetMaximumBin())*0.15);
            histSubtracted -> SetLineColor(kGreen+1);
            histSubtracted -> SetLineWidth(2);
            histSubtracted -> SetMaximum( histSimulation->GetBinContent(histSimulation->GetMaximumBin())*1.15);
            histSubtracted -> SetMinimum(-histSimulation->GetBinContent(histSimulation->GetMaximumBin())*0.15);

            auto line0 = new TLine(0,0,350,0);
            line0 -> SetLineStyle(2);

            histSubtracted -> Draw("same");
            histSimulation -> Draw("same");
            line0 -> Draw();

            int tbRangeUser1 = 350;
            int tbRangeUser2 = 0;
            for (auto iHit=0; iHit<numRecHits; ++iHit)
            {
                auto tbHit = ana -> GetTbHit(iHit);
                auto amplitude = ana -> GetAmplitude(iHit);
                auto graphFitted = pulse -> GetPulseGraph(tbHit, amplitude);
                graphFitted -> Draw("samelx");
                if (tbRangeUser1 > tbHit - 10) tbRangeUser1 = tbHit - 10;
                if (tbRangeUser2 < tbHit + pulse->GetTrailingWidth()) tbRangeUser2 = tbHit + pulse->GetTrailingWidth();
            }

            //if (fSimIDArray.size()>0) histSubtracted -> GetXaxis() -> SetRangeUser(tbRangeUser1,tbRangeUser2);
        }
    }

    file -> cd();
    treeHit -> Write();
    treeEvent -> Write();
    (new TParameter<double>("seed",seed)) -> Write();
    (new TParameter<double>("pedestalFluctuationLevel",sim->GetPedestalFluctuationLevel())) -> Write();

    e_save_all();
}

void SetIdealPulseSimulation()
{
    fSimName = "IdealPulse";
    //fSimIDArray.clear();
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
    //fSimIDArray.clear();
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
    //fSimIDArray.clear();
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

void SetTestPulseSimulation()
{
    fSimName = "TestPulse";
    //fSimIDArray.clear();
    fSimNumChannels = 10000;
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


void JustForDrawing()
{
    fSimName = Form("%s_JD",fSimName.Data());
    //fSimIDArray.clear();
    //fSimNumChannels = 100;
    fSimNumChannels = 24;
    fSetDraw = true;
}

void DebugPulseFit(const char* fileName, int numHitsUser)
{
    int bSimID = 0;
    auto file = new TFile(fileName);
    auto treeHit = (TTree*) file -> Get("hit");
    treeHit -> SetBranchAddress("id", &bSimID);
    auto numHits = treeHit -> GetEntries();
    if (numHitsUser>numHits) numHitsUser = numHits;
    for (auto iHit=0; iHit<numHitsUser; ++iHit) {
        treeHit -> GetEntry(iHit);
        fSimIDArray.push_back(bSimID);
    }
    file -> Close();
    fSimName = Form("%s_Debug",fSimName.Data());
    fSetDraw = false;
}
