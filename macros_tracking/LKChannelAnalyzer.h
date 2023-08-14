#ifndef LKCHANNELANALYZER_HH
#define LKCHANNELANALYZER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "LKPulse.cpp"
#include "LKPulse.h"
#include <vector>
using namespace std;

//class LKChannelHit
//{
//    public:
//        LKChannelHit(double tb, double amp) : fAmplitude(amp), fTb(tb) {}
//        ~LKChannelHit() {}
//        double fAmplitude;
//        double fTb;
//        //double integral;
//        //bool saturated;
//};

class LKChannelAnalyzer : public TObject
{
    public:
        LKChannelAnalyzer();
        virtual ~LKChannelAnalyzer() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        LKPulse* GetPulse() const  { return fPulse; }
        int GetNumHits() const  { return fNumHits; }

        void SetPulse(const char* fileName);
        void SetPulse(LKPulse* pulse) { fPulse = pulse; }
        void SetNumHits(int numHits) { fNumHits = numHits; }
        void SetThreshold(int value) { fThreshold = value; }
        void SetThresholdOneTbStep(int value) { fThresholdOneTbStep = value; }

        int GetThreshold()          { return fThreshold; }
        int GetThresholdOneTbStep() { return fThresholdOneTbStep; }

        void Analyze(double* data);
        //LKChannelHit GetHit(int i) { return fChannelHitArray[i]; }

        /** 1.
         * Find the first peak from adc time-bucket starting from input tbCurrent
         * tbCurrent and tbStart becomes time-bucket of the peak and starting point
         */
        bool FindPeak(double *buffer, int &tbPointer, int &tbStartOfPulse);
        /** 2.
         * Perform least square fitting with the the pulse around tbStart ~ tbPeak.
         * This process is Iteration based process using method LSFitPuse();
         */
        bool FitPulse(double *buffer, int tbStartOfPulse, int tbPeak, double &tbHit, double &amplitude, double &squareSum, int &ndf, bool &isSaturated);
        /** 3.
         * Test pulse with the previous pulse
         * If the pulse is not distinguishable from the previous pulse, return false.
         * If the pulse is distinguishable, subtract pulse distribution from adc,
         * and return true
         */
        bool TestPulse(double *buffer, double tbHitPre, double amplitudePre, double tbHit, double amplitude);
        /**
         * Perform least square fitting with the fixed parameter tbStart and ndf.
         * This process is analytic. The amplitude is choosen imidiatly.
         */
        void LeastSquareFitAtGivenTb(double *buffer, double tbStartOfPulse, int ndf, double &amplitude, double &chi2);

    private:

        //vector<LKChannelHit> fChannelHitArray;

        LKPulse*     fPulse = nullptr;
        double       fBuffer[512];
        int          fNumHits = 0;

        int          fTbMax = 350;
        int          fTbStart = 1;
        int          fTbStartCut = 330;
        int          fThreshold = 600;
        int          fThresholdOneTbStep = 2;
        int          fNumAcendingCut = 5;
        int          fDynamicRange = 4096;
        int          fNDFPulse = 0;
        int          fIterMax = 40;

        /**
         * Proportional parameter for the time-bucket step when fitting tbStart.
         *
         * Each time the least square fit is performed for amplitude of the pulse
         * with fixed tbStart, we have to select next candidate of the tbStart. 
         * The step of time-bucket from current tbStart is choosen as
         * step = fScaleTbStep / peak^2 * stepChi2NDF.
         *
         * for stepChi2NDF, see fStepChi2NDFCut.
         */
        double fScaleTbStep = 5000.;

        /**
         * The default cut for stepChi2NDF. If stepChi2NDF becomes lower than fStepChi2NDFCut
         * for twice in a row, the fit is satisfied.
         *
         * stepChi2NDF is defined by
         *   minus of [difference between least-squares of previous fit and current fit] 
         *   divided by [difference between time-bucket of previous fit and current fit] 
         *   divided by NDF: 
         *   stepChi2NDF = (lsPre-lsCur)/(tbCur-tbPre)/ndf.
         *
         * This cut is re-defined as effective threhold for each pulses 
         * betaCut = fStepChi2NDFCut * peak^2
         */
        double fStepChi2NDFCut = 1.e-5;
        //double fStepChi2NDFCut = 1;

#define DEBUG_FITPULSE
#ifdef DEBUG_FITPULSE
    public:
        TGraph* dGraphStep = nullptr;
        TGraph* dGraphTime = nullptr;
        TGraph* dGraphChi2 = nullptr;
        TGraph* dGraphBeta = nullptr;
        TGraph* dGraphTbC2 = nullptr;
#endif

    ClassDef(LKChannelAnalyzer,1);
};

#endif
