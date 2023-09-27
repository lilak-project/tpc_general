#include "ejungwooA.h"

#include "LKHoughTransformTracker.cpp"

void testMultiTracks()
{
    gStyle -> SetOptStat(0);

    int seed = time(0);
    seed = 1694408477;
    cout << seed << endl;
    gRandom -> SetSeed(seed);

    int numTracks = 2;
    int numBinsT = 20;
    int numBinsR = 20;
    int nx = 40;
    int ny = 40;
    double x1 = -120;
    double x2 = 120;
    double y1 = 150;
    double y2 = 450;
    double wx = (x2-x1);
    double wy = (y2-y1);
    double dx = (x2-x1)/nx;
    double dy = (y2-y1)/ny;
    double wMin = 100;
    double wMax = 1000;
    double yErrMax = 1*dy;
    double xt = (x1+x2)/2;
    double yt = y2;
    int numRandom = numBinsT;

    auto hist = new TH2D("hist",Form("%d",seed),nx,x1,x2,ny,y1,y2);
    hist -> SetStats(0);

    auto tk1 = new LKHoughTransformTracker();
    tk1 -> SetTransformCenter(xt, yt);
    tk1 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk1 -> SetParamSpaceBins(numBinsR, numBinsT);
    tk1 -> SetCorrelateBoxBand();

    auto func = new TF1("track",Form("[0]*(x-%f)+[1]+%f",(x1+x2)/2.,(y1+y2)/2.),x1,x2);
    for (auto iTrack=0; iTrack<numTracks; ++iTrack) {
        auto phi = gRandom->Uniform(0,TMath::Pi());
        auto slope = TMath::Tan(phi);
        auto interception = 0;
        func -> SetParameters(slope,interception);
        for (auto ix=0; ix<numRandom; ++ix) {
            auto x = gRandom -> Uniform(x1,x2);
            auto y = func -> Eval(x) + gRandom -> Gaus(0,yErrMax);
            if (y>y2||y<y1) {
                --ix;
                continue;
            }
            auto w = gRandom -> Uniform(wMin,wMax);
            hist -> Fill(x,y,w);
        }
    }

    int countImagePoints = 0;
    for (auto binx=1; binx<=nx; ++binx) {
        for (auto biny=1; biny<=ny; ++biny) {
            auto content = hist -> GetBinContent(binx,biny);
            if (content>0) {
                ++countImagePoints;
                //content = countImagePoints;
                hist -> SetBinContent(binx,biny,content);
                auto xCenter = hist->GetXaxis()->GetBinCenter(binx);
                auto yCenter = hist->GetYaxis()->GetBinCenter(biny);
                tk1 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
            }
        }
    }

    auto cvs = ejungwoo::Canvas("cvs",80,90,4,3);

    int iCvs = 0;
    for (auto iTrack=0; iTrack<numTracks; ++iTrack)
    {
        auto numIteration = 3;
        for (auto iTransform=0; iTransform<numIteration; ++iTransform)
        {
            tk1 -> Transform();

            iCvs++;

            auto paramPointAtMax = tk1 -> FindNextMaximumParamPoint();
            auto graphHoughAtMax = paramPointAtMax -> GetRangeGraphInParamSpace(1);
            auto graphImageAtMax = paramPointAtMax -> GetBandInImageSpace(x1,x2,y1,y2);
            graphImageAtMax -> SetFillColor(kRed);
            graphImageAtMax -> SetLineColor(kRed);
            graphImageAtMax -> SetLineStyle(1);
            //graphImageAtMax -> SetFillStyle(3354);
            graphImageAtMax -> SetFillStyle(3395);

            cvs -> cd(2*iCvs-1);
            auto histParam = tk1 -> GetHistParamSpace(Form("paramSpace%d",iCvs));
            histParam -> Draw("colz");

            TGraph *graphHoughReinit = nullptr;
            TGraph *graphImageReinit = nullptr;
            if (iTransform!=numIteration-1) {
                auto paramPointRange = tk1 -> ReinitializeFromLastParamPoint();
                graphHoughReinit = paramPointRange -> GetRangeGraphInParamSpace(1);
                graphImageReinit = paramPointRange -> GetBandInImageSpace(x1,x2,y1,y2);
                graphImageReinit -> SetFillColor(kYellow);
                graphImageReinit -> SetFillStyle(3345);
            }

            graphHoughAtMax -> Draw("samel");
            if (graphHoughReinit!=nullptr) graphHoughReinit -> Draw("samel");

            cvs -> cd(2*iCvs);
            auto histImage = new TH2D(Form("frame%d",iCvs), "", nx, x1, x2, ny, y1, y2);
            histImage -> SetTitle(Form("%s (%dx%d), TC (x,y) = (%.2f, %.2f);#theta (degrees);Radius", tk1->GetCorrelatorName().Data(), nx, ny, xt, yt));
            histImage -> Draw("colz");
            {
                auto marker = new TMarker(xt,yt,20);
                marker -> SetMarkerSize(1);
                marker -> SetMarkerColor(kBlack);
                marker -> Draw("same");
                marker = new TMarker(xt,yt,24);
                marker -> SetMarkerSize(2);
                marker -> SetMarkerColor(kBlack);
                marker -> Draw("same");
            }
            graphImageAtMax -> Draw("samelf");
            if (graphImageReinit!=nullptr) graphImageReinit -> Draw("samelf");

            auto graphImageData = tk1 -> GetGraphImageSapce();
            graphImageData -> SetMarkerStyle(20);
            if (tk1 -> IsCorrelatePointBand()) graphImageData -> Draw("samepx");
            if (tk1 -> IsCorrelateBoxLine()) graphImageData -> Draw("samepz");
            if (tk1 -> IsCorrelateBoxBand()) graphImageData -> Draw("samepz");
            if (tk1 -> IsCorrelateBoxRBand()) graphImageData -> Draw("samepz");
            if (tk1 -> IsCorrelateDistance()) graphImageData -> Draw("samepx");

            if (iTransform==numIteration-1) {
                auto track = tk1 -> FitTrackWithParamPoint(paramPointAtMax);
                auto graphTrack = track -> TrajectoryOnPlane(LKVector3::kX,LKVector3::kY);
                graphTrack -> Draw("samel");
                tk1 -> Reset();
            }

        }
    }

    e_save_all();
}
