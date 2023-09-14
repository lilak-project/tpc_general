#ifndef LKHOUGHTRANSFORMTRACKER_HH
#define LKHOUGHTRANSFORMTRACKER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "LKGeoLine.h"

#include "LKImagePoint.h"
#include "LKHoughPointRT.h"
#include "LKImagePoint.cpp"
#include "LKHoughPointRT.cpp"

#include "LKHoughWeightingFunction.h"

/**
    @brief The KHoughTransformTracker is tool for finding and fitting a straight line track from a provided list of hits.

    The Hough transform introduces Hough space to find the appropriate parameters for the given model,
    which, in this case, is a straight line. The space describing the real dimensions (x, y) is referred to as image space,
    while the space describing the parameters of a straight line (radius and theta) is referred to as Hough space.
    Each space is defined as classes LKImagePoint and LKHoughPointRT.

    We define a "hough line", which is a straight line defined by two parameters selected from hough space.
    The first parameter, "radius," represents the length of the radial line extending from the transform center.
    At the end of the radial line, a hough line is drawn perpendicular to it.
    The angle between the x-axis and the radial line defines the second parameter, "theta".
    Thus, the angle between the x-axis and the hough line is theta + 90 degrees.
    Note that the unit of angles used within this method is degrees.

    @param Transform-center The transform center is the position in image space where the Hough transform will be performed.
           The choice of the transform center's position will affect the results. It must be set using SetTransformCenter().
    @param Image-space-range The number of bins, minimum and maximum ranges for x and y, must be set. It must be set using SetImageSpaceRange().
    @param Hough-space-range The number of bins for radius and theta parameters should be set.
           The range of the radius is automatically calculated using the transform center and image space range.
           The binning does not need to be finer than 100x100 (in cases the case of TPCs with "average" spatial resolution for example),
           if the Hough transform is employed only for the purpose of collecting hits and not for fitting tracks.
           It must be set using SetHoughSpaceBins(). In case user wants to set the full range of radius and theta, it can be set using SetHoughSpaceRange().
           For line fitting, the FitTrackWithHoughPoint() method can be used.
    @param Correlation-method Choose from the following options. More descriptions are given below paragraph.
           1. Point-Band correlator : SetCorrelatePointBand()
           1. Box-Line correlator : SetCorrelateBoxLine()
           3. Box-Band correlator : SetCorrelateBoxBand()
           4. Distance correlator : SetCorrelateDistance()

    While the "hough line" represents the line defined from the central value of the selected bin in Hough space,
    a "hough band" represents the range that includes all parameter values within the selected bin range in Hough space.
    It needs to be noted that each image point also has its error represented by the bin in image space.
    This definition will help understanding the correlator description below:

    There are three correlation methods to choose from:
    1. Point-Band correlator : This correlator checks if the Hough band include the center point of the image-bin.
                               This method doos not care of the point error and take care of hough point error.
                               The speed of correlator is the fastest out of all.
    2. Box-Line correlator : This correlator checks if the Hough line passes through the image-bin-box.
                             By selecting this correlator, the Hough point will be filled up only when the line precisely intersects the image-bin.
                             This method take care of the point error and do not take care of hough point error.
                             This method is slower than Point-Band correlator.
                             It is not recommanded to use this method unless one understands what this actually does.
    3. Box-Band correlator : This correlator verifies whether there is an overlap between the Hough band and the image-bin-box.
                             The correlation condition is less strict compared to the Point-Band and Box-Line correlators,
                             but it is more reasonable because both the image point and the hough point are represented as bin boxes
                             rather than single points.
                             This method provides a general explanation for Hough transform
                             and is suitable for detectors with average spatial resolution.
                             However this method is slower than Point-Band and Box-Line correlators.
                             This correlator is default option.
    4. Distance correlator : This correlator calculate the perpendicular distance from image-bin center to the hough line.
                             User should choose the cut fMaxWeightingDistance,
                             The default value of fMaxWeightingDistance is diagonal length of image space bin chosen from GetRangeImageSpace() method.
                             the maximum distance from the hough line to weight the hough point using SetMaxWeightingDistance().
                             The speed of this method depend on cut value. For most of the cases, this is the most slowest out of all.

    ## Weighting function
    Weighting function is a function that gives weight value to fill up the hough space bin by correlating with image-bin.
    Defualt weighting function is LKHoughWFInverse. The weight in this class is defined by:
        weight = [distance from image point to hough line] .....
    Other weighting functions can be found in LKHoughTransformTracker.h, or one could create one by inheriting LKHoughTransformTracker class.

    The author of this method does not recommend the use of the Hough transform method as a track fitter.
    The Hough transform tool is efficient when it comes to grouping, But its performance as a fitter is limited by various factors.
    Instead, The author recommand to find the group of hits,
    then use FitTrackWithHoughPoint() method which employ the least chi-squared method to fit straight line.

    If one intends to employ hough transform for the fitter,
    it's needed to use the smallest possible Hough space binning to meet the user's desired resolution.
    If the spatial resolution is not impressive, the Box-Band correlator should be used. However, these adjustments will increase computation time.
    Even if good track parameters are identified, there's no assurance that the fit result will be superior to the least chi-squared method.
    In case of situation where only a single track exist in the event,
    small number of bins for hough space can be used along with RetransformFromLastHoughPoint() method.
    RetransformFromLastHoughPoint() method find the maximum hough space bin,
    and recalculate Hough transform parameters to range hough space within the selected bin.
    After, the Transform() method will be called which is equalivant to dividing hough space in to n^2 x m^2 bins,
    where nxm is the binning chosen by the user.

    Example of using LKHoughTransformTracker:

    @code{.cpp}
    {
         auto tracker = new LKHoughTransformTracker();
         tracker -> SetTransformCenter(TVector3(0,0,0));
         tracker -> SetImageSpaceRange(120, -150, 150, 120, 0, 500);
         tracker -> SetHoughSpaceBins(numBinsR, numBinsT);
         for (...) {
             tracker -> AddImagePoint(x, xerror, y, yerror, weight);
         }
         tracker -> Transform();
         auto houghPoint = tracker -> FindNextMaximumHoughPoint();
         track = tracker -> FitTrackWithHoughPoint(houghPoint);
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
        int GetNumBinsHoughSpace(int ixy) const  { return fNumBinsHoughSpace[ixy]; }
        double GetRangeImageSpace(int ixy, int i) const  { return fRangeImageSpace[ixy][i]; }
        double GetRangeHoughSpace(int ixy, int i) const  { return fRangeHoughSpace[ixy][i]; }
        double** GetHoughData() const  { return fHoughData; }
        double** GetImageData() const  { return fImageData; }
        int GetNumImagePoints() const  { return fNumImagePoints; }
        int GetNumHoughPoints() const  { return fNumBinsHoughSpace[0]*fNumBinsHoughSpace[1]; }
        double GetMaxWeightingDistance(double distance) const { return fMaxWeightingDistance; }

        void SetTransformCenter(double x, double y) { fTransformCenter = TVector3(x,y,0); }
        void SetImageSpaceRange(int nx, double x2, double x1, int ny, double y1, double y2);
        void SetHoughSpaceBins(int nr, int nt);
        void SetHoughSpaceRange(int nr, double r2, double r1, int nt, double t1, double t2);
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
        LKHoughPointRT* GetHoughPoint(int i);
        LKHoughPointRT* GetHoughPoint(int ir, int it);

        void Transform();
        LKHoughPointRT* FindNextMaximumHoughPoint();
        LKLinearTrack* FitTrackWithHoughPoint(LKHoughPointRT* houghPoint, double weightCut=-1);
        void CleanLastHoughPoint(double rWidth=-1, double tWidth=-1);
        LKHoughPointRT* ReinitializeFromLastHoughPoint();
        void RetransformFromLastHoughPoint();

        TH2D* GetHistImageSpace(TString name="", TString title="");
        TH2D* GetHistHoughSpace(TString name="", TString title="");
        void DrawAllHoughLines(int i=-1, bool drawRadialLine=true);
        void DrawAllHoughBands();

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
        double       fRangeHoughSpaceInit[2][2] = {{0}};
        double**     fHoughData;
        double**     fImageData;
        int          fIdxSelectedR = -1;
        int          fIdxSelectedT = -1;
        double       fMaxWeightingDistance = 0;

        bool            fUsingImagePointArray = true;
        int             fNumImagePoints = 0;
        TClonesArray*   fImagePointArray = nullptr;
        LKImagePoint*   fImagePoint = nullptr;
        LKHoughPointRT* fHoughPoint = nullptr;

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
