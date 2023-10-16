#include "ejungwooA.h"

#include "LKHTLineTracker.cpp"

void testCorrelators()
{
    gStyle -> SetOptStat(0);

    int seed = time(0);
    //seed = 1694313874;
    seed = 1694408477;
    cout << seed << endl;
    gRandom -> SetSeed(seed);

    int numTracks = 1;
    int numBinsT = 50;
    int numBinsR = 50;
    int nx = 50;
    int ny = 50;
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

    auto tk1 = new LKHTLineTracker();
    tk1 -> SetTransformCenter(xt, yt);
    tk1 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk1 -> SetParamSpaceBins(numBinsR, numBinsT);
    tk1 -> SetCorrelatePointBand();

    auto tk2 = new LKHTLineTracker();
    tk2 -> SetTransformCenter(xt, yt);
    tk2 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk2 -> SetParamSpaceBins(numBinsR, numBinsT);
    tk2 -> SetCorrelateBoxLine();

    auto tk3 = new LKHTLineTracker();
    tk3 -> SetTransformCenter(xt, yt);
    tk3 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk3 -> SetParamSpaceBins(numBinsR, numBinsT);
    //tk3 -> SetCorrelateBoxBand();
    tk3 -> SetCorrelateBoxRBand();

    auto tk4 = new LKHTLineTracker();
    tk4 -> SetTransformCenter(xt, yt);
    tk4 -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
    tk4 -> SetParamSpaceBins(numBinsR, numBinsT);
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

    auto cvs = ejungwoo::Canvas("cvs",100,90,4,2);

    int iCvs = 0;
    for (auto tk : {tk1, tk2, tk3, tk4})
    {
        tk -> Transform();

        ++iCvs;

        cvs -> cd(iCvs+4);
        auto histParam = tk -> GetHistParamSpace(Form("paramSpace%d",iCvs));
        histParam -> Draw("colz");

        cvs -> cd(iCvs);
        auto histImage = new TH2D(Form("frame%d",iCvs), "", nx, x1, x2, ny, y1, y2);
        histImage -> Draw("colz");
        auto marker = new TMarker(xt,yt,20);
        marker -> SetMarkerSize(1);
        marker -> SetMarkerColor(kBlack);
        marker -> Draw("same");
        marker = new TMarker(xt,yt,24);
        marker -> SetMarkerSize(2);
        marker -> SetMarkerColor(kBlack);
        marker -> Draw("same");

        auto graphData = tk -> GetDataGraphImageSapce();

        for (auto iTrack=0; iTrack<numTracks; ++iTrack)
        {
            auto paramPoint = tk -> FindNextMaximumParamPoint();
            tk -> CleanLastParamPoint();//cleanRange,cleanRange);
            if (paramPoint -> fWeight<-1) break;
            cvs -> cd(iCvs);

            TGraph *graph;
            if (tk -> IsCorrelatePointBand()) graph = paramPoint -> GetBandInImageSpace(x1,x2,y1,y2);
            if (tk -> IsCorrelateBoxLine()) graph = paramPoint -> GetLineInImageSpace(0,x1,x2,y1,y2);
            if (tk -> IsCorrelateBoxBand()) graph = paramPoint -> GetBandInImageSpace(x1,x2,y1,y2);
            if (tk -> IsCorrelateBoxRBand()) graph = paramPoint -> GetRBandInImageSpace(x1,x2,y1,y2);
            if (tk -> IsCorrelateDistance()) graph = paramPoint -> GetLineInImageSpace(0,x1,x2,y1,y2);
            graph -> SetFillColor(kYellow);
            graph -> SetFillStyle(3344);
            graph -> Draw("samelf");

            auto track = tk -> FitTrackWithParamPoint(paramPoint);
            auto graph2 = track -> TrajectoryOnPlane(LKVector3::kX,LKVector3::kY);
            graph2 -> Draw("samel");

            cvs -> cd(iCvs+4);
            auto graphHoughAtMax = paramPoint -> GetRangeGraphInParamSpace(1);
            graphHoughAtMax -> Draw("samel");
        }

        cvs -> cd(iCvs);
        graphData -> SetMarkerStyle(20);
        graphData -> SetMarkerSize(0.5);
        if (tk -> IsCorrelatePointBand()) graphData -> Draw("samepx");
        if (tk -> IsCorrelateBoxLine()) graphData -> Draw("samepz");
        if (tk -> IsCorrelateBoxBand()) graphData -> Draw("samepz");
        if (tk -> IsCorrelateBoxRBand()) graphData -> Draw("samepz");
        if (tk -> IsCorrelateDistance()) graphData -> Draw("samepx");
    }

    //e_save_all();
}
