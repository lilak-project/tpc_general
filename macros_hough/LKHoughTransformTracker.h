#ifndef LKHOUGHTRANSFORMTRACKER_HH
#define LKHOUGHTRANSFORMTRACKER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "LKGeoLine.h"

#include "LKImagePoint.h"
#include "LKParamPointRT.h"
#include "LKImagePoint.cpp"
#include "LKParamPointRT.cpp"

#include "LKHoughWeightingFunction.h"

/**
    @brief The KHoughTransformTracker is tool for finding and fitting a straight line track from a provided list of hits.

    @param HT Hough Transform
    @param IMS Image Space
    @param IMP Image Point
    @param IMB Image point Box : box representing the bin-range or error of image point. 
    @param PMS Parameter Space
    @param PMP Parameter Point
    @param PML Parameter point presented as a Line
    @param PMB Parameter point presented as a Band
    @param PML Radial Line comming out from the transform center
    @param TC Transform Center : A origin point of Hough transform
    @param WF Weighting Function

    The Hough transform (HT) introduces parameter space (PMS) to find the appropriate parameters for the given model,
    which, in this case, is a straight line. The space describing the real dimensions (x, y) is referred to as image space (IMS),
    while the space describing the parameters of a model, (a straight line for this case,) (radius and theta) is referred to as PMS.
    Each space is defined as classes LKImagePoint and LKParamPointRT.

    We define a line; Parameter presented as a Line (PML), which is a straight line defined by two parameters selected from parameter space.
    The first parameter, "radius," represents the length of the radial line (PML) extending from the transform center.
    At the end of the PML, a PML is drawn perpendicular to it.
    The angle between the x-axis and the PML defines the second parameter, "theta".
    Thus, the angle between the x-axis and the PML is theta + 90 degrees.
    Note that the unit of angles used within this method is degrees.

    @param Transform-center (TC) The transform center is the position in IMS where the HT will be performed.
           The choice of the transform center will affect the results. It must be set using SetTransformCenter().
    @param Image-space-range The number of bins, minimum and maximum ranges for x and y, must be set. It must be set using SetImageSpaceRange().
    @param Param-space-range The number of bins for radius and theta parameters should be set.
           The range of the radius is automatically calculated using the transform center and IMS range.
           The binning does not need to be finer than 200x200 (in cases the case of TPCs with "average" spatial resolution for example),
           if the HT is employed only for the purpose of collecting hits and not for fitting tracks.
           It must be set using SetParamSpaceBins(). In case user wants to set the full range of radius and theta, it can be set using SetParamSpaceRange().
           For line fitting, the FitTrackWithParamPoint() method can be used.
    @param Correlation-method Choose from the following options. More descriptions are given below paragraph.
           1. Point-Band correlator : SetCorrelatePointBand()
           1. Box-Line correlator : SetCorrelateBoxLine()
           3. Box-Band correlator : SetCorrelateBoxBand()
           4. Distance correlator : SetCorrelateDistance()

    While the PML represents the line defined from the central value of the selected bin in PMS,
    a band; Parameter presented as a Band (PMB), represents the range that includes all parameter values within the selected bin range in PMS.
    It needs to be noted that each image point (IMP) also has its error represented by the bin in IMS.
    This definition will help understanding the correlator description below:

    There are three correlation methods to choose from:
    1. Point-Band correlator : This correlator checks if the Hough band include the center point of the image point box (IMB).
                               This method doos not care of the point error and take care of parameter point (PMP) error.
                               The speed of correlator is the fastest out of all.
    2. Box-Line correlator : This correlator checks if the Hough line passes through the IMB.
                             By selecting this correlator, the Hough point will be filled up only when the line precisely intersects the IMB.
                             This method take care of the point error and do not take care of PMP error.
                             This method is slower than Point-Band correlator.
                             It is not recommanded to use this method unless one understands what this actually does.
    3. Box-Band correlator : This correlator verifies whether there is an overlap between the Hough band and the IMB.
                             The correlation condition is less strict compared to the Point-Band and Box-Line correlators,
                             but it is more reasonable because both the PMP and the PMP are represented as bin boxes
                             rather than single points.
                             This method provides a general explanation for 
                             and is suitable for detectors with average spatial resolution.
                             However this method is slower than Point-Band and Box-Line correlators.
                             This correlator is default option.
    4. Distance correlator : This correlator calculate the perpendicular distance from IMB center to the PML.
                             User should choose the cut fMaxWeightingDistance,
                             The default value of fMaxWeightingDistance is diagonal length of IMS bin chosen from GetRangeImageSpace() method.
                             the maximum distance from the PML to weight the PMP using SetMaxWeightingDistance().
                             The speed of this method depend on cut value. For most of the cases, this is the most slowest out of all.

    ## Weighting function
    Weighting function is a function that gives weight value to fill up the PMS bin by correlating with IMB.
    Defualt weighting function is LKHoughWFInverse. The weight in this class is defined by: weight = [IMP weight] * [IMP error] / ([distance from IMP to PML] + [IMP error])
    Other weighting functions can be found in LKHoughTransformTracker.h, or one could create one by inheriting LKHoughTransformTracker class.

    The author of this method does not recommend the use of the HT method as a track fitter.
    The HT tool is efficient when it comes to grouping, But its performance as a fitter is limited by various factors.
    Instead, The author recommand to find the group of hits,
    then use FitTrackWithParamPoint() method which employ the least chi-squared method to fit straight line.

    If one intends to employ HT for the fitter, it's needed to use the smallest possible PMS binning to meet the user's desired resolution.
    If the spatial resolution is not impressive, the Box-Band correlator should be used. However, these adjustments will increase computation time.
    Even if good track parameters are identified, there's no assurance that the fit result will be superior to the least chi-squared method.
    In case of situation where only a single track exist in the event,
    small number of bins for PMS can be used along with RetransformFromLastParamPoint() method.
    RetransformFromLastParamPoint() method find the maximum PMS-bin,
    and recalculate HT parameters to range PMS within the selected bin.
    After, the Transform() method will be called which is equalivant to dividing PMS in to n^2 x m^2 bins,
    where nxm is the binning chosen by the user.

    Example of using LKHoughTransformTracker:

    @code{.cpp}
    {
         auto tracker = new LKHoughTransformTracker();
         tracker -> SetTransformCenter(TVector3(0,0,0));
         tracker -> SetImageSpaceRange(120, -150, 150, 120, 0, 500);
         tracker -> SetParamSpaceBins(numBinsR, numBinsT);
         for (...) {
             tracker -> AddImagePoint(x, xerror, y, yerror, weight);
         }
         tracker -> Transform();
         auto paramPoint = tracker -> FindNextMaximumParamPoint();
         track = tracker -> FitTrackWithParamPoint(paramPoint);
    }
    @endcode

    @todo The effect coming from choosing transform center in different positions should be added.
 */

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
        int GetNumBinsParamSpace(int ixy) const  { return fNumBinsParamSpace[ixy]; }
        double GetRangeImageSpace(int ixy, int i) const  { return fRangeImageSpace[ixy][i]; }
        double GetRangeParamSpace(int ixy, int i) const  { return fRangeParamSpace[ixy][i]; }
        double** GetParamData() const  { return fParamData; }
        double** GetImageData() const  { return fImageData; }
        int GetNumImagePoints() const  { return fNumImagePoints; }
        int GetNumParamPoints() const  { return fNumBinsParamSpace[0]*fNumBinsParamSpace[1]; }
        double GetMaxWeightingDistance(double distance) const { return fMaxWeightingDistance; }

        void SetTransformCenter(double x, double y) { fTransformCenter = TVector3(x,y,0); }
        void SetImageSpaceRange(int nx, double x2, double x1, int ny, double y1, double y2);
        void SetParamSpaceBins(int nr, int nt);
        void SetParamSpaceRange(int nr, double r2, double r1, int nt, double t1, double t2);
        void AddImagePoint(double x, double xError, double y, double yError, double weight);
        void AddImagePointBox(double x1, double y1, double x2, double y2, double weight);
        void SetImageData(double** imageData);

        void SetCorrelatePointBand();
        void SetCorrelateBoxLine();
        void SetCorrelateBoxBand();
        void SetCorrelateDistance();
        void SetMaxWeightingDistance(double distance) { fMaxWeightingDistance = distance; }
        void SetCutNumTrackHits(double value) { fCutNumTrackHits = value; }

        void SetWFConst()   { fWeightingFunction = new LKHoughWFConst(); }
        void SetWFLinear()  { fWeightingFunction = new LKHoughWFLinear(); }
        void SetWFInverse() { fWeightingFunction = new LKHoughWFInverse(); }
        void SetWeightingFunction(LKHoughWeightingFunction* wf) { fWeightingFunction = wf; }

        LKImagePoint* GetImagePoint(int i);
        LKParamPointRT* GetParamPoint(int i);
        LKParamPointRT* GetParamPoint(int ir, int it);

        void Transform();
        LKParamPointRT* FindNextMaximumParamPoint();
        LKLinearTrack* FitTrackWithParamPoint(LKParamPointRT* paramPoint, double weightCut=-1);
        void CleanLastParamPoint(double rWidth=-1, double tWidth=-1);
        LKParamPointRT* ReinitializeFromLastParamPoint();
        void RetransformFromLastParamPoint();

        TH2D* GetHistImageSpace(TString name="", TString title="");
        TH2D* GetHistParamSpace(TString name="", TString title="");
        void DrawAllParamLines(int i=-1, bool drawRadialLine=true);
        void DrawAllParamBands();

    private:

        TVector3     fTransformCenter;
        bool         fInitializedImageData = false;
        bool         fInitializedParamData = false;
        int          fNumBinsImageSpace[2] = {10,10};
        int          fNumBinsParamSpace[2] = {10,10};
        double       fBinSizeImageSpace[2] = {0};
        double       fBinSizeParamSpace[2] = {0};
        double       fBinSizeMaxImageSpace = 0;
        double       fBinSizeMaxParamSpace = 0;
        double       fRangeImageSpace[2][2] = {{0}};
        double       fRangeParamSpace[2][2] = {{0}};
        double       fRangeParamSpaceInit[2][2] = {{0}};
        double**     fParamData;
        double**     fImageData;
        int          fIdxSelectedR = -1;
        int          fIdxSelectedT = -1;
        double       fMaxWeightingDistance = 0;

        bool            fUsingImagePointArray = true;
        int             fNumImagePoints = 0;
        TClonesArray*   fImagePointArray = nullptr;
        LKImagePoint*   fImagePoint = nullptr;
        LKParamPointRT* fParamPoint = nullptr;

        int           fNumLinearTracks = 0;
        TClonesArray* fTrackArray = nullptr;

        int          fCutNumTrackHits = 5;
        LKODRFitter* fLineFitter = nullptr;

        const int    kCorrelatePointBand = 0;
        const int    kCorrelateBoxLine = 1;
        const int    kCorrelateBoxBand = 2;
        const int    kCorrelateDistance = 3;
        int          fCorrelateType = kCorrelateBoxBand;

        LKHoughWeightingFunction* fWeightingFunction = nullptr;

    ClassDef(LKHoughTransformTracker,1);
};

#endif
