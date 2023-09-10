#include "ejungwooA.h"
#include "LKHoughTransformTracker.cpp"

void drawDefinition()
{
    int nx = 10;
    int ny = 10;
    double x1 = 0;
    double x2 = 10;
    double y1 = 0;
    double y2 = 10;

    auto cvs = ejungwoo::CanvasR("cvs",100,80,2,1);

    for (auto i : {1,2,3,4}) {
        auto hline = houghPoint -> GetLineInImageSpace(i,x1,x2,y1,y2);
        hline -> Draw("samel");
        auto rline = houghPoint -> GetRadialLineInImageSpace(i,0.5);
        rline -> Draw("samel");
    }
}
