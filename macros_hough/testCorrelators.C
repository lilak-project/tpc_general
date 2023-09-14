#include "ejungwooA.h"

#include "LKHoughTransformTracker.cpp"

void testCorrelators()
{
    gStyle -> SetOptStat(0);

    int seed = time(0);
    //seed = 1694313874;
    seed = 1694408477;
    cout << seed << endl;
    gRandom -> SetSeed(seed);

    int numTracks = 1;
    int numBinsT = 30;
    int numBinsR = 30;
    int nx = 30;
    int ny = 30;
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
    tk1 -> SetHoughSpaceBins(numBinsR, numBinsT);
    tk1 -> SetCorrelatePointBand();

    auto tk2 = new LKHoughTransformTracker();
    tk2 -> SetTransformCenter(xt, yt);
    tk2 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk2 -> SetHoughSpaceBins(numBinsR, numBinsT);
    tk2 -> SetCorrelateBoxLine();

    auto tk3 = new LKHoughTransformTracker();
    tk3 -> SetTransformCenter(xt, yt);
    tk3 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk3 -> SetHoughSpaceBins(numBinsR, numBinsT);
    tk3 -> SetCorrelateBoxBand();

    auto tk4 = new LKHoughTransformTracker();
    tk4 -> SetTransformCenter(xt, yt);
    tk4 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk4 -> SetHoughSpaceBins(numBinsR, numBinsT);
    tk4 -> SetCorrelateDistance();
    tk4 -> SetMaxWeightingDistance(0.1*sqrt(wx*wx+wy+wy));

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
                tk2 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
                tk3 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
                tk4 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
            }
        }
    }

    auto DrawHoughSpace = [](LKHoughTransformTracker* tk, int idx)
    {
        auto hist = tk -> GetHistHoughSpace(Form("houghSpace%d",idx));
        hist -> Draw("colz");
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
            auto houghPoint = tk -> FindNextMaximumHoughPoint();
            tk -> CleanLastHoughPoint(cleanRange,cleanRange);
            //tk -> CleanLastHoughPoint(0,cleanRange);
            if (houghPoint -> fWeight<-1)
                break;
            pad -> cd();
            if (0) {
                for (auto i : {0,1,2,3,4}) {
                    auto graph = houghPoint -> GetLineInImageSpace(i,x1,x2,y1,y2);
                    graph -> SetLineColor(color);
                    graph -> Draw("samel");
                }
            }
            if (1) {
                auto graph = houghPoint -> GetBandInImageSpace(x1,x2,y1,y2);
                graph -> SetFillColor(color);
                graph -> SetFillStyle(3344);
                graph -> Draw("samelf");
            }
            if (1) {
                auto track = tk -> FitTrackWithHoughPoint(houghPoint);
                auto graph = track -> TrajectoryOnPlane(LKVector3::kX,LKVector3::kY);
                graph -> Draw("samel");
            }
        }
    };

    auto cvs = ejungwoo::Canvas("cvs",100,90,4,2);

    int iCvs = 0;
    for (auto tk : {tk1, tk2, tk3, tk4})
    {
        tk -> Transform();

        ++iCvs;

        cvs -> cd(iCvs+4);
        DrawHoughSpace(tk,iCvs);

        cvs -> cd(iCvs);
        DrawImageSpace(tk,iCvs);
        DrawTransformCenter();
        FindAndDraw(tk,1*numTracks,cvs->cd(iCvs),33);
    }
    //e_save_all();
}