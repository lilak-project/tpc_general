#include "LKHoughPointRT.h"

ClassImp(LKHoughPointRT);

LKHoughPointRT::LKHoughPointRT()
{
    Clear();
}

LKHoughPointRT::LKHoughPointRT(double xc, double yc, double r1, double t1, double r2, double t2, double w)
{
    Clear();
    SetPoint(xc,yc,r1,t1,r2,t2,w);
}

void LKHoughPointRT::Clear(Option_t *option)
{
    //LKContainer::Clear(option);
    fXTransformCenter = 0;
    fYTransformCenter = 0;
    fRadius0 = 0;
    fRadius1 = 0;
    fRadius2 = 0;
    fTheta0 = 0;
    fTheta1 = 0;
    fTheta2 = 0;
    fWeight = 0;
}

void LKHoughPointRT::Print(Option_t *option) const
{
    e_info << "R/T/Y = " << fRadius0 << "(" << fRadius1 << ", " << fRadius2 << "), " << fTheta0 << "(" << fTheta1 << ", " << fTheta2 << "), " 
        << "y = " << (fRadius0) /TMath::Sin(TMath::DegToRad()*fTheta0) << " + " <<  -TMath::Cos(TMath::DegToRad()*fTheta0) / TMath::Sin(TMath::DegToRad()*fTheta0)  << " * x " << endl;;
}

void LKHoughPointRT::Copy(TObject &object) const
{
    // You should copy data from this container to objCopy
    //LKContainer::Copy(object);
    auto objCopy = (LKHoughPointRT &) object;
    objCopy.SetRadius(fRadius1,fRadius2);
    objCopy.SetTheta(fTheta1,fRadius2);
    objCopy.SetWeight(fWeight);
}

TVector3 LKHoughPointRT::GetCorner(int i) const
{
         if (i==1) { return TVector3(fRadius0, fTheta0, 0); }
    else if (i==2) { return TVector3(fRadius0, fTheta0, 0); }
    else if (i==3) { return TVector3(fRadius0, fTheta0, 0); }
    else if (i==4) { return TVector3(fRadius0, fTheta0, 0); }
    return TVector3(-999,-999,-999);
}

void LKHoughPointRT::IsInside(double r, double t)
{
    // TODO
    return true;
}

TGraph* LKHoughPointRT::GetLineInImageSpace(int i, double x1, double x2, double y1, double y2)
{
    // TODO
    GetImagePoints(i,x1,x2,y1,y2);
    auto graph = new TGraph();
    graph -> SetPoint(0,x1,y1);
    graph -> SetPoint(1,x2,y2);
    return graph;
}

TGraphErrors* LKHoughPointRT::GetBandInImageSpace(double x1, double x2, double y1, double y2)
{
    vector<double> xCrossArray;

    for (auto i : {1,2,3,4}) {
        double x1i = x1;
        double x2i = x2;
        double y1i = y1;
        double y2i = y2;
        GetImagePoints(i,x1i,x2i,y1i,y2i);
        double si = (y2i - y1i) / (x2i - x1i);
        for (auto j : {1,2,3,4}) {
            double x1j = x1;
            double x2j = x2;
            double y1j = y1;
            double y2j = y2;
            GetImagePoints(j,x1j,x2j,y1j,y2j);
            double sj = (y2j - y1j) / (x2j - x1j);

            if ((si-sj)<1.e-10)
                continue;
            else {
                double xCross = (si*x1i - sj*x1j - y2i + y2j) / (si - sj);
                if (xCross>x1 && xCross<x2) {
                    xCrossArray.push_back(xCross);
                }
            }
        }
    }

    xCrossArray.push_back(x1);
    xCrossArray.push_back(x2);
    sort(xCrossArray.begin(), xCrossArray.end());

    vector<double> yCrossMaxArray;
    vector<double> yCrossMinArray;
    for (auto x : xCrossArray) {
        yCrossMinArray.push_back(DBL_MAX);
        yCrossMaxArray.push_back(-DBL_MAX);
    }

    for (auto i : {1,2,3,4}) {
        double x1i = x1;
        double x2i = x2;
        double y1i = y1;
        double y2i = y2;
        GetImagePoints(i,x1i,x2i,y1i,y2i);
        for (auto iCross=0; iCross<xCrossArray.size(); ++iCross) {
            auto xCross = xCrossArray[iCross];
            auto yCross = (xCross - x1i) * (y2i - y1i) / (x2i - x1i) + y1i;
            auto yCrossMin = yCrossMinArray[iCross];
            auto yCrossMax = yCrossMaxArray[iCross];
            if (yCrossMin>yCross) yCrossMinArray[iCross] = yCross;
            if (yCrossMax<yCross) yCrossMaxArray[iCross] = yCross;
        }
    }

    auto graph = new TGraphErrors();
    for (int iCross=0; iCross<xCrossArray.size(); ++iCross) {
        //lk_debug << iCross << endl;
        auto xCross = xCrossArray[iCross];
        auto yCrossMin = yCrossMinArray[iCross];
        graph -> SetPoint(graph->GetN(), xCross, yCrossMin);
    }
    //for (int iCross=0; iCross<xCrossArray.size(); ++iCross) {
    for (int iCross=xCrossArray.size()-1; iCross>=0; --iCross) {
        //lk_debug << iCross << endl;
        auto xCross = xCrossArray[iCross];
        auto yCrossMax = yCrossMaxArray[iCross];
        graph -> SetPoint(graph->GetN(), xCross, yCrossMax);
    }
    auto xCross = xCrossArray[0];
    auto yCrossMin = yCrossMinArray[0];
    graph -> SetPoint(graph->GetN(), xCross, yCrossMin);
    return graph;
}

