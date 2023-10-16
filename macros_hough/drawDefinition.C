#include "ejungwooA.h"
#include "LKHTLineTracker.cpp"

void drawDefinition()
{
    gStyle -> SetOptStat(0);

    int nt = 10;
    int nr = 10;
    int nx = 100;
    int ny = 100;
    double x1 = 0;
    double x2 = 100;
    double y1 = 0;
    double y2 = 100;

    {
        auto tk = new LKHTLineTracker();
        tk -> SetTransformCenter(0,0);
        tk -> SetImageSpaceRange(nx,x1,x2,ny,y1,y2);
        tk -> SetParamSpaceBins(nr,nt);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();

        auto cvs = ejungwoo::Canvas("cvs",90,70,2,1);

        cvs -> cd(1);
        tk -> GetHistImageSpace() -> Draw();
        auto paramPoint = tk -> GetParamPoint(7,2);//new LKParamPointRT(0,0,40,30,60,45);
        auto band = paramPoint -> GetRibbonInImageSpace(x1,x2,y1,y2);
        band -> SetFillColor(kGray);
        band -> SetFillStyle(3135);
        band -> Draw("samelf");
        auto rband = paramPoint -> GetBandInImageSpace(x1,x2,y1,y2);
        rband -> SetFillColor(kCyan+1);
        rband -> SetFillStyle(3156);
        rband -> Draw("samelf");
        for (auto i : {0,1,2,3,4,5,7}) {
            auto mline = paramPoint -> GetLineInImageSpace(i,x1,x2,y1,y2);
            if (i==0) {
                mline -> SetLineColor(kRed+1);
                mline -> SetLineWidth(2);
            }
            mline -> Draw("samel");
            auto rline = paramPoint -> GetRadialLineInImageSpace(i,3);
            rline -> SetLineColor(kBlack);
            rline -> SetLineWidth(2);
            rline -> Draw("samel");
        }

        cvs -> cd(2);
        auto histParam = tk -> GetHistParamSpace("histParam");
        for (auto ir=1; ir<=nt; ++ir)
            for (auto it=1; it<=nr; ++it)
                histParam -> SetBinContent(ir,it,(it-1)*nt+ir);
        histParam -> Draw("text");
        auto paramBox = paramPoint -> GetRangeGraphInParamSpace();
        paramBox -> Draw("samel");
    }

    for (auto idx : {0,1,2,3,4})
    {
        auto tk = new LKHTLineTracker();
        tk -> SetTransformCenter(0,0);
        tk -> SetImageSpaceRange(nx,x1,x2,ny,y1,y2);
        tk -> SetParamSpaceBins(100,100);
        if (idx==0) tk -> SetCorrelatePointBand();
        if (idx==1) tk -> SetCorrelatePointBand();
        if (idx==2) tk -> SetCorrelateBoxLine();
        if (idx==3) tk -> SetCorrelateBoxBand();
        if (idx==4) tk -> SetCorrelateBoxRibbon();
        tk -> SetWFConst();

        auto cvs = ejungwoo::Canvas(Form("cvs_%d",idx),90,70,2,1);

        tk -> AddImagePoint(30,5,30,5,1);
        tk -> Transform();
        cvs -> cd(1);
        tk -> GetHistImageSpace(Form("histImage_%d",idx)) -> Draw();

        cvs -> cd(2);
        auto histParam = tk -> GetHistParamSpace(Form("histParam_%d",idx));
        histParam -> SetMaximum(2);
        histParam -> Draw("colz");

        if (idx!=0)
        for (auto i : {0,1,2,3,4})
        {
            LKParamPointRT* paramPoint = nullptr;
            if (i==0) paramPoint = tk->GetParamPoint(60,10);;
            if (i==1) paramPoint = tk->GetParamPoint(65,25);
            if (i==2) paramPoint = tk->GetParamPoint(60,45);
            if (i==3) paramPoint = tk->GetParamPoint(45,85);
            if (i==4) paramPoint = tk->GetParamPoint(46,85);

            cvs -> cd(1);
            TGraph *rband = nullptr;
            if (idx==1) rband = paramPoint -> GetBandInImageSpace(x1,x2,y1,y2);
            if (idx==2) rband = paramPoint -> GetLineInImageSpace(i,x1,x2,y1,y2);
            if (idx==3) rband = paramPoint -> GetBandInImageSpace(x1,x2,y1,y2);
            if (idx==4) rband = paramPoint -> GetRibbonInImageSpace(x1,x2,y1,y2);
            rband -> SetFillColor(kCyan+1);
            rband -> SetFillStyle(3156);
            rband -> Draw("samelf");

            cvs -> cd(2);
            auto paramBox = paramPoint -> GetRangeGraphInParamSpace();
            paramBox -> Draw("samel");
        }

        cvs -> cd(1);
        auto graphData = tk -> GetDataGraphImageSapce();
        if (idx==0||idx==1) {
            graphData -> SetMarkerStyle(20);
            graphData -> SetMarkerColor(kRed+1);
            graphData -> SetMarkerSize(2);
            graphData -> Draw("samepx");
        }
        else {
            graphData -> SetLineWidth(2);
            graphData -> Draw("samep");
        }
    }

    e_save_all();
}
