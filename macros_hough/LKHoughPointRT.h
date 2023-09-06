#ifndef LKHOUGHPOINTRT_HH
#define LKHOUGHPOINTRT_HH

#include "TClonesArray.h"
#include "TObject.h"
//#include "LKContainer.h"
#include "LKLogger.h"
#include "LKGeoLine.h"

//class LKHoughPointRT : public LKContainer
class LKHoughPointRT : public TObject
{
    public:
        LKHoughPointRT();
        LKHoughPointRT(double xc, double yc, double r1, double t1, double r2, double t2, double w=0);
        virtual ~LKHoughPointRT() { ; }

        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;
        void Copy(TObject &object) const;

        double GetR0() const  { return fRadius0; }
        double GetR1() const  { return fRadius1; }
        double GetR2() const  { return fRadius2; }
        double GetT0() const  { return fTheta0; }
        double GetT1() const  { return fTheta1; }
        double GetT2() const  { return fTheta2; }
        double GetWeight() const  { return fWeight; }
        TVector3 GetCorner(int i) const;

        void SetTransformCenter(double xc, double yc) { fXTransformCenter = xc; fYTransformCenter = yc; }
        void SetRadius(double r1, double r2) { fRadius0 = .5*(r1+r2); fRadius1 = r1; fRadius2 = r2; }
        void SetTheta(double t1, double t2) { fTheta0 = .5*(t1+t2); fTheta1 = t1; fTheta2 = t2; }
        void SetWeight(double weight) { fWeight = weight; }

        double GetCenterR() const;
        double GetCenterT() const;
        void IsInside(double r, double t);
        TGraph* GetLineInImageSpace(int i, double x1, double x2, double y1, double y2);
        TGraphErrors* GetBandInImageSpace(double x1, double x2, double y1, double y2);

        double DistanceToPoint(TVector3 point);

        double EvalX(double y) const { return EvalX(0,y); }
        double EvalX(int i, double y) const;
        double EvalY(double x) const { return EvalY(0,x); }
        double EvalY(int i, double x) const;
        void GetImagePoints(int i, double &x1, double &x2, double &y1, double &y2);
        LKGeoLine GetGeoLine(int i, double x1, double x2, double y1, double y2);

        double       fXTransformCenter;
        double       fYTransformCenter;

        double       fRadius0;
        double       fRadius1;
        double       fRadius2;
        double       fTheta0;
        double       fTheta1;
        double       fTheta2;
        double       fWeight;

    ClassDef(LKHoughPointRT,1);
};

#endif
