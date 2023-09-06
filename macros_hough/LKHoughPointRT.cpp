#include "LKHoughPointRT.h"

ClassImp(LKHoughPointRT);

LKHoughPointRT::LKHoughPointRT()
{
    Clear();
}

LKHoughPointRT::LKHoughPointRT(double xc, double yc, double r1, double t1, double r2, double t2, double w)
{
    Clear();
    SetTransformCenter(xc,yc);
    SetRadius(r1,r2);
    SetTheta(t1,t2);
    fWeight = w;
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

TGraph* LKHoughPointRT::GetLineInImageSpace(int i, double x1, double x2)
{
    // TODO
    return new TGraph();
}

TGraphErrors* LKHoughPointRT::GetBandInImageSpace(double x1, double x2)
{
    // TODO
    return new TGraphErrors();
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

LKGeoLine LKHoughPointRT::GetGeoLine(int i, double x1, double x2, double y1, double y2)
{
    LKGeoLine line;
    //if (1) {
    if ((fTheta0>45&&fTheta0<135)||(fTheta0<-45&&fTheta0>-135)) {
        double xi = x1;
        double xf = x2;
        double yi = EvalY(xi);// - fXTransformCenter);
        double yf = EvalY(xf);// - fXTransformCenter);
        //yi = yi + fYTransformCenter;
        //yf = yf + fYTransformCenter;
        line.SetLine(xi,yi,0,xf,yf,0);
    }
    else {
        double yi = y1;
        double yf = y2;
        double xi = EvalX(yi);// - fYTransformCenter);
        double xf = EvalX(yf);// - fYTransformCenter);
        //xi = xi + fXTransformCenter;
        //xf = xf + fXTransformCenter;
        line.SetLine(xi,yi,0,xf,yf,0);
    }
    return line;
}

