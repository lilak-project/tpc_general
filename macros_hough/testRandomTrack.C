#include "ejungwooA.h"

#include "LKHoughTransformTracker.cpp"

void testRandomTrack()
{
    gStyle -> SetOptStat(0);

    int seed = time(0);
    seed = 1693993116;
    cout << seed << endl;
    gRandom -> SetSeed(seed);

    int numTracks = 2;
    int numRandom = 50;
    int numBinsT = 150;
    int numBinsR = 150;
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
    double yErrMax = .2*(y2/ny);
    TVector3 transformCenter((x1+x2)/2.,y2,0);

    auto hist = new TH2D("hist",Form("%d",seed),nx,x1,x2,ny,y1,y2);
    hist -> SetStats(0);

    auto tk2 = new LKHoughTransformTracker();
    tk2 -> SetTransformCenter(transformCenter);
    tk2 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk2 -> SetHoughSpaceBins(numBinsR, numBinsT);
    tk2 -> SetCorrelateBoxLine();

    auto tk3 = new LKHoughTransformTracker();
    tk3 -> SetTransformCenter(transformCenter);
    tk3 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk3 -> SetHoughSpaceBins(numBinsR, numBinsT);
    tk3 -> SetCorrelateBoxBand();

    auto tk4 = new LKHoughTransformTracker();
    tk4 -> SetTransformCenter(transformCenter);
    tk4 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk4 -> SetHoughSpaceBins(numBinsR, numBinsT);
    tk4 -> SetCorrelateDistance();

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

    for (auto binx=1; binx<=nx; ++binx) {
        for (auto biny=1; biny<=ny; ++biny) {
            auto content = hist -> GetBinContent(binx,biny);
            if (content>0) {
                auto xCenter = hist->GetXaxis()->GetBinCenter(binx);
                auto yCenter = hist->GetYaxis()->GetBinCenter(biny);
                tk2 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
                tk3 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
                tk4 -> AddImagePoint(xCenter,.5*dx, yCenter,.5*dy, content);
            }
        }
    }

    auto FindAndDraw = [x1,x2,y1,y2](LKHoughTransformTracker* tk, Int_t numPoints, TVirtualPad* pad, Color_t color, double cleanRange=-1)
    {
        gStyle -> SetPalette(kBird);
        for (auto iTrack=0; iTrack<numPoints; ++iTrack) {
            auto houghPoint = tk -> GetNextMaximumHoughPoint();
            tk -> CleanLastHoughPoint(cleanRange,cleanRange);
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
                graph -> SetFillStyle(3244);
                graph -> Draw("samelf");
            }
            if (1) {
                auto track = tk -> FitTrackWithHoughPoint(houghPoint);
                track -> Print();
                auto graph = track -> TrajectoryOnPlane(LKVector3::kX,LKVector3::kY);
                graph -> Draw("samel");
            }
        }
    };

    tk2 -> Transform();
    tk3 -> Transform();
    tk4 -> Transform();

    auto cvs = ejungwoo::Canvas("cvs",90,100,3,2);

    cvs -> cd(4); tk2 -> GetHistHoughSpace("b2") -> Draw("colz");
    cvs -> cd(5); tk3 -> GetHistHoughSpace("b3") -> Draw("colz");
    cvs -> cd(6); tk4 -> GetHistHoughSpace("b4") -> Draw("colz");

    cvs -> cd(1); tk2 -> GetHistImageSpace("imageSpace2") -> Draw("colz"); FindAndDraw(tk2,2,cvs->cd(1),kGreen);
    cvs -> cd(2); tk3 -> GetHistImageSpace("imageSpace3") -> Draw("colz"); FindAndDraw(tk3,2,cvs->cd(2),kGreen);
    cvs -> cd(3); tk4 -> GetHistImageSpace("imageSpace4") -> Draw("colz"); FindAndDraw(tk4,2,cvs->cd(3),kGreen);

    e_save_all();
}
