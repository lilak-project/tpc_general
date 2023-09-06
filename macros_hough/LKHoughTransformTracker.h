#ifndef LKHOUGHTRANSFORMTRACKER_HH
#define LKHOUGHTRANSFORMTRACKER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "LKGeoLine.h"

#include "LKImagePoint.cpp"
#include "LKHoughPointRT.cpp"
#include "LKImagePoint.h"
#include "LKHoughPointRT.h"

class LKHoughTransformTracker : public TObject
{
    public:
        LKHoughTransformTracker();
        virtual ~LKHoughTransformTracker() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        TVector3 GetTransformCenter() const  { return fTransformCenter; }
        int GetNumBinsImageSpace(int ixy) const  { return fNumBinsImageSpace[ixy]; }
        int GetNumBinsHoughSpace(int ixy) const  { return fNumBinsHoughSpace[ixy]; }
        double GetRangeImageSpace(int ixy, int i) const  { return fRangeImageSpace[ixy][i]; }
        double GetRangeHoughSpace(int ixy, int i) const  { return fRangeHoughSpace[ixy][i]; }
        double** GetHoughData() const  { return fHoughData; }
        double** GetImageData() const  { return fImageData; }
        bool GetImageIsPoints() const  { return fImageIsPoints; }
        int GetNumImagePoints() const  { return fNumImagePoints; }
        int GetNumHoughPoints() const  { return fNumBinsHoughSpace[0]*fNumBinsHoughSpace[1]; }

        //void SetNumBinsImageSpace(int ixy, int numBinsImageSpace) { fNumBinsImageSpace[ixy] = numBinsImageSpace; }
        //void SetNumBinsHoughSpace(int ixy, int numBinsHoughSpace) { fNumBinsHoughSpace[ixy] = numBinsHoughSpace; }
        //void SetRangeImageSpace(int ixy, int i, double rangeImageSpace) { fRangeImageSpace[ixy][i] = rangeImageSpace; }
        //void SetRangeHoughSpace(int ixy, int i, double rangeHoughSpace) { fRangeHoughSpace[ixy][i] = rangeHoughSpace; }
        //void SetHoughData(double** houghData);// { fHoughData = houghData; }
        void SetTransformCenter(TVector3 transformCenter) { fTransformCenter = transformCenter; }
        void SetImageSpaceRange(int nx, double x2, double x1, int ny, double y1, double y2);
        void SetHoughSpaceBins(int nr, int nt);
        void SetHoughSpaceRange(int nr, double r2, double r1, int nt, double t1, double t2);
        void AddImagePoint(double x, double xError, double y, double yError, double weight);
        void AddImagePointBox(double x1, double y1, double x2, double y2, double weight);
        void SetImageData(double** imageData);// { fImageData = imageData; }

        LKImagePoint GetImagePoint(int i);
        //LKImagePoint FindImagePoint(double xImageSpace, double yImageSpace);
        LKHoughPointRT GetHoughPoint(int i);
        LKHoughPointRT GetHoughPoint(int iriXHoughSpace, int it);
        LKHoughPointRT FindHoughPoint(double xHoughSpace, double yHoughSpace);

        void Transform();
        double CorrelateImageToHough(LKImagePoint imagePoint, LKHoughPointRT houghPoint);
        LKHoughPointRT FindNextMaximumHoughPoint();
        void ClearLastMaximumHoughPoint(double rWidth=0, double tWidth=0);

        void Analyze(int i);

        TH2D* GetHistImageSpace(TString name="");
        TH2D* GetHistHoughSpace(TString name="");
        void DrawAllHoughLines();

        double EvalFromHoughParameter(double x, double radius, double theta);

    private:

        TVector3     fTransformCenter;
        int          fNumBinsImageSpace[2] = {10,10};
        int          fNumBinsHoughSpace[2] = {10,10};
        int          fBinSizeImageSpace[2] = {0};
        double       fBinSizeHoughSpace[2] = {0};
        double       fRangeImageSpace[2][2] = {{0}};
        double       fRangeHoughSpace[2][2] = {{0}};
        double**     fHoughData;
        double**     fImageData;
        bool         fImageIsPoints = true;
        int          fNumImagePoints;
        vector<LKImagePoint> fImagePointArray;
        int          fIdxSelectedR = -1;
        int          fIdxSelectedT = -1;

    ClassDef(LKHoughTransformTracker,1);
};

#endif
