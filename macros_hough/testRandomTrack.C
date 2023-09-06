#include "ejungwooA.h"

#include "LKHoughTransformTracker.cpp"

void testRandomTrack()
{
    gStyle -> SetOptStat(0);

    int seed = time(0);
    //seed = 1693992527;
    seed = 1693993116;
    cout << seed << endl;
    gRandom -> SetSeed(seed);

    int numTracks = 2;
    int numRandom = 100;
    int numBinsT = 200;
    int numBinsR = 200;
    int numBinsT2 = 10;
    int numBinsR2 = 10;
    int nx = 100;
    int ny = 100;
    double x1 = -120;
    double x2 = 120;
    double y1 = 150;
    double y2 = 450;
    double dx = (x2-x1)/nx;
    double dy = (y2-y1)/ny;
    double wMin = 100;
    double wMax = 1000;
    double yErrMax = .2*(y2/ny);
    TVector3 transformCenter((x1+x2)/2.,y2,0);

    auto hist = new TH2D("hist",Form("%d",seed),nx,x1,x2,ny,y1,y2);
    hist -> SetStats(0);

    auto tk = new LKHoughTransformTracker();
    tk -> SetTransformCenter(transformCenter);
    tk -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk -> SetHoughSpaceBins(numBinsR, numBinsT);

    auto tk2 = new LKHoughTransformTracker();
    tk2 -> SetTransformCenter(transformCenter);
    tk2 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk2 -> SetHoughSpaceBins(numBinsR2, numBinsT2);

    auto func = new TF1("track",Form("[0]*(x-%f)+[1]+%f",(x1+x2)/2.,(y1+y2)/2.),x1,x2);
    for (auto iTrack=0; iTrack<numTracks; ++iTrack)
    {
        auto phi = gRandom->Uniform(0,TMath::Pi());
        auto slope = TMath::Tan(phi);
        auto interception = 0;//gRandom -> Uniform(y1+.4*dy,y2-.4*dy);
        func -> SetParameters(slope,interception);
        for (auto ix=0; ix<numRandom; ++ix) {
            auto x = gRandom -> Uniform(x1,x2);
            auto y = func -> Eval(x) + gRandom -> Gaus(0,yErrMax);
            auto w = gRandom -> Uniform(wMin,wMax);
            hist -> Fill(x,y,w);
        }
    }

    for (auto binx=1; binx<=nx; ++binx) {
        for (auto biny=1; biny<=ny; ++biny) {
            auto content = hist -> GetBinContent(binx,biny);
            if (content>0) {
                auto xCenter = hist->GetXaxis()->GetBinCenter(binx);
                auto yCenter = hist->GetYaxis()->GetBinCenter(biny);
                tk -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
                tk2 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
            }
        }
    }

    tk -> Transform();
    tk2 -> Transform();

    gStyle -> SetPalette(kBird);

    auto cvs = ejungwoo::Canvas("cvs",90,100,3,2);

    cvs -> cd(1);
    tk -> GetHistImageSpace() -> Draw("colz");
    auto x = transformCenter.X();
    auto y = transformCenter.Y();
    auto marker = new TMarker(x,y,42);
    marker -> SetMarkerSize(5);
    marker -> Draw();

    cvs -> cd(2);
    tk -> GetHistHoughSpace("before") -> Draw("colz");
    for (auto iTrack=0; iTrack<numTracks; ++iTrack) {
        auto houghPoint = tk -> FindNextMaximumHoughPoint();
        tk -> ClearLastMaximumHoughPoint();
        if (houghPoint.fWeight<-1)
            break;
        houghPoint.Print();
        cvs -> cd(1);
        auto line = houghPoint.GetGeoLine(0,x1,x2,y1,y2).DrawArrowXY(0);
        line -> Draw();
    }

    cvs -> cd(3);
    tk -> GetHistHoughSpace("after") -> Draw("colz");

    cvs -> cd(4);
    tk2 -> GetHistHoughSpace("before2") -> Draw("colz");
    {
        auto houghPoint = tk2 -> FindNextMaximumHoughPoint();
        tk2 -> ClearLastMaximumHoughPoint();
        if (houghPoint.fWeight<-1)
        houghPoint.Print();
        cvs -> cd(1);
        auto line = houghPoint.GetGeoLine(0,x1,x2,y1,y2).DrawArrowXY(0);
        line -> SetLineColor(kRed);
        line -> Draw();
    }
    cvs -> cd(5);
    tk2 -> GetHistHoughSpace("after2") -> Draw("colz");
    {
        auto houghPoint = tk2 -> FindNextMaximumHoughPoint();
        tk2 -> ClearLastMaximumHoughPoint();
        if (houghPoint.fWeight<-1)
        houghPoint.Print();
        cvs -> cd(1);
        auto line = houghPoint.GetGeoLine(0,x1,x2,y1,y2).DrawArrowXY(0);
        line -> SetLineColor(kRed);
        line -> Draw();
    }
    cvs -> cd(6);
    tk2 -> GetHistHoughSpace("after3") -> Draw("colz");

    for (auto iTrack=0; iTrack<10; ++iTrack) {
        auto houghPoint = tk2 -> FindNextMaximumHoughPoint();
        tk2 -> ClearLastMaximumHoughPoint();
        if (houghPoint.fWeight<-1)
            break;
        houghPoint.Print();
        cvs -> cd(1);
        auto line = houghPoint.GetGeoLine(0,x1,x2,y1,y2).DrawArrowXY(0);
        line -> SetLineColor(kGray);
        line -> Draw();
    }
}