double LKHoughPointRT::EvalY(int i, double x) const
{
    double radius, theta;
         if (i==0) { radius = fRadius0, theta = fTheta0; }
    else if (i==1) { radius = fRadius1, theta = fTheta1; }
    else if (i==2) { radius = fRadius1, theta = fTheta2; }
    else if (i==3) { radius = fRadius2, theta = fTheta1; }
    else if (i==4) { radius = fRadius2, theta = fTheta2; }
    double y = (radius - (x-fXTransformCenter)*TMath::Cos(TMath::DegToRad()*theta)) / TMath::Sin(TMath::DegToRad()*theta) + fYTransformCenter;

    return y;
}

double LKHoughPointRT::EvalX(int i, double y) const
{
    double radius, theta;
         if (i==0) { radius = fRadius0, theta = fTheta0; }
    else if (i==1) { radius = fRadius1, theta = fTheta1; }
    else if (i==2) { radius = fRadius1, theta = fTheta2; }
    else if (i==3) { radius = fRadius2, theta = fTheta1; }
    else if (i==4) { radius = fRadius2, theta = fTheta2; }
    double x = (radius - (y-fYTransformCenter)*TMath::Sin(TMath::DegToRad()*theta)) / TMath::Cos(TMath::DegToRad()*theta) + fXTransformCenter;

    return x;
}

void LKHoughPointRT::GetImagePoints(int i, double &x1, double &x2, double &y1, double &y2)
{
    if ((fTheta0>45&&fTheta0<135)||(fTheta0<-45&&fTheta0>-135)) {
        double xi = x1;
        double xf = x2;
        double yi = EvalY(i,xi);
        double yf = EvalY(i,xf);
        x1 = xi;
        x2 = xf;
        y1 = yi;
        y2 = yf;
    }
    else {
        double yi = y1;
        double yf = y2;
        double xi = EvalX(i,yi);
        double xf = EvalX(i,yf);
        x1 = xi;
        x2 = xf;
        y1 = yi;
        y2 = yf;
    }
}

LKGeoLine LKHoughPointRT::GetGeoLine(int i, double x1, double x2, double y1, double y2)
{
    LKGeoLine line;
    if ((fTheta0>45&&fTheta0<135)||(fTheta0<-45&&fTheta0>-135)) {
        double xi = x1;
        double xf = x2;
        double yi = EvalY(i,xi);
        double yf = EvalY(i,xf);
        line.SetLine(xi,yi,0,xf,yf,0);
    }
    else {
        double yi = y1;
        double yf = y2;
        double xi = EvalX(i,yi);
        double xf = EvalX(i,yf);
        line.SetLine(xi,yi,0,xf,yf,0);
    }
    return line;
}

double LKHoughPointRT::DistanceToPoint(TVector3 point)
{
    double xi, xf, yi, yf;
    if ((fTheta0>45&&fTheta0<135)||(fTheta0<-45&&fTheta0>-135)) {
        xi = 0;
        if (fXTransformCenter==0) xi = 1;
        xf = fXTransformCenter;
        yi = EvalY(xi);
        yf = EvalY(xf);
    }
    else {
        yi = 0;
        if (fYTransformCenter==0) yi = 1;
        yf = fYTransformCenter;
        xi = EvalX(yi);
        xf = EvalX(yf);
    }
    TVector3 refi = TVector3(xi,yi,0);
    TVector3 ldir = TVector3(xf-xi,yf-yi,0).Unit();
    TVector3 poca = refi + ldir.Dot((point-refi)) * ldir;
    double distance = (point-poca).Mag();
    return distance;
}
