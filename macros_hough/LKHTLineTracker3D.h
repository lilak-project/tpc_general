#ifndef LKHOUGHTRANSFORMTRACKER_HH
#define LKHOUGHTRANSFORMTRACKER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "LKGeoLine.h"
#include "TGraphErrors.h"

#include "LKImagePoint.h"
#include "LKParamPointRT.h"
#include "LKImagePoint.cpp"
#include "LKParamPointRT.cpp"
#include "LKHoughTransformTracker.cpp"

#include "LKHoughWeightingFunction.h"

/**
 * 3D straight line tracker using hough transform
 */
class LKHTLineTracker3D : public TObject
{
    public:
        LKHTLineTracker3D();
        virtual ~LKHTLineTracker3D() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;
        void Reset();

        //TVector3 GetTransformCenter() const  { return fTransformCenter; }
        //int GetNumBinsImageSpace(int ixy) const  { return fNumBinsImageSpace[ixy]; }
        //int GetNumBinsParamSpace(int ixy) const  { return fNumBinsParamSpace[ixy]; }
        //double GetRangeImageSpace(int ixy, int i) const  { return fRangeImageSpace[ixy][i]; }
        //double GetRangeParamSpace(int ixy, int i) const  { return fRangeParamSpace[ixy][i]; }
        //double** GetParamData() const  { return fParamData; }
        //double** GetImageData() const  { return fImageData; }
        //int GetNumImagePoints() const  { return fNumImagePoints; }
        //int GetNumParamPoints() const  { return fNumBinsParamSpace[0]*fNumBinsParamSpace[1]; }
        //double GetMaxWeightingDistance(double distance) const { return fMaxWeightingDistance; }

        void SetTransformCenter(double x, double y, double z) { fTransformCenter = TVector3(x,y,z); }
        void SetImageSpaceRange(int nx, double x1, double x2, int ny, double y1, double y2, int nz, double z1, double z2)
        void SetParamSpaceBins(int ny, int nz, int nt, int np);
        void SetParamSpaceRange(int ny, double y1, double y2, int nz, double z1, double z2, int nt, double t1, double t2, int np, double p1, double p2);

        void AddImagePoint(double x, double xError, double y, double yError, double z, double zError, double weight);
        void AddImagePointBox(double x1, double y1, double x2, double y2, double z1, double z2, double weight);

        TString GetCorrelatorName();
        bool IsCorrelatePointBand() { if (fCorrelateType==kCorrelatePointBand) return true; return false; }
        bool IsCorrelateBoxLine()   { if (fCorrelateType==kCorrelateBoxLine)   return true; return false; }
        bool IsCorrelateBoxBand()   { if (fCorrelateType==kCorrelateBoxBand)   return true; return false; }
        bool IsCorrelateBoxRBand()  { if (fCorrelateType==kCorrelateBoxRBand)  return true; return false; }
        bool IsCorrelateDistance()  { if (fCorrelateType==kCorrelateDistance)  return true; return false; }

        void SetCorrelatePointBand();
        void SetCorrelateBoxLine();
        void SetCorrelateBoxBand();
        void SetCorrelateBoxRBand();
        void SetCorrelateDistance();
        void SetMaxWeightingDistance(double distance) { fMaxWeightingDistance = distance; }
        void SetCutNumTrackHits(double value) { fCutNumTrackHits = value; }

        void SetWFConst()   { fWeightingFunction = new LKHoughWFConst(); }
        void SetWFLinear()  { fWeightingFunction = new LKHoughWFLinear(); }
        void SetWFInverse() { fWeightingFunction = new LKHoughWFInverse(); }
        void SetWeightingFunction(LKHoughWeightingFunction* wf) { fWeightingFunction = wf; }

        LKImagePoint* GetImagePoint(int i);
        LKImagePoint* PopImagePoint(int i);
        LKParamPointRT* GetParamPoint(int iy, int iz, int it, int ip);

        void Transform();
        LKParamPointRT* FindNextMaximumParamPoint();
        //LKParamPointRT* FindNextMaximumParamPoint2();
        //LKLinearTrack* FitTrackWithParamPoint(LKParamPointRT* paramPoint, double weightCut=-1);
        void CleanLastParamPoint(double yWidth=-1, double zWidth=-1, double tWidth=-1, double pWidth=-1);
        LKParamPointRT* ReinitializeFromLastParamPoint();
        void RetransformFromLastParamPoint();

        //TGraphErrors *GetDataGraphImageSapce();
        //TH2D* GetHistImageSpace(TString name="", TString title="");
        //TH2D* GetHistParamSpace(TString name="", TString title="");
        //void DrawToPads(TVirtualPad* padImage, TVirtualPad* padParam);
        //static void ClickPadParam(int iPlane);
        //TGraph* GetGraphPathToMaxWeight() { return fGraphPathToMaxWeight; }

    private:
        TVector3     fTransformCenter;
        bool         fInitializedImageData = false;
        bool         fInitializedParamData = false;
        double       fBinSizeMaxImageSpace = 0;

        double       fBinSizeMaxParamSpace = 0;
        int          fNumBinsImageSpace[3] = {10,10,10};
        int          fNumBinsParamSpace[4] = {10,10,10,10};
        double       fBinSizeImageSpace[3] = {0};
        double       fBinSizeParamSpace[4] = {0};
        double       fRangeImageSpace[3][2] = {{0}};
        double       fRangeParamSpace[4][2] = {{0}};
        double       fRangeParamSpaceInit[4][2] = {{0}};
        double****   fParamData;
        int          fIdxSelectedY = -1;
        int          fIdxSelectedZ = -1;
        int          fIdxSelectedT = -1;
        int          fIdxSelectedP = -1;

        double       fMaxWeightingDistance = 0;

        int             fNumImagePoints = 0;
        TClonesArray*   fImagePointArray = nullptr;
        LKImagePoint*   fImagePoint = nullptr;
        LKParamPointRT* fParamPoint = nullptr;

        int           fNumLinearTracks = 0;
        TClonesArray* fTrackArray = nullptr;

        int          fCutNumTrackHits = 5;
        LKODRFitter* fLineFitter = nullptr;

        //const int    kCorrelatePointBand = 0;
        //const int    kCorrelateBoxLine = 1;
        const int    kCorrelateBoxBand = 2;
        const int    kCorrelateBoxRBand = 3;
        //const int    kCorrelateDistance = 4;
        int          fCorrelateType = kCorrelateBoxRBand;

        LKHoughWeightingFunction* fWeightingFunction = nullptr;

        TGraph* fGraphPathToMaxWeight = nullptr;

        TPad *fPadImage = nullptr;
        TPad *fPadParam = nullptr;
        TH2D *fHistImage = nullptr;
        TH2D *fHistParam = nullptr;

    ClassDef(LKHTLineTracker3D,1);
};

#endif
