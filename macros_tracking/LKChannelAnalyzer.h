#ifndef LKCHANNELANALYZER_HH
#define LKCHANNELANALYZER_HH

#define DEBUG_FITPULSE

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
        void LeastSquareFitAtGivenTb(double *buffer, double tbStartOfPulse, int ndf, double &amplitude, double &chi2NDF);


        int GetTbMax() const  { return fTbMax; }
        int GetTbStart() const  { return fTbStart; }
        int GetTbStartCut() const  { return fTbStartCut; }
        int GetThreshold() const  { return fThreshold; }
        int GetThresholdOneTbStep() const  { return fThresholdOneTbStep; }
        int GetNumAcendingCut() const  { return fNumAcendingCut; }
        int GetDynamicRange() const  { return fDynamicRange; }
        int GetNDFPulse() const  { return fNDFPulse; }
        int GetIterMax() const  { return fIterMax; }
        double GetScaleTbStep() const { return fScaleTbStep; }
        double GetTbStepCut() const { return fTbStepCut; }

        void SetTbMax(int tbMax) { fTbMax = tbMax; }
        void SetTbStart(int tbStart) { fTbStart = tbStart; }
        void SetTbStartCut(int tbStartCut) { fTbStartCut = tbStartCut; }
        void SetThreshold(int threshold) { fThreshold = threshold; }
        void SetThresholdOneTbStep(int thresholdOneTbStep) { fThresholdOneTbStep = thresholdOneTbStep; }
        void SetNumAcendingCut(int numAcendingCut) { fNumAcendingCut = numAcendingCut; }
        void SetDynamicRange(int dynamicRange) { fDynamicRange = dynamicRange; }
        //void SetNDFPulse(int nDFPulse) { fNDFPulse = nDFPulse; }
        void SetIterMax(int iterMax) { fIterMax = iterMax; }
        void SetScaleTbStep(double scaleTbStep) { fScaleTbStep = scaleTbStep; }
        void SetTbStepCut(double scaleTbStep) { fTbStepCut = scaleTbStep; }

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
        int          fIterMax = 15;
        double       fScaleTbStep = 0.5;
        double       fTbStepCut = 1.e-5;

#ifdef DEBUG_FITPULSE
    public:
        TGraph* dGraphTb = nullptr;
        TGraph* dGraphTbStep = nullptr;
        TGraph* dGraphChi2 = nullptr;
        TGraph* dGraphBeta = nullptr;
        TGraph* dGraphTbChi2 = nullptr;
        TGraph* dGraphTbBeta = nullptr;
        TGraph* dGraphBetaInv = nullptr;
        TGraph* dGraphTbBetaInv = nullptr;
#endif

    ClassDef(LKChannelAnalyzer,1);
};

#endif
