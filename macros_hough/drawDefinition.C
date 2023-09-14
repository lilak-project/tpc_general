#include "ejungwooA.h"
#include "LKHoughTransformTracker.cpp"

void drawDefinition()
{
    int nx = 100;
    int ny = 100;
    double x1 = 0;
    double x2 = 100;
    double y1 = 0;
    double y2 = 100;

    auto cvs = ejungwoo::Canvas("cvs",1,1);
    auto hist = new TH2D("hist","",nx,x1,x2,ny,y1,y2);
    hist -> Draw();

    auto houghPoint = new LKHoughPointRT(0,0,40,30,60,45);
    auto band = houghPoint -> GetBandInImageSpace(x1,x2,y1,y2);
    band -> SetFillColor(33);
    band -> SetFillStyle(3344);
    band -> Draw("samelf");
    for (auto i : {1,2,3,4}) {
        auto hline = houghPoint -> GetLineInImageSpace(i,x1,x2,y1,y2);
        hline -> Draw("samel");
        auto rline = houghPoint -> GetRadialLineInImageSpace(i,5);
        rline -> Draw("samel");
    }
}
