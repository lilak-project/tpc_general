#ifndef LKIMAGEPOINT_CPP
#define LKIMAGEPOINT_CPP

#include "LKImagePoint.h"

ClassImp(LKImagePoint);

LKImagePoint::LKImagePoint()
{
    Clear();
}

LKImagePoint::LKImagePoint(double x1, double y1, double x2, double y2, double w)
{
    Clear();
    SetPoint(x1,y1,x2,y2,w);
}

void LKImagePoint::Clear(Option_t *option)
{
    //LKContainer::Clear(option);
    fX0 = 0;
    fX1 = 0;
    fX2 = 0;
    fY0 = 0;
    fY1 = 0;
    fY2 = 0;
    fWeight = 0;
}

void LKImagePoint::Print(Option_t *option) const
{
    // You will probability need to modify here
    e_info << "LKImagePoint" << std::endl;
    e_info << "fX1 : " << fX1 << std::endl;
    e_info << "fX2 : " << fX2 << std::endl;
    e_info << "fY1 : " << fY1 << std::endl;
    e_info << "fY2 : " << fY2 << std::endl;
    e_info << "fWeight : " << fWeight << std::endl;
}

void LKImagePoint::Copy(TObject &object) const
{
    // You should copy data from this container to objCopy
    //LKContainer::Copy(object);
    auto objCopy = (LKImagePoint &) object;
    objCopy.SetX(fX1,fX2);
    objCopy.SetY(fY1,fY2);
    objCopy.SetWeight(fWeight);
}

TVector3 LKImagePoint::GetCorner(int i) const
{
         if (i==0) { return TVector3(fX0, fY0, 0); }
    else if (i==1) { return TVector3(fX1, fY1, 0); }
    else if (i==2) { return TVector3(fX1, fY2, 0); }
    else if (i==3) { return TVector3(fX2, fY1, 0); }
    else if (i==4) { return TVector3(fX2, fY2, 0); }
    return TVector3(-999,-999,-999);
}

TVector3 LKImagePoint::GetCenter() const
{
    return TVector3(fX0,fY0,0);
}

double LKImagePoint::GetCenterX() const
{
    return fX0;
}

double LKImagePoint::GetCenterY() const
{
    return fY0;
}

bool LKImagePoint::IsInside(double x, double y)
{
    // TODO
    return false;
}

double LKImagePoint::EvalR(int i, double theta, double xt, double yt)
{
    double radius = -1;
    //     if (i==0) radius = ((fX0-xt)*TMath::Cos(TMath::DegToRad()*theta)) + ((fY0-yt)*TMath::Sin(TMath::DegToRad()*theta));
    //else if (i==1) radius = ((fX1-xt)*TMath::Cos(TMath::DegToRad()*theta)) + ((fY1-yt)*TMath::Sin(TMath::DegToRad()*theta));
    //else if (i==2) radius = ((fX1-xt)*TMath::Cos(TMath::DegToRad()*theta)) + ((fY2-yt)*TMath::Sin(TMath::DegToRad()*theta));
    //else if (i==3) radius = ((fX2-xt)*TMath::Cos(TMath::DegToRad()*theta)) + ((fY1-yt)*TMath::Sin(TMath::DegToRad()*theta));
    //else if (i==4) radius = ((fX2-xt)*TMath::Cos(TMath::DegToRad()*theta)) + ((fY2-yt)*TMath::Sin(TMath::DegToRad()*theta));
         if (i==0) radius = abs((fX0-xt)*TMath::Cos(TMath::DegToRad()*theta)) + abs((fY0-yt)*TMath::Sin(TMath::DegToRad()*theta));
    else if (i==1) radius = abs((fX1-xt)*TMath::Cos(TMath::DegToRad()*theta)) + abs((fY1-yt)*TMath::Sin(TMath::DegToRad()*theta));
    else if (i==2) radius = abs((fX1-xt)*TMath::Cos(TMath::DegToRad()*theta)) + abs((fY2-yt)*TMath::Sin(TMath::DegToRad()*theta));
    else if (i==3) radius = abs((fX2-xt)*TMath::Cos(TMath::DegToRad()*theta)) + abs((fY1-yt)*TMath::Sin(TMath::DegToRad()*theta));
    else if (i==4) radius = abs((fX2-xt)*TMath::Cos(TMath::DegToRad()*theta)) + abs((fY2-yt)*TMath::Sin(TMath::DegToRad()*theta));
    return radius;
}

#endif
