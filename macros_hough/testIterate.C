#include "ejungwooA.h"

#include "LKHoughTransformTracker.cpp"

void testIterate()
{
    gStyle -> SetOptStat(0);

    int seed = time(0);
    seed = 1694408477;
    cout << seed << endl;
    gRandom -> SetSeed(seed);

    int numTracks = 1;
    int numBinsT = 25;
    int numBinsR = 25;
    int nx = 100;
    int ny = 100;
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

    auto DrawParamSpace = [](LKHoughTransformTracker* tk, int idx)
    {
        auto hist = tk -> GetHistParamSpace(Form("paramSpace%d",idx));
        hist -> Draw("colz");
        //hist -> Draw("lego2z");
        //hist -> Draw("arr");
    };
    
    auto DrawImageSpace = [nx,x1,x2,wx,ny,y1,y2,wy](LKHoughTransformTracker *tk, int idx)
    {
        auto hist = new TH2D(Form("frame%d",idx), "", nx, -150, +150, ny, 150, 450);
        auto hist2 = tk -> GetHistImageSpace(Form("imageSpace%d",idx));
        hist2 -> Draw("same colz");
        hist -> SetTitle(hist2->GetTitle());

        if (1)
        {
            double zmin = hist -> GetZaxis() -> GetXmin();
            double zmax = hist -> GetZaxis() -> GetXmax();
            hist -> Fill(x1-2.*wx,0.,zmax);
            hist -> GetZaxis() -> SetRangeUser(zmin, zmax);
            hist -> SetMinimum(zmin);
            hist -> SetMaximum(zmax);
            //hist -> Draw("");
            hist2 -> Draw("same colz");
        }
        else
            hist2 -> Draw("colz");
    };

    auto DrawTransformCenter = [xt, yt]()
    {
        auto marker = new TMarker(xt,yt,20);
        marker -> SetMarkerSize(1);
        marker -> SetMarkerColor(kBlack);
        marker -> Draw("same");

        marker = new TMarker(xt,yt,24);
        marker -> SetMarkerSize(2);
        marker -> SetMarkerColor(kBlack);
        marker -> Draw("same");
    };

    auto FindAndDraw = [x1,x2,y1,y2](LKHoughTransformTracker* tk, Int_t numPoints, TVirtualPad* pad, Color_t color, double cleanRange=-1)
    {
        gStyle -> SetPalette(kBird);
        for (auto iTrack=0; iTrack<numPoints; ++iTrack)
        {
            auto paramPoint = tk -> FindNextMaximumParamPoint();
            tk -> CleanLastParamPoint(0,cleanRange);
            if (paramPoint -> fWeight<-1)
                break;
            pad -> cd();
            if (0) {
                for (auto i : {0,1,2,3,4}) {
                    auto graph = paramPoint -> GetLineInImageSpace(i,x1,x2,y1,y2);
                    graph -> SetLineColor(color);
                    graph -> Draw("samel");
                }
            }
            if (1) {
                auto graph = paramPoint -> GetBandInImageSpace(x1,x2,y1,y2);
                graph -> SetFillColor(color);
                graph -> SetFillStyle(3344);
                graph -> Draw("samelf");
            }
            if (0) {
                auto track = tk -> FitTrackWithParamPoint(paramPoint);
                auto graph = track -> TrajectoryOnPlane(LKVector3::kX,LKVector3::kY);
                graph -> Draw("samel");
            }
        }
    };

    auto cvs = ejungwoo::Canvas("cvs",80,90,3,2);

    int iCvs = 0;
    for (auto i : {0,1,2})
    {
        tk1 -> Transform();

        ++iCvs;

        auto paramPointAtMax = tk1 -> FindNextMaximumParamPoint();
        auto graphHoughAtMax = paramPointAtMax -> GetRangeGraphInParamSpace(1);
        auto graphImageAtMax = paramPointAtMax -> GetBandInImageSpace(x1,x2,y1,y2);
        graphImageAtMax -> SetFillColor(kRed);
        graphImageAtMax -> SetFillStyle(3354);

        cvs -> cd(iCvs);
        DrawParamSpace(tk1,iCvs);

        auto paramPointRange = tk1 -> ReinitializeFromLastParamPoint();
        auto graphHoughReinit = paramPointRange -> GetRangeGraphInParamSpace(1);
        auto graphImageReinit = paramPointRange -> GetBandInImageSpace(x1,x2,y1,y2);
        graphImageReinit -> SetFillColor(33);
        graphImageReinit -> SetFillStyle(3345);

        cvs -> cd(iCvs);
        graphHoughAtMax -> Draw("samel");
        graphHoughReinit -> Draw("samel");

        cvs -> cd(iCvs+3);
        DrawImageSpace(tk1,iCvs);
        graphImageAtMax -> Draw("samelf");
        graphImageReinit -> Draw("samelf");
    }
}
