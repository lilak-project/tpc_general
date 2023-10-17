#include "ejungwooA.h"
#include "LKHTLineTracker.cpp"

void drawDefinition()
{
    gStyle -> SetOptStat(0);

    bool draw_all_rep_param = true;
    bool draw_definitiion_param = true;
    bool draw_rep_param = true;
    bool draw_rep_image = true;

    if (draw_all_rep_param)
    {
        int nr = 2;
        int nt = 5;
        int nx = 100;
        int ny = 100;
        double x1 = 0;
        double x2 = 100;
        double y1 = 0;
        double y2 = 100;
        double xrange = 110;

        auto tk = new LKHTLineTracker();
        tk -> SetTransformCenter(0,0);
        tk -> SetImageSpaceRange(nx,x1,x2,ny,y1,y2);
        tk -> SetParamSpaceBins(nr,nt);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();

        auto cvs = ejungwoo::Canvas("cvs_param_all",80,70,2,1);

        cvs -> cd(1);
        auto hist = new TH2D("hist",";x;y",100,-xrange,xrange,100,-xrange,xrange);
        hist -> Draw();
        for (auto ir=0; ir<nr; ++ir)
            for (auto it=0; it<nt; ++it)
            {
                auto paramPoint = tk -> GetParamPoint(ir,it);
                auto line3d = paramPoint -> GetGeoLineInImageSpace(0,x1,x2,y1,y2);
                auto point1 = line3d.GetPointAtX(-xrange);
                auto point2 = line3d.GetPointAtX(xrange);
                auto graphModel = new TGraph();
                graphModel -> SetPoint(0,point1.X(),point1.Y());
                graphModel -> SetPoint(1,point2.X(),point2.Y());
                graphModel -> SetLineWidth(2);
                //if (ir==0||ir==3) { graphModel -> SetLineColor(44); }
                graphModel -> Draw("samel");

                auto rline = paramPoint -> GetRadialLineInImageSpace(0,3);
                rline -> SetLineColor(kGray+1);
                rline -> Draw("samel");
            }
        auto graphRange = new TGraph();
        graphRange -> SetPoint(0,x1,y1);
        graphRange -> SetPoint(1,x1,y2);
        graphRange -> SetPoint(2,x2,y2);
        graphRange -> SetPoint(3,x2,y1);
        graphRange -> SetPoint(4,x1,y1);
        graphRange -> SetLineColor(kBlue);
        graphRange -> SetFillColor(33);
        graphRange -> SetFillStyle(3002);
        graphRange -> Draw("samelf");

        cvs -> cd(2);
        auto histParam = tk -> GetHistParamSpace("histParam");
        for (auto ir=1; ir<=nt; ++ir)
            for (auto it=1; it<=nr; ++it)
                histParam -> SetBinContent(ir,it,(it-1)*nt+ir);
        histParam -> Draw("text col");
    }

    if (draw_definitiion_param)
    {
        int nt = 10;
        int nr = 10;
        int nx = 100;
        int ny = 100;
        double x1 = 0;
        double x2 = 100;
        double y1 = 0;
        double y2 = 100;

        auto tk = new LKHTLineTracker();
        tk -> SetTransformCenter(0,0);
        tk -> SetImageSpaceRange(nx,x1,x2,ny,y1,y2);
        tk -> SetParamSpaceBins(nr,nt);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();

        auto cvs = ejungwoo::Canvas("cvs_definition",40,70,1,1);

        cvs -> cd(1);
        tk -> GetHistImageSpace() -> Draw();
        auto paramPoint = tk -> GetParamPoint(7,1);
        auto mline = paramPoint -> GetLineInImageSpace(0,x1,x2,y1,y2);
        mline -> SetLineColor(kRed+1);
        mline -> SetLineWidth(2);
        mline -> Draw("samel");
        auto rline = paramPoint -> GetRadialLineInImageSpace(0,3);
        rline -> SetLineColor(kBlack);
        rline -> SetLineWidth(2);
        rline -> Draw("samel");
        LKGeoCircle circle(0,0,15);
        auto graphAngle = circle.DrawCircle(100,0,TMath::DegToRad()*paramPoint->GetT0());
        graphAngle -> SetLineWidth(2);
        graphAngle -> Draw("samel");
        auto tt = new TLatex(18.5,4.8,"#theta");
        tt -> SetTextAlign(22);
        tt -> SetTextFont(132);
        tt -> Draw();
        auto tt2 = new TLatex(31,21,"r");
        tt2 -> SetTextAlign(22);
        tt2 -> SetTextFont(132);
        tt2 -> SetTextSize(.060);
        tt2 -> Draw();
    }

    if (draw_rep_param)
    {
        int nt = 10;
        int nr = 10;
        int nx = 100;
        int ny = 100;
        double x1 = 0;
        double x2 = 100;
        double y1 = 0;
        double y2 = 100;

        auto tk = new LKHTLineTracker();
        tk -> SetTransformCenter(0,0);
        tk -> SetImageSpaceRange(nx,x1,x2,ny,y1,y2);
        tk -> SetParamSpaceBins(nr,nt);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();

        auto cvs = ejungwoo::Canvas("cvs_representation",80,70,2,1);

        cvs -> cd(1);
        tk -> GetHistImageSpace() -> Draw();
        auto paramPoint = tk -> GetParamPoint(7,2);
        auto band = paramPoint -> GetRibbonInImageSpace(x1,x2,y1,y2);
        band -> SetFillColor(kGray);
        band -> SetFillStyle(3253);
        band -> Draw("samelf");
        auto rband = paramPoint -> GetBandInImageSpace(x1,x2,y1,y2);
        rband -> SetFillColor(kCyan+1);
        rband -> SetFillStyle(3002);
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

    if (draw_rep_image)
        for (auto idx : {0,1,2,3,4})
        {
            int nt = 10;
            int nr = 10;
            int nx = 100;
            int ny = 100;
            double x1 = 0;
            double x2 = 100;
            double y1 = 0;
            double y2 = 100;

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

            auto cvs = ejungwoo::Canvas(Form("cvs_definition_correlator_%d",idx),80,70,2,1);

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
                    if (paramPoint->GetWeight()<=0)
                        rband -> SetFillColor(kGray);
                    rband -> SetFillStyle(3002);
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
