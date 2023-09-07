#ifndef LKIMAGEPOINT_HH
#define LKIMAGEPOINT_HH

#include "TClonesArray.h"
//#include "LKContainer.h"
#include "TObject.h"
#include "LKLogger.h"

//class LKImagePoint : public LKContainer
class LKImagePoint : public TObject
{
    public:
        LKImagePoint();
        LKImagePoint(double x1, double y1, double x2, double y2, double w=0);
        virtual ~LKImagePoint() { ; }

        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;
        void Copy(TObject &object) const;

        double GetX0() const  { return fX0; }
        double GetX1() const  { return fX1; }
        double GetX2() const  { return fX2; }
        double GetY0() const  { return fY0; }
        double GetY1() const  { return fY1; }
        double GetY2() const  { return fY2; }
        double GetWeight() const  { return fWeight; }

        TVector3 GetCenter() const;
        TVector3 GetCorner(int i) const;

        void SetPoint(double x1, double y1, double x2, double y2, double w=0) { SetX(x1,x2); SetY(y1,y2); fWeight = w; }
        void SetX(double x1, double x2) { fX0 = .5*(x1+x2); fX1 = x1; fX2 = x2; }
        void SetY(double y1, double y2) { fY0 = .5*(y1+y2); fY1 = y1; fY2 = y2; }
        void SetWeight(double weight) { fWeight = weight; }

        double GetCenterX() const;
        double GetCenterY() const;
        bool IsInside(double x, double y);
        double EvalR(int i, double theta, double xt, double yt);

        double       fX0;
        double       fX1;
        double       fX2;
        double       fY0;
        double       fY1;
        double       fY2;
        double       fWeight;

    ClassDef(LKImagePoint,1);
};

#endif
