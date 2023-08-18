#include "ejungwooA.h"
#include "LKPulseAnalyzer.cpp"
#include "LKPulseAnalyzer.h"
#include "LKChannelAnalyzer.cpp"
#include "LKChannelAnalyzer.h"
//#include "LKPulse.cpp"
#include "LKPulse.h"

void anaFitPulse(double scaleBeta = 0.2)
{
    //e_batch();

    int anaMode = 0;
    int iterationMax = 50;
    int scaleBeta100 = scaleBeta*100;

    gStyle -> SetOptStat(0);

    int bufferI[350] = {0};
    double buffer[350] = {0};
    const char *fileNames[] = {
        "dataExample/buffer_MMCenter1_1000057.dat",
        "dataExample/buffer_MMCenter1_10024.dat",
        "dataExample/buffer_MMCenter1_10028.dat",
        "dataExample/buffer_MMCenter1_2010023.dat",
        "dataExample/buffer_MMCenter1_2011044.dat",
        "dataExample/buffer_MMCenter1_22003013.dat",
        "dataExample/buffer_MMCenter1_1000057.dat",
        //"dataExample/buffer_MMCenter1_1002000.dat",
        //"dataExample/buffer_MMCenter1_12038.dat",
        //"dataExample/buffer_MMCenter1_2000054.dat",
        //"dataExample/buffer_MMCenter1_20011054.dat",
        //"dataExample/buffer_MMCenter1_22003013.dat",
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

        auto hist = new TH1D(Form("histChannel_%d_s%d",iFile,scaleBeta100),Form("%s;tb;",fileName),350,0,350);
        for (auto i=0; i<350; ++i)
            hist -> SetBinContent(i+1,buffer[i]);

        auto histSubtracted = new TH1D(Form("histSubtractedChannel_%d_s%d",iFile,scaleBeta100),Form("%s;tb;",fileName),350,0,350);

        auto anaC = new LKChannelAnalyzer();
        anaC -> SetPulse("dataExample/pulseReference_MMCenter1.root");
        anaC -> SetTbMax(350);
        anaC -> SetTbStart(1);
        anaC -> SetTbStartCut(330);
        anaC -> SetThreshold(200);
        anaC -> SetThresholdOneTbStep(2);
        anaC -> SetNumAcendingCut(5);
        anaC -> SetDynamicRange(4096);
        anaC -> SetIterMax(iterationMax);
        anaC -> SetScaleTbStep(scaleBeta);
        anaC -> SetTbStepCut(0.01);

        auto pulse = anaC -> GetPulse();
        auto ndfWL = int(pulse -> GetLeadingWidth()) + int(pulse->GetFWHM()/3);

        int tbPulse;
        int tbPeak;
        if (1)
        {
            int tbPointer = 0;
            anaC -> FindPeak(buffer, tbPointer, tbPulse);
            tbPeak = tbPulse + pulse -> GetLeadingWidth();
        }
        if (0)
        {
            anaP -> AddChannel(bufferI);
            tbPeak = anaP -> GetTbAtMaxValue();
            tbPulse = int(tbPeak - pulse->GetLeadingWidth());
        }

        if (anaMode==0)
        {
            int tbPointer = 0;
            anaC -> Analyze(buffer);
            auto numHits = anaC -> GetNumHits();

            auto cvsDebug = e_cvs(Form("cvsAna0_%d_s%d",iFile,scaleBeta100),"",1500,1000);
            hist -> SetMarkerStyle(24);
            hist -> SetMarkerColor(kBlack);
            hist -> Draw("");

            for (auto iHit=0; iHit<numHits; ++iHit)
            {
                auto tbHit = anaC -> GetTbHit(iHit);
                auto amplitude = anaC -> GetAmplitude(iHit);
                e_warning << iHit << " " << tbHit << " " << amplitude << endl;
                auto graphFitted = pulse -> GetPulseGraph(tbHit, amplitude);
                graphFitted -> SetFillColor(kGreen);
                //graphFitted -> Draw("samel3");
                graphFitted -> Draw("samelz");
            }

            auto bufferSubtracted = anaC -> GetBuffer();
            for (auto i=0; i<350; ++i)
                histSubtracted -> SetBinContent(i+1,bufferSubtracted[i]);

            histSubtracted -> SetLineColor(kBlack);
            histSubtracted -> SetLineStyle(2);
            histSubtracted -> Draw("same");
        }
        else if (anaMode==1)
        {
            double tbHit;
            double amplitude;

            int ndf = ndfWL;
            bool isSaturated;
            double chi2Fitted;
            anaC -> FitPulse(buffer, tbPulse, tbPeak, tbHit, amplitude, chi2Fitted, ndf, isSaturated);

#ifdef DEBUG_FITPULSE
            auto cvsDebug = e_cvs(Form("cvsDebug_%d_s%d",iFile,scaleBeta100),"",3500,2000,3,2);
            int iCvs = 1;
            for (auto graph : {
                    anaC->dGraphTbChi2,
                    anaC->dGraphTbBeta,
                    anaC->dGraphBeta,
                    anaC->dGraphTbStep,
                    anaC->dGraphTb,
                    }
                )
            {
                iCvs++;
                cvsDebug -> cd(iCvs);
                auto frame = e_hist(graph,Form("frame%d%d_s%d",iFile,scaleBeta100,iCvs),Form("ex%d) C = %.2f%s",iFile,scaleBeta,graph->GetTitle()));
                frame -> Draw();
                graph -> Draw("samepl");

                continue;
                if (graph==anaC->dGraphTbChi2) {
                    cvsDebug -> cd(iCvs);
                    auto fit = new TF1(Form("fitTbC2_%d_s%d",iFile,scaleBeta100),"pol2",0,350);
                    fit -> SetLineColor(kRed);
                    fit -> SetLineStyle(2);
                    graph -> Fit(fit,"QN0");
                    fit -> Draw("samel");
                }
            }
            cvsDebug -> cd(1);
            hist -> GetXaxis() -> SetRangeUser(tbPulse-2,tbPulse+55);
            hist -> SetMarkerStyle(20);
            hist -> SetMarkerColor(kBlack);
            hist -> Draw("p");
            auto graphFitted = pulse -> GetPulseGraph(tbHit, amplitude);
            graphFitted -> SetFillColor(kGreen);
            graphFitted -> Draw("samelpz");
            hist -> Draw("samep");

            cvsDebug -> Modified();
            cvsDebug -> Update();
#endif
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
            auto cvsChannel = e_cvs(Form("channelFit_%d_s%d",iFile,scaleBeta100));
            hist -> Draw();
            auto graphFitted = pulse -> GetPulseGraph(tbAtLeastChi2, amplitudeAtLeastChi2);
            graphFitted -> SetFillColor(kGreen);
            graphFitted -> Draw("samel3");
            hist -> Draw("same");

            auto cvsChi2 = e_cvs(Form("chi2_%d_s%d",iFile,scaleBeta100));
            graphChi2 -> SetMarkerStyle(20);
            //graphChi2 -> SetMarkerSize(0.4);
            graphChi2 -> Draw("apl");
        }
    }

    e_save_all();
}
