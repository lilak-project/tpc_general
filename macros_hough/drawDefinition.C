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

    auto cvs = ejungwoo::Canvas("cvs",1,1);
    auto hist = new TH2D("hist","",nx,x1,x2,ny,y1,y2);
    hist -> Draw();

    auto houghPoint = new LKHoughPointRT(0,0,5,30,5,45);
    //for (auto i : {1,2,3,4}) {
    for (auto i : {1}) {
        auto hline = houghPoint -> GetLineInImageSpace(i,x1,x2,y1,y2);
        hline -> Draw("samel");
        auto rline = houghPoint -> GetRadialLineInImageSpace(i,0.5);
        rline -> Draw("samel");
    }
}
