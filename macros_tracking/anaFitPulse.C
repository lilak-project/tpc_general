#include "LKPulseAnalyzer.cpp"
#include "LKPulseAnalyzer.h"
#include "LKChannelAnalyzer.cpp"
#include "LKChannelAnalyzer.h"
//#include "LKPulse.cpp"
#include "LKPulse.h"

TH2D *MakeHist(TGraph *graph, const char* name, const char* title);

void anaFitPulse()
{
    e_batch();

    gStyle -> SetOptStat(0);

    int bufferI[350] = {0};
    double buffer[350] = {0};
    const char *fileNames[] = {
        "example/buffer_MMCenter1_1002000.dat",
        "example/buffer_MMCenter1_12038.dat",
        "example/buffer_MMCenter1_2000054.dat",
        "example/buffer_MMCenter1_20011054.dat",
        "example/buffer_MMCenter1_22003013.dat",
    };

    auto anaP = new LKPulseAnalyzer("ana%d","data");

    int iFile = 0;
    for (auto fileName : fileNames)
    {
        iFile++;

        ifstream file_buffer(fileName);
        int iTb = 0;
        double value;
        while (file_buffer>>value) {
            buffer[iTb] = value;
            bufferI[iTb] = int(value);
            ++iTb;
        }
        file_buffer.close();

        auto hist = new TH1D(Form("histChannel_%d",iFile),"",350,0,350);
        for (auto i=0; i<350; ++i)
            hist -> SetBinContent(i+1,buffer[i]);

        auto anaC = new LKChannelAnalyzer();
        anaC -> Init();
        anaC -> SetPulse("example/pulseReference_MMCenter1.root");
        auto pulse = anaC -> GetPulse();

        anaP -> AddChannel(bufferI);
        int tbPulse = anaP -> GetFirstPulseTb() - 10;
        int tbPeak = anaP -> GetTbAtMaxValue();
        if (tbPeak<1) tbPeak = 1;
        if (tbPulse<1) tbPulse = tbPeak - 10;
        if (tbPulse<1) 
            continue;
        lk_debug << tbPulse << " " << tbPeak << endl;

        bool testWithAna = true;

        if (testWithAna)
        {
            int    ndf = 40;
            bool   isSaturated;
            double tbHit;
            double amplitude;
            double chi2Fitted;
            anaC -> FitPulse(buffer, tbPulse, tbPeak, tbHit, amplitude, chi2Fitted, ndf, isSaturated);

            auto cvsDebug = e_cvs(Form("cvsDebug_%d",iFile),"",3000,2000,3,2);
            auto hist1 = MakeHist(anaC -> dGraphStep, Form("hist1_%d",iFile), ";i;dtb ");
            auto hist2 = MakeHist(anaC -> dGraphTime, Form("hist2_%d",iFile), ";i;time");
            auto hist3 = MakeHist(anaC -> dGraphChi2, Form("hist3_%d",iFile), ";i;chi2");
            auto hist4 = MakeHist(anaC -> dGraphBeta, Form("hist4_%d",iFile), ";i;tbStep");
            auto hist5 = MakeHist(anaC -> dGraphTbC2, Form("hist5_%d",iFile), ";tb;chi2");
            cvsDebug -> cd(1); hist1 -> Draw(); anaC -> dGraphStep -> Draw("samepl");
            cvsDebug -> cd(2); hist2 -> Draw(); anaC -> dGraphTime -> Draw("samepl");
            cvsDebug -> cd(3); hist3 -> Draw(); anaC -> dGraphChi2 -> Draw("samepl");
            cvsDebug -> cd(4); hist4 -> Draw(); anaC -> dGraphBeta -> Draw("samepl");
            cvsDebug -> cd(5); hist5 -> Draw(); anaC -> dGraphTbC2 -> Draw("samepl");
            cvsDebug -> cd(6);
            hist -> GetXaxis() -> SetRangeUser(tbPulse-5,tbPulse+60);
            hist -> SetMarkerStyle(20);
            hist -> SetMarkerSize(0.5);
            hist -> SetMarkerColor(kBlack);
            hist -> Draw("p");
            auto graphFitted = pulse -> GetPulseGraph(tbHit, amplitude);
            lk_debug << tbHit << " " << amplitude << " " << chi2Fitted << endl;
            graphFitted -> SetFillColor(kGreen);
            graphFitted -> Draw("samelpz");
            hist -> Draw("samep");
            cvsDebug -> Modified();
            cvsDebug -> Update();
        }
        else
        {
            auto graphChi2 = new TGraph();
            double leastChi2 = DBL_MAX;
            double tbAtLeastChi2 = -1;
            double amplitudeAtLeastChi2 = -1;
            double chi2 = 0;
            double amplitude = 0;

            for (double tbTest=tbPulse-2; tbTest<=tbPulse+10; tbTest+=0.50)
            { 
                anaC -> LeastSquareFitAtGivenTb(buffer, tbTest, 30, amplitude, chi2);
                graphChi2 -> SetPoint(graphChi2->GetN(),tbTest,chi2);
                if (chi2<leastChi2) {
                    amplitudeAtLeastChi2 = amplitude;
                    tbAtLeastChi2 = tbTest;
                    leastChi2 = chi2;
                }
            }
            auto cvsChannel = e_cvs(Form("channelFit_%d",iFile));
            hist -> Draw();
            auto graphFitted = pulse -> GetPulseGraph(tbAtLeastChi2, amplitudeAtLeastChi2);
            graphFitted -> SetFillColor(kGreen);
            graphFitted -> Draw("samel3");
            hist -> Draw("same");

            auto cvsChi2 = e_cvs(Form("chi2_%d",iFile));
            graphChi2 -> SetMarkerStyle(20);
            graphChi2 -> SetMarkerSize(0.4);
            graphChi2 -> Draw("apl");
        }
    }
}

TH2D *MakeHist(TGraph *graph, const char* name, const char* title)
{
    int nbins = graph -> GetN();
    double x1=DBL_MAX, x2=-DBL_MAX, y1=DBL_MAX, y2=-DBL_MAX;
    for (auto iPoint=0; iPoint<nbins; ++iPoint) {
        double x0, y0;
        graph -> GetPoint(iPoint,x0,y0);
        if (x0<x1) x1 = x0;
        if (x0>x2) x2 = x0;
        if (y0<y1) y1 = y0;
        if (y0>y2) y2 = y0;
    }
    x1 = x1 - (x2-x1) * 0.05;
    x2 = x2 + (x2-x1) * 0.05;
    y1 = y1 - (y2-y1) * 0.05;
    y2 = y2 + (y2-y1) * 0.05;
    auto hist = new TH2D(name,title,100,x1,x2,100,y1,y2);
    return hist;
}
