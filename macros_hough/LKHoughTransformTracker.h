#ifndef LKHOUGHTRANSFORMTRACKER_HH
#define LKHOUGHTRANSFORMTRACKER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "LKGeoLine.h"

#include "LKImagePoint.h"
#include "LKHoughPointRT.h"
#include "LKImagePoint.cpp"
#include "LKHoughPointRT.cpp"

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
        int GetNumImagePoints() const  { return fNumImagePoints; }
        int GetNumHoughPoints() const  { return fNumBinsHoughSpace[0]*fNumBinsHoughSpace[1]; }

        void SetTransformCenter(TVector3 transformCenter) { fTransformCenter = transformCenter; }
        void SetImageSpaceRange(int nx, double x2, double x1, int ny, double y1, double y2);
        void SetHoughSpaceBins(int nr, int nt);
        void SetHoughSpaceRange(int nr, double r2, double r1, int nt, double t1, double t2);
        void AddImagePoint(double x, double xError, double y, double yError, double weight);
        void AddImagePointBox(double x1, double y1, double x2, double y2, double weight);
        void SetImageData(double** imageData);// { fImageData = imageData; }

        void SetCorrelateBoxLine()  { fCorrelateType = kCorrelateBoxLine; }
        void SetCorrelateBoxBand()  { fCorrelateType = kCorrelateBoxBand; }
        void SetCorrelateDistance() { fCorrelateType = kCorrelateDistance; }

        void SetCutNumTrackHits(double value) { fCutNumTrackHits = value; }

        LKImagePoint* GetImagePoint(int i);
        LKHoughPointRT* GetHoughPoint(int i);
        LKHoughPointRT* GetHoughPoint(int ir, int it);

        void Transform();
        double WeightingDistanceLinear(double distance, double imageWeight);
        double WeightingDistanceInvProp(double distance, double imageWeight);
        double CorrelateBoxLine(LKImagePoint* imagePoint, LKHoughPointRT* houghPoint, double imageWeight);
        double CorrelateBoxBand(LKImagePoint* imagePoint, LKHoughPointRT* houghPoint, double imageWeight);
        double CorrelateDistance(LKImagePoint* imagePoint, LKHoughPointRT* houghPoint, double imageWeight);

        LKHoughPointRT* GetNextMaximumHoughPoint();
        LKLinearTrack* FitTrackWithHoughPoint(LKHoughPointRT* houghPoint, double distCut=0);
        void CleanLastHoughPoint(double rWidth=-1, double tWidth=-1);
        void CleanAndRetransform();

        void Analyze(int i);

        TH2D* GetHistImageSpace(TString name="", TString title="");
        TH2D* GetHistHoughSpace(TString name="", TString title="");
        void DrawAllHoughLines();

        double EvalFromHoughParameter(double x, double radius, double theta);

    private:

        TVector3     fTransformCenter;
        bool         fInitializedImageData = false;
        bool         fInitializedHoughData = false;
        int          fNumBinsImageSpace[2] = {10,10};
        int          fNumBinsHoughSpace[2] = {10,10};
        double       fBinSizeImageSpace[2] = {0};
        double       fBinSizeHoughSpace[2] = {0};
        double       fBinSizeMaxImageSpace = 0;
        double       fBinSizeMaxHoughSpace = 0;
        double       fRangeImageSpace[2][2] = {{0}};
        double       fRangeHoughSpace[2][2] = {{0}};
        double**     fHoughData;
        double**     fImageData;
        int          fIdxSelectedR = -1;
        int          fIdxSelectedT = -1;

        bool            fUsingImagePointArray = true;
        int             fNumImagePoints = 0;
        TClonesArray*   fImagePointArray = nullptr;
        LKImagePoint*   fImagePoint = nullptr;
        LKHoughPointRT* fHoughPoint = nullptr;

        int           fNumLinearTracks = 0;
        TClonesArray* fTrackArray = nullptr;

        LKODRFitter* fLineFitter = nullptr;
        int          fCutNumTrackHits = 5;

        const int    kCorrelateBoxLine = 0;
        const int    kCorrelateBoxBand = 1;
        const int    kCorrelateDistance = 2;
        int          fCorrelateType = kCorrelateDistance;

    ClassDef(LKHoughTransformTracker,1);
};

#endif
