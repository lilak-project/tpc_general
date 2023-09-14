#ifndef LKHOUGHPOINTRT_HPP
#define LKHOUGHPOINTRT_HPP

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
    e_info << "R = " << fRadius0 << "(" << fRadius1 << ", " << fRadius2 << "), T = " << fTheta0 << "(" << fTheta1 << ", " << fTheta2 << "), " 
        << "equation: y = " <<  -TMath::Cos(TMath::DegToRad()*fTheta0) / TMath::Sin(TMath::DegToRad()*fTheta0)  << " * x + " << (fRadius0) /TMath::Sin(TMath::DegToRad()*fTheta0) << endl;;
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

TVector3 LKHoughPointRT::GetCorner(int iHoughCorner) const
{
         if (iHoughCorner==0) { return TVector3(fRadius0, fTheta0, 0); }
    else if (iHoughCorner==1) { return TVector3(fRadius1, fTheta1, 0); }
    else if (iHoughCorner==2) { return TVector3(fRadius1, fTheta2, 0); }
    else if (iHoughCorner==3) { return TVector3(fRadius2, fTheta1, 0); }
    else if (iHoughCorner==4) { return TVector3(fRadius2, fTheta2, 0); }
    return TVector3(-999,-999,-999);
}

void LKHoughPointRT::IsInside(double r, double t)
{
    // TODO
    return true;
}

TGraph* LKHoughPointRT::GetLineInImageSpace(int iHoughCorner, double x1, double x2, double y1, double y2)
{
    // TODO
    GetImagePoints(iHoughCorner,x1,x2,y1,y2);
    auto graph = new TGraph();
    graph -> SetPoint(0,x1,y1);
    graph -> SetPoint(1,x2,y2);
    return graph;
}

TGraph* LKHoughPointRT::GetRadialLineInImageSpace(int iHoughCorner, double angleSize)
{
    double xi = 0;
    double xf = 0;
    double yi = 0;
    double yf = 0;
    GetImagePoints(iHoughCorner,xi,xf,yi,yf);
    TVector3 pointOnLine(xf-xi,yf-yi,0);

    TVector3 poca = GetPOCA(iHoughCorner);
    TVector3 dirCenter = (TVector3(fXTransformCenter,fYTransformCenter,0) - poca).Unit();
    TVector3 dirLine = pointOnLine.Unit();

    if (angleSize==0)
        angleSize = 0.1*(TVector3(fXTransformCenter,fYTransformCenter,0) - poca).Mag();

    TVector3 point1 = poca   + dirLine   * angleSize;
    TVector3 point2 = point1 + dirCenter * angleSize;
    TVector3 point3 = point2 - dirLine   * angleSize;

    TVector3 extra1 = point1 - dirLine   * 0.5*angleSize;
    TVector3 extra2 = point2;
    TVector3 extra3 = point1;
    TVector3 extra4 = point2 - dirLine * 0.5*angleSize;
    TVector3 extra5 = poca;

    auto graph = new TGraph();
    graph -> SetLineColor(kBlue);
    graph -> SetPoint(0,fXTransformCenter,fYTransformCenter);
    graph -> SetPoint(1,poca.X(),poca.Y());
    graph -> SetPoint(2,point1.X(),point1.Y());
    graph -> SetPoint(3,point2.X(),point2.Y());
    graph -> SetPoint(4,point3.X(),point3.Y());

    if (iHoughCorner>=1) graph -> SetPoint(5,extra1.X(),extra1.Y());
    if (iHoughCorner>=2) graph -> SetPoint(6,extra2.X(),extra2.Y());
    if (iHoughCorner>=2) graph -> SetPoint(7,extra3.X(),extra3.Y());
    if (iHoughCorner>=3) graph -> SetPoint(8,extra4.X(),extra4.Y());
    if (iHoughCorner>=4) graph -> SetPoint(9,extra5.X(),extra5.Y());
    return graph;
}

