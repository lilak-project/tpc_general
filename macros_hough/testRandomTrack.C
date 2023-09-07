#include "ejungwooA.h"

#include "LKHoughTransformTracker.cpp"

void testRandomTrack()
{
    gStyle -> SetOptStat(0);

    int seed = time(0);
    seed = 1693993116;
    cout << seed << endl;
    gRandom -> SetSeed(seed);

    int numTracks = 1;
    int numRandom = 50;
    int numBinsT = 60;
    int numBinsR = 60;
    int nx = 80;
    int ny = 80;
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
    double yErrMax = dy;
    double xt = (x1+x2)/2;
    double yt = y2;

    auto hist = new TH2D("hist",Form("%d",seed),nx,x1,x2,ny,y1,y2);
    hist -> SetStats(0);

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
    //tk4 -> SetHoughSpaceRange(1,0,200,1,-120,120);
    //tk4 -> SetHoughSpaceRange(1,0,200,1,-10,10);
    //tk4 -> SetHoughSpaceRange(1,80,100,1,170,180);
    tk4 -> SetCorrelateDistance();
    tk4 -> SetMaxWeightingDistance(0.5*sqrt(wx*wx+wy+wy));

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

    auto DrawImageSpace = [nx,x1,x2,wx,ny,y1,y2,wy](LKHoughTransformTracker *tk, int idx)
    {
        auto hist = new TH2D(Form("frame%d",idx), "", nx, x1-wx, x2+wx, ny, y1-wy, y2+wy);
        auto hist2 = tk -> GetHistImageSpace(Form("imageSpace%d",idx));
        hist2 -> Draw("same colz");
        hist -> SetTitle(hist2->GetTitle());

        //double zmin = hist -> GetZaxis() -> GetXmin();
        //double zmax = hist -> GetZaxis() -> GetXmax();
        //hist -> Fill(x1-2.*wx,0.,zmax);
        //hist -> GetZaxis() -> SetRangeUser(zmin, zmax);
        //hist -> SetMinimum(zmin);
        //hist -> SetMaximum(zmax);

        //hist -> Draw("");
        //hist2 -> Draw("same colz");
        hist2 -> Draw("colz");
    };

    auto DrawTransformCenter = [xt, yt]()
    {
        auto marker = new TMarker(xt,yt,20);
        marker -> SetMarkerSize(2);
        marker -> SetMarkerColor(kBlack);
        marker -> Draw("same");

        marker = new TMarker(xt,yt,24);
        marker -> SetMarkerSize(5);
        marker -> SetMarkerColor(kBlack);
        marker -> Draw("same");
    };

    auto FindAndDraw = [x1,x2,y1,y2](LKHoughTransformTracker* tk, Int_t numPoints, TVirtualPad* pad, Color_t color, double cleanRange=-1)
    {
        //gStyle -> SetPalette(kCandy);
        //gStyle -> SetPalette(kPastel);
        gStyle -> SetPalette(kRainbow);
        //gStyle -> SetPalette(kBird);
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

    tk2 -> Transform();
    tk3 -> Transform();
    tk4 -> Transform();

    auto cvs = ejungwoo::Canvas("cvs",90,100,3,2);

    cvs -> cd(4); tk2 -> GetHistHoughSpace("b2") -> Draw("colz");
    cvs -> cd(5); tk3 -> GetHistHoughSpace("b3") -> Draw("colz");
    cvs -> cd(6); tk4 -> GetHistHoughSpace("b4") -> Draw("colz");

    cvs -> cd(1); DrawImageSpace(tk2,2); DrawTransformCenter(); FindAndDraw(tk2,numTracks,cvs->cd(1),33);
    cvs -> cd(2); DrawImageSpace(tk3,3); DrawTransformCenter(); FindAndDraw(tk3,numTracks,cvs->cd(2),33);
    cvs -> cd(3); DrawImageSpace(tk4,4); DrawTransformCenter(); FindAndDraw(tk4,numTracks,cvs->cd(3),33);

    //tk4 -> DrawAllHoughLines();
    //tk4 -> DrawAllHoughBands();

    //e_save_all();
}
