#include "LKLogger.h"
#include "ejungwooA.h"

void drawSimResult()
{
    gStyle -> SetOptStat(0);

    //for (const char* nameSim : {"idealPulse", "cleanPulse","dirtyPulse"})
    for (const char* nameSim : {"IdealPulse", "CleanPulse","MessyPulse"})
    {
        auto file = new TFile(Form("data/simulationSummary_%s.root",nameSim));
        auto pedestalFluctuationLevel = ((TParameter<double>*) file -> Get("pedestalFluctuationLevel")) -> GetVal();
        auto treeEvent = (TTree*) file -> Get("event");
        auto treeHit = (TTree*) file -> Get("hit");
        e_test << treeHit -> GetEntries() << endl;

        const char* nameHist1 = Form("histTb_%s",nameSim);
        const char* nameHist2 = Form("histAmp_%s",nameSim);
        const char* nameHist3 = Form("histTbDiff_%s",nameSim);
        const char* nameHist4 = Form("histTbDiffSaturated_%s",nameSim);
        const char* nameHist5 = Form("histAmpDiff_%s",nameSim);
        const char* nameHist6 = Form("histAmpDiffSaturated_%s",nameSim);
        const char* nameHist7 = Form("histRemainder_%s",nameSim);
        const char* nameHist8 = Form("histRemainderSaturated_%s",nameSim);

        const char* titleAll       = Form("[%s]  FL=%.2f  All"      ,nameSim,pedestalFluctuationLevel);;
        const char* titleSaturated = Form("[%s]  FL=%.2f  Saturated",nameSim,pedestalFluctuationLevel);;
        const char* titleAmp0L4096  = Form("[%s]  FL=%.2f  Amp0<4096",nameSim,pedestalFluctuationLevel);;

        double dTbMin  = 1.25*treeHit -> GetMinimum("tbSim-tbRec");
        double dTbMax  = 1.25*treeHit -> GetMaximum("tbSim-tbRec");
        double dAmpMin = 1.25*treeHit -> GetMinimum("ampSim-ampRec");
        double dAmpMax = 1.25*treeHit -> GetMaximum("ampSim-ampRec");
        if (dTbMin<-1) dTbMin=-1;
        if (dTbMax> 1) dTbMax= 1;
        if (dAmpMin<-100) dAmpMin=-100;
        if (dAmpMax> 100) dAmpMax= 100;

        auto hist1 = new TH2D(nameHist1,Form("%s;tbSim;tbRec",  titleAll),200,0,350,200,0,350);
        auto hist2 = new TH2D(nameHist2,Form("%s;ampSim;ampRec",titleAll),200,0,10000,200,0,10000);
        auto hist3 = new TH1D(nameHist3,Form("%s;tbSim-tbRec",  titleAmp0L4096),200,dTbMin,dTbMax);
        auto hist4 = new TH1D(nameHist4,Form("%s;tbSim-tbRec",  titleSaturated),200,dTbMin,dTbMax);
        auto hist5 = new TH1D(nameHist5,Form("%s;ampSim-ampRec",titleAmp0L4096),200,dAmpMin,dAmpMax);
        auto hist6 = new TH1D(nameHist6,Form("%s;ampSim-ampRec",titleSaturated),200,dAmpMin,dAmpMax);
        auto hist7 = new TH2D(nameHist7,Form("%s;tbSim-tbRec;tbRemainder",titleAmp0L4096),200,dTbMin,dTbMax,200,0,1);
        auto hist8 = new TH2D(nameHist8,Form("%s;tbSim-tbRec;tbRemainder",titleSaturated),200,dTbMin,dTbMax,200,0,1);

        treeHit -> Project(nameHist1,"tbRec:tbSim");
        treeHit -> Project(nameHist2,"ampRec:ampSim");
        treeHit -> Project(nameHist3,"tbSim-tbRec"                 ,"ampSim<4096");
        treeHit -> Project(nameHist4,"tbSim-tbRec"                 ,"ampSim>=4096");
        treeHit -> Project(nameHist5,"ampSim-ampRec"               ,"ampSim<4096");
        treeHit -> Project(nameHist6,"ampSim-ampRec"               ,"ampSim>=4096");
        treeHit -> Project(nameHist7,"tbSim-int(tbSim):tbSim-tbRec","ampSim<4096");
        treeHit -> Project(nameHist8,"tbSim-int(tbSim):tbSim-tbRec","ampSim>=4096");

        auto func3 = e_fit_gaus(hist3,1.5);
        auto func4 = e_fit_gaus(hist4,1.5);
        auto func5 = e_fit_gaus(hist5,1.5);
        auto func6 = e_fit_gaus(hist6,1.5);

        auto cvs = e_cvs(Form("cvsSimResult_%s",nameSim),"",2500,2000,3,3);
        cvs -> cd(1); hist1 -> Draw("colz");
        cvs -> cd(2); hist2 -> Draw("colz");
        cvs -> cd(4); hist7 -> Draw("colz");
        cvs -> cd(7); hist8 -> Draw("colz");
        cvs -> cd(5); hist3 -> Draw(); func3 -> Draw("same");
        cvs -> cd(8); hist4 -> Draw(); func4 -> Draw("same");
        cvs -> cd(6); hist5 -> Draw(); func5 -> Draw("same");
        cvs -> cd(9); hist6 -> Draw(); func6 -> Draw("same");

        double mean3 = func3 -> GetParameter(1);
        double mean4 = func4 -> GetParameter(1);
        double mean5 = func5 -> GetParameter(1);
        double mean6 = func6 -> GetParameter(1);
        double sigm3 = func3 -> GetParameter(2);
        double sigm4 = func4 -> GetParameter(2);
        double sigm5 = func5 -> GetParameter(2);
        double sigm6 = func6 -> GetParameter(2);

        hist3 -> SetTitle(Form("%s, MS=(%.2f, %.2f)", hist3->GetTitle(),mean3,sigm3));
        hist4 -> SetTitle(Form("%s, MS=(%.2f, %.2f)", hist4->GetTitle(),mean4,sigm4));
        hist5 -> SetTitle(Form("%s, MS=(%.2f, %.2f)", hist5->GetTitle(),mean5,sigm5));
        hist6 -> SetTitle(Form("%s, MS=(%.2f, %.2f)", hist6->GetTitle(),mean6,sigm6));
    }

    e_save_all();
}