TGraph* LKHoughPointRT::GetBandInImageSpace(double x1, double x2, double y1, double y2)
{
    vector<double> xCrossArray;

    double combination[12][2] = {
        {1,2}, {1,4}, {2,3}, {3,4},
        {1,5}, {2,5}, {3,5}, {4,5},
        {1,6}, {2,6}, {3,6}, {4,6},
    };
    for (auto iCombination=0; iCombination<12; ++iCombination)
    {
        auto iHoughCorner = combination[iCombination][0];
        auto jHoughCorner = combination[iCombination][1];
        if (iHoughCorner==jHoughCorner)
            continue;

        double x1i = x1;
        double x2i = x2;
        double y1i = y1;
        double y2i = y2;
        GetImagePoints(iHoughCorner,x1i,x2i,y1i,y2i);
        double si = (y2i - y1i) / (x2i - x1i); // slope
        double bi = y1i - si * x1i; // interception

        double x1j = x1;
        double x2j = x2;
        double y1j = y1;
        double y2j = y2;
        if (jHoughCorner==5) y2j = y1;
        else if (jHoughCorner==6) y1j = y2;
        else GetImagePoints(jHoughCorner,x1j,x2j,y1j,y2j);
        double sj = (y2j - y1j) / (x2j - x1j); // slope
        double bj = y1j - sj * x1j; // interception

        if (abs(si-sj)<1.e-10)
            continue;
        else {
            double xCross = (bj - bi) / (si - sj);
            if (xCross>x1 && xCross<x2) {
                xCrossArray.push_back(xCross);
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

    for (auto iHoughCorner : {1,2,3,4}) {
        double x1i = x1;
        double x2i = x2;
        double y1i = y1;
        double y2i = y2;
        GetImagePoints(iHoughCorner,x1i,x2i,y1i,y2i);
        for (auto iCross=0; iCross<xCrossArray.size(); ++iCross) {
            auto xCross = xCrossArray[iCross];
            auto yCross = (xCross - x1i) * (y2i - y1i) / (x2i - x1i) + y1i;
            auto yCrossMin = yCrossMinArray[iCross];
            auto yCrossMax = yCrossMaxArray[iCross];
            if (yCrossMin>yCross) yCrossMinArray[iCross] = yCross;
            if (yCrossMax<yCross) yCrossMaxArray[iCross] = yCross;
        }
    }

    auto graph = new TGraph();
    for (int iCross=0; iCross<xCrossArray.size(); ++iCross) {
        auto xCross = xCrossArray[iCross];
        auto yCross = yCrossMinArray[iCross];
        if (yCross<y1) yCross = y1;
        if (yCross>y2) yCross = y2;
        graph -> SetPoint(graph->GetN(), xCross, yCross);
    }
    for (int iCross=xCrossArray.size()-1; iCross>=0; --iCross) {
        auto xCross = xCrossArray[iCross];
        auto yCross = yCrossMaxArray[iCross];
        if (yCross<y1) yCross = y1;
        if (yCross>y2) yCross = y2;
        graph -> SetPoint(graph->GetN(), xCross, yCross);
    }
    auto xCross = xCrossArray[0];
    auto yCrossMin = yCrossMinArray[0];
    graph -> SetPoint(graph->GetN(), xCross, yCrossMin);
    return graph;
}

TGraph* LKHoughPointRT::GetRangeGraphInHoughSpace(bool drawYX)
{
    auto graph = new TGraph();
    graph -> SetLineColor(kRed);
    graph -> SetLineWidth(3);
    for (auto iHoughCorner : {1,2,4,3,1}) {
        auto pos = GetCorner(iHoughCorner);
        if (drawYX)
            graph -> SetPoint(graph->GetN(),pos.Y(),pos.X());
        else
            graph -> SetPoint(graph->GetN(),pos.X(),pos.Y());
    }
    return graph;
}

TVector3 LKHoughPointRT::GetPOCA(int iHoughCorner)
{
    double radius, theta;
         if (iHoughCorner==0) { radius = fRadius0, theta = fTheta0; }
    else if (iHoughCorner==1) { radius = fRadius1, theta = fTheta1; }
    else if (iHoughCorner==2) { radius = fRadius1, theta = fTheta2; }
    else if (iHoughCorner==3) { radius = fRadius2, theta = fTheta1; }
    else if (iHoughCorner==4) { radius = fRadius2, theta = fTheta2; }
    return TVector3(radius*TMath::Cos(TMath::DegToRad()*theta)+fXTransformCenter,radius*TMath::Sin(TMath::DegToRad()*theta)+fYTransformCenter,0);
}

double LKHoughPointRT::EvalY(int iHoughCorner, double x) const
{
    double radius, theta;
         if (iHoughCorner==0) { radius = fRadius0, theta = fTheta0; }
    else if (iHoughCorner==1) { radius = fRadius1, theta = fTheta1; }
    else if (iHoughCorner==2) { radius = fRadius1, theta = fTheta2; }
    else if (iHoughCorner==3) { radius = fRadius2, theta = fTheta1; }
    else if (iHoughCorner==4) { radius = fRadius2, theta = fTheta2; }
    double y = (radius - (x-fXTransformCenter)*TMath::Cos(TMath::DegToRad()*theta)) / TMath::Sin(TMath::DegToRad()*theta) + fYTransformCenter;
    //lk_debug << radius << " " << theta << " >> " << x << " " << y << endl;

    return y;
}

double LKHoughPointRT::EvalX(int iHoughCorner, double y) const
{
    double radius, theta;
         if (iHoughCorner==0) { radius = fRadius0, theta = fTheta0; }
    else if (iHoughCorner==1) { radius = fRadius1, theta = fTheta1; }
    else if (iHoughCorner==2) { radius = fRadius1, theta = fTheta2; }
    else if (iHoughCorner==3) { radius = fRadius2, theta = fTheta1; }
    else if (iHoughCorner==4) { radius = fRadius2, theta = fTheta2; }
    double x = (radius - (y-fYTransformCenter)*TMath::Sin(TMath::DegToRad()*theta)) / TMath::Cos(TMath::DegToRad()*theta) + fXTransformCenter;
    //lk_debug << radius << " " << theta << " >> " << x << " " << y << endl;

    return x;
}

void LKHoughPointRT::GetImagePoints(int iHoughCorner, double &x1, double &x2, double &y1, double &y2)
{
    if (x1==0&&x2==0&&y1==0&&y2==0) {
        x1 = ((fXTransformCenter==0) ? 1. : 0.);
        y1 = ((fYTransformCenter==0) ? 1. : 0.);
        x2 = fXTransformCenter;
        y2 = fYTransformCenter;
    }

    if ((fTheta0>45&&fTheta0<135)||(fTheta0<-45&&fTheta0>-135)) {
        double xi = x1;
        double xf = x2;
        double yi = EvalY(iHoughCorner,xi);
        double yf = EvalY(iHoughCorner,xf);
        x1 = xi;
        x2 = xf;
        y1 = yi;
        y2 = yf;
    }
    else {
        double yi = y1;
        double yf = y2;
        double xi = EvalX(iHoughCorner,yi);
        double xf = EvalX(iHoughCorner,yf);
        x1 = xi;
        x2 = xf;
        y1 = yi;
        y2 = yf;
    }
}

LKGeoLine LKHoughPointRT::GetGeoLine(int iHoughCorner, double x1, double x2, double y1, double y2)
{
    GetImagePoints(iHoughCorner,x1,x2,y1,y2);
    LKGeoLine line(x1,y1,0,x2,y2,0);
    return line;
}

double LKHoughPointRT::DistanceToPoint(TVector3 point)
{
    double xi = 0;
    double xf = 0;
    double yi = 0;
    double yf = 0;
    GetImagePoints(0,xi,xf,yi,yf);

    TVector3 refi = TVector3(xi,yi,0);
    TVector3 ldir = TVector3(xf-xi,yf-yi,0).Unit();
    TVector3 poca = refi + ldir.Dot((point-refi)) * ldir;
    double distance = (point-poca).Mag();
    return distance;
}

double LKHoughPointRT::DistanceToPoint(int iCorner, TVector3 point)
{
    double xi = 0;
    double xf = 0;
    double yi = 0;
    double yf = 0;
    GetImagePoints(iCorner,xi,xf,yi,yf);

    TVector3 refi = TVector3(xi,yi,0);
    TVector3 ldir = TVector3(xf-xi,yf-yi,0).Unit();
    TVector3 poca = refi + ldir.Dot((point-refi)) * ldir;
    double distance = (point-poca).Mag();
    return distance;
}

double LKHoughPointRT::CorrelateBoxLine(LKImagePoint* imagePoint)
{
    bool existAboveLine = false;
    bool existBelowLine = false;
    for (auto iImageCorner : {1,2,3,4})
    {
        auto point = imagePoint -> GetCorner(iImageCorner);
        auto y = EvalY(point.X());
        if (y<point.Y()) existAboveLine = true;
        if (y>point.Y()) existBelowLine = true;
    }
    if (existAboveLine&&existBelowLine) {
        auto distance = DistanceToPoint(imagePoint->GetCenter());
        return distance;
    }

    return -1;
}

double LKHoughPointRT::CorrelateBoxBand(LKImagePoint* imagePoint)
{
    auto weight = 0;
    int includedBelowOrAbove[4] = {0};

    for (auto iHoughCorner : {1,2,3,4})
    {
        bool existAboveLine = false;
        bool existBelowLine = false;

        for (auto iImageCorner : {1,2,3,4})
        {
            auto point = imagePoint -> GetCorner(iImageCorner);
            auto yHoughLine = EvalY(iHoughCorner, point.X());
            if (yHoughLine<=point.Y()) existAboveLine = true;
            if (yHoughLine> point.Y()) existBelowLine = true;
            //lk_debug << "yHoughLine(" << iHoughCorner << ") = " << yHoughLine << ", yImagePoint(" << iImageCorner << ") = " << point.Y() << endl;
        }

        if (existAboveLine&&existBelowLine)
            includedBelowOrAbove[iHoughCorner-1] = 0;
        else if (existAboveLine)
            includedBelowOrAbove[iHoughCorner-1] = 1;
        else
            includedBelowOrAbove[iHoughCorner-1] = 2;
    }

    bool existAbove = false;
    bool existBelow = false;
    bool crossOver = false;
    for (auto iHoughCorner : {1,2,3,4}) {
             if (includedBelowOrAbove[iHoughCorner-1]==0) { crossOver = true;  }
        else if (includedBelowOrAbove[iHoughCorner-1]==1) { existAbove = true; }
        else if (includedBelowOrAbove[iHoughCorner-1]==2) { existBelow = true; }
        //     if (includedBelowOrAbove[iHoughCorner-1]==0) lk_debug << "hough line-" << iHoughCorner << " cross over " << endl;
        //else if (includedBelowOrAbove[iHoughCorner-1]==1) lk_debug << "hough line-" << iHoughCorner << " is above line " << endl;
        //else if (includedBelowOrAbove[iHoughCorner-1]==2) lk_debug << "hough line-" << iHoughCorner << " is below line " << endl;
    }
    if (existAbove&&existBelow)
        crossOver = true;

    if (crossOver) {
        auto distance = DistanceToPoint(imagePoint->GetCenter());
        return distance;
    }

    return -1;
}

double LKHoughPointRT::DistanceToImagePoint(LKImagePoint* imagePoint)
{
    auto distance = DistanceToPoint(imagePoint->GetCenter());
    return distance;
}

double LKHoughPointRT::DistanceToImagePoint(int iHoughCorner, LKImagePoint* imagePoint)
{
    auto distance = DistanceToPoint(iHoughCorner, imagePoint->GetCenter());
    return distance;
}

#endif
