#include "ejungwooA.h"
#include "LKHoughWeightingFunction.h"

void drawWeightingFunction()
{
    int nx = 100;
    double xmin = 0.;
    double xmax = 4.;
    double dx = (xmax - xmin) / nx;
    LKHoughWeightingFunction *weightingFunction[3];
    weightingFunction[0] = new LKHoughWFConst();
    weightingFunction[1] = new LKHoughWFLinear();
    weightingFunction[2] = new LKHoughWFInverse();
    TString names[] = {
        "LKHoughWFConst",
        "LKHoughWFLinear",
        "LKHoughWFInverse",
    };
    auto cvs = ejungwoo::Canvas("cvsWeightingFunction",100,50,3,1);

    for (auto i : {0,1,2})
    {
        auto wf = weightingFunction[i];
        auto hist = new TH2D(Form("hist%d",i),names[i]+";distance / error;weight / weight_{point}",nx,xmin,xmax,100,0,1.2);
        auto graph = new TGraph();
        for (int i=0; i<nx; ++i)
        {
            double x = xmin + dx*i;
            auto weight = wf -> EvalFromDistance(x,1,1);
            graph -> SetPoint(graph->GetN(),x,weight);
        }
        cvs -> cd(i+1) -> SetGrid(1,1);
        hist -> SetStats(0);
        hist -> Draw();
        graph -> SetMarkerStyle(20);
        graph -> SetMarkerSize(0.8);
        graph -> Draw("samelp");
    }
}
