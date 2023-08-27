#include "LKLogger.h"
#include "ejungwooA.h"
#include "setCuts.h"

void drawSimResult()
{
    gStyle -> SetOptStat(0);

    TCut cutSaturated("ampSim< 4096");
    TCut cutWithinDyR("ampSim>=4096");

    TCut cutGeneral("cut0","");
    //TCut cutGeneral("cut_ndf","ndf>10");
    //TCut cutGeneral("cut_ndf_tb","ndf>10&&tbSim<300");

    TCut cutSelected("");
    //TCut cutSelected = cutt_tbRem_dTb_leftCross;
    //TCut cutSelected = cutt_tbRem_dTb_topPattern;
    //TCut cutSelected = cutt_ampRec_ampSim_split;
    //TCut cutSelected = cutt_ampRem_dTb_sideBranch;

    for (const char* nameSim : {"IdealPulse", "CleanPulse","MessyPulse"})
    //for (const char* nameSim : {"TestPulse"})
    {
        auto file = new TFile(Form("data/simulationSummary_%s.root",nameSim));
        auto pedestalFluctuationLevel = ((TParameter<double>*) file -> Get("pedestalFluctuationLevel")) -> GetVal();
        auto treeEvent = (TTree*) file -> Get("event");
        auto treeHit = (TTree*) file -> Get("hit");
        e_info << treeHit -> GetEntries() << endl;

        const char* titleAll       = Form("[%s]  FL=%.2f  All"      ,nameSim,pedestalFluctuationLevel);;
        const char* titleSaturated = Form("[%s]  FL=%.2f  Saturated",nameSim,pedestalFluctuationLevel);;
        const char* titleWithinDyR  = Form("[%s]  FL=%.2f  Amp0<4096",nameSim,pedestalFluctuationLevel);;

        const char* nameHist1 =  Form("histTb_%s",nameSim);
        const char* nameHist2 =  Form("histAmp_%s",nameSim);
        const char* nameHist3 =  Form("histTbDiff_%s",nameSim);
        const char* nameHist4 =  Form("histTbDiffSaturated_%s",nameSim);
        const char* nameHist5 =  Form("histAmpDiff_%s",nameSim);
        const char* nameHist6 =  Form("histAmpDiffSaturated_%s",nameSim);
        const char* nameHist7 =  Form("histRemainder_%s",nameSim);
        const char* nameHist8 =  Form("histRemainderSaturated_%s",nameSim);
        const char* nameHist9 =  Form("histNDF_%s",nameSim);
        const char* nameHist10 = Form("histChi2_%s",nameSim);
        const char* nameHist11 = Form("histTbDiffChi2_%s",nameSim);
        const char* nameHist12 = Form("histTbDiffChi2Saturated_%s",nameSim);
        const char* nameHist13 = Form("histTbDiffAmpSim_%s",nameSim);
        const char* nameHist14 = Form("histTbDiffAmpSimSaturated_%s",nameSim);

        auto hist1  = new TH2D(nameHist1, Form("%s;tbSim;tbRec",  titleAll),200,0,350,200,0,350);
        auto hist2  = new TH2D(nameHist2, Form("%s;ampSim;ampRec",titleAll),200,0,10000,200,0,10000);
        auto hist3  = new TH1D(nameHist3, Form("%s;tbSim-tbRec",  titleWithinDyR),200,-1,1);
        auto hist4  = new TH1D(nameHist4, Form("%s;tbSim-tbRec",  titleSaturated),200,-1,1);
        auto hist5  = new TH1D(nameHist5, Form("%s;ampSim-ampRec",titleWithinDyR),200,-100,100);
        auto hist6  = new TH1D(nameHist6, Form("%s;ampSim-ampRec",titleSaturated),200,-100,100);
        auto hist7  = new TH2D(nameHist7, Form("%s;tbSim-tbRec;tbRemainder",titleWithinDyR),200,-1,1,200,0,1);
        auto hist8  = new TH2D(nameHist8, Form("%s;tbSim-tbRec;tbRemainder",titleSaturated),200,-1,1,200,0,1);
        auto hist9  = new TH1D(nameHist9, Form("%s;NDF",titleSaturated),25,0,25);
        auto hist10 = new TH1D(nameHist10,Form("%s;Chi2",titleAll),200,0,0.15);
        auto hist11 = new TH2D(nameHist11,Form("%s;tbSim-tbRec;Chi2",titleWithinDyR),200,-1,1,200,0,0.15);
        auto hist12 = new TH2D(nameHist12,Form("%s;tbSim-tbRec;Chi2",titleSaturated),200,-1,1,200,0,0.15);
        auto hist13 = new TH2D(nameHist13,Form("%s;tbSim-tbRec;ampSim",titleWithinDyR),200,-1,1,200,0,10000);
        auto hist14 = new TH2D(nameHist14,Form("%s;tbSim-tbRec;ampSim",titleSaturated),200,-1,1,200,0,10000);

        treeHit -> Project(nameHist1 ,"tbRec:tbSim"                 ,cutGeneral);
        treeHit -> Project(nameHist2 ,"ampRec:ampSim"               ,cutGeneral);
        treeHit -> Project(nameHist3 ,"tbSim-tbRec"                 ,cutGeneral&&cutSaturated);
        treeHit -> Project(nameHist4 ,"tbSim-tbRec"                 ,cutGeneral&&cutWithinDyR);
        treeHit -> Project(nameHist5 ,"ampSim-ampRec"               ,cutGeneral&&cutSaturated);
        treeHit -> Project(nameHist6 ,"ampSim-ampRec"               ,cutGeneral&&cutWithinDyR);
        treeHit -> Project(nameHist7 ,"tbSim-int(tbSim):tbSim-tbRec",cutGeneral&&cutSaturated);
        treeHit -> Project(nameHist8 ,"tbSim-int(tbSim):tbSim-tbRec",cutGeneral&&cutWithinDyR);
        treeHit -> Project(nameHist9 ,"ndf"                         ,cutGeneral);
        treeHit -> Project(nameHist10,"chi2NDF"                     ,cutGeneral);
        treeHit -> Project(nameHist11,"chi2NDF:tbSim-tbRec"         ,cutGeneral&&cutSaturated);
        treeHit -> Project(nameHist12,"chi2NDF:tbSim-tbRec"         ,cutGeneral&&cutWithinDyR);
        treeHit -> Project(nameHist13,"ampSim:tbSim-tbRec"          ,cutGeneral&&cutSaturated);
        treeHit -> Project(nameHist14,"ampSim:tbSim-tbRec"          ,cutGeneral&&cutWithinDyR);

        auto func3 = e_fitg(hist3,1.5);
        auto func4 = e_fitg(hist4,1.5);
        auto func5 = e_fitg(hist5,1.5);
        auto func6 = e_fitg(hist6,1.5);

        auto cvs = e_cvs_full(Form("cvsSimResult_%s",nameSim));
        cvs -> Divide(4,3);
        cvs -> cd(1);  hist1 -> Draw("colz");
        cvs -> cd(2);  hist2 -> Draw("colz");
        cvs -> cd(3) -> SetLogy(); hist9 -> Draw();
        cvs -> cd(4);  hist10 -> Draw();

        cvs -> cd(5);  hist7 -> Draw("colz");
        cvs -> cd(9);  hist8 -> Draw("colz");

        cvs -> cd(6);  hist13 -> Draw("colz");//hist11 -> Draw("colz");
        cvs -> cd(10); hist14 -> Draw("colz");//hist12 -> Draw("colz");

        cvs -> cd(7);  hist3 -> Draw(); func3 -> Draw("same");
        cvs -> cd(11); hist4 -> Draw(); func4 -> Draw("same");
        //cvs -> cd(7);  hist13 -> Draw("colz");
        //cvs -> cd(11); hist14 -> Draw("colz");

        cvs -> cd(8);  hist5 -> Draw(); func5 -> Draw("same");
        cvs -> cd(12); hist6 -> Draw(); func6 -> Draw("same");

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

        if (cutSelected!="") {
            cutSelected = cutSelected&&cutGeneral;
            const char* fileName2 = Form("data/simulationSelection_%s.%s.root",nameSim,cutSelected.GetName());
            e_cout << fileName2 << endl;
            auto file2 = new TFile(fileName2,"recreate");
            auto treeSelection = treeHit -> CopyTree(cutSelected);
            //treeSelection -> Scan();
            treeSelection -> Write();
        }
    }

    e_save_all();
}
