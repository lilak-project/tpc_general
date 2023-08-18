#ifndef LKCHANNELANALYZER_HH
#define LKCHANNELANALYZER_HH

#define DEBUG_FINDPEAK
#define DEBUG_CHANA_ANALYZE
//#define DEBUG_FITPULSE

#include "TObject.h"
#include "LKLogger.h"
#include "LKPulse.cpp"
#include "LKPulse.h"
#include <vector>
using namespace std;

/**
 * LKChannelAnalyzer should be set with pulse data file create from LKPulseAnalyzer.
 * See LKPulseAnalyzer for pulse data file.
 * Some parameters are set from pulse data file. Look for keyword "Automatically set".
 * Some parameters must be set by user. Look for keyword "Must be set".
 *
 * Critical parameters affecting the fitting speed and fit resolution are
 *
 * - fNDFFit : Number of points to use. Related to fitting speed. Automatically set from pulse: fNDFFit = fWidthLeading + fFWHM/4. Can be set with Set SetNDFPulse()
 * - fIterMax : Maximum iteration cut. Must be set with SetIterMax()
 * - fTbStepCut : Break from loop if fit-TB-step < fTbStepCut. Related to fitting speed and resolution. Can be set with SetTbStepCut().
 * - fScaleTbStep : (~0.2) Scale factor for choosing fit-TB-step for next TB candidate. Related to fitting speed and iteration #. Must be set with SetScaleTbStep()
 *
 * These parameters depend on experimental settings and therefore, performance by changing them should be tested.
 *
 * Example of using this LKChannelAnalyzer
 * \code{.cpp}
 *  {
 *      double* buffer = get_data_somehow;
 *
 *      auto ana = LKChannelAnalyzer()
 *      ana -> SetPulse("pulse_data_created_from_LKPulseAnalyzer.root");
 *      ana -> SetTbMax(512);
 *      ana -> SetTbStart(0);
 *      ana -> SetDynamicRange(4096);
 *      ana -> SetThresholdOneTbStep(2);
 *      ana -> SetThreshold(100);
 *      ana -> SetIterMax(15);
 *      ana -> SetTbStepCut(0.01);
 *      ana -> SetScaleTbStep(0.2);
 *      ana -> Analyze(buffer);
 *
 *      auto numHits = ana -> GetNumHits();
 *      for (auto iHit=0; iHit<numHits; ++iHit) {
 *          auto tbHit = ana -> GetTbHit(iHit);
 *          auto amplitude = ana -> GetAmplitude(iHit);
 *      }
 *  }
 * \endcode
 */

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
        double GetTbHit(int i) const  { return fTbHitArray[i]; }
        double GetAmplitude(int i) const  { return fAmplitudeArray[i]; }

        void SetPulse(const char* fileName);
        void SetPulse(LKPulse* pulse) { fPulse = pulse; }


        void Analyze(double* data);
        //LKChannelHit GetHit(int i) { return fChannelHitArray[i]; }

        /**
         * Find first peak from adc time-bucket starting from input tbCurrent
         * tbCurrent and tbStart becomes time-bucket of the peak and starting point
         */
        bool FindPeak(double *buffer, int &tbPointer, int &tbStartOfPulse);
        /**
         * Perform least square fitting with the the pulse around tbStart ~ tbPeak.
         * This process is iteration based process using method LSFitPuse()
         */
        bool FitPulse(double *buffer, int tbStartOfPulse, int tbPeak, double &tbHit, double &amplitude, double &squareSum, int &ndf, bool &isSaturated);
        /**
         * Compare pulse with the previous pulse
         * If the pulse is not distinguishable from the previous pulse, return false.
         * If the pulse is distinguishable, subtract pulse distribution from adc, and return true
         */
        bool TestPulse(double *buffer, double tbHitPre, double amplitudePre, double tbHit, double amplitude);
        /**
         * Perform least square fitting with the fixed parameter tbStart and ndf.
         * Amplitude is calculated from weighted least chi-square fitting
         * with waveform f(x) and weight w(x) = 1/(stddev(x)^2).
         * Amplitude = (Sum_i y_i * w(x_i) * f(x_i)) / (Sum_i w(x_i) f^2(x_i)
         */
        void LeastSquareFitAtGivenTb(double *buffer, double tbStartOfPulse, int ndf, double &amplitude, double &chi2NDF);

        int GetTbMax() const  { return fTbMax; }
        int GetTbStart() const  { return fTbStart; }
        int GetTbStartCut() const  { return fTbStartCut; }
        int GetThreshold() const  { return fThreshold; }
        int GetThresholdOneTbStep() const  { return fThresholdOneStep; }
        int GetNumTbAcendingCut() const  { return fNumTbAcendingCut; }
        int GetDynamicRange() const  { return fDynamicRange; }
        int GetNDF() const  { return fNDFFit; }
        int GetIterMax() const  { return fIterMax; }
        double GetScaleTbStep() const { return fScaleTbStep; }
        double GetTbStepCut() const { return fTbStepCut; }

        void SetTbMax(int tbMax) { fTbMax = tbMax; }
        void SetTbStart(int tbStart) { fTbStart = tbStart; }
        void SetTbStartCut(int tbStartCut) { fTbStartCut = tbStartCut; }
        void SetThreshold(int threshold) { fThreshold = threshold; }
        void SetThresholdOneStep(int oneStepThreshold) { fThresholdOneStep = oneStepThreshold; }
        void SetNumTbAcendingCut(int numAcendingCut) { fNumTbAcendingCut = numAcendingCut; }
        void SetDynamicRange(int dynamicRange) { fDynamicRange = dynamicRange; }
        void SetNDFPulse(int nDFPulse) { fNDFFit = nDFPulse; }
        void SetIterMax(int iterMax) { fIterMax = iterMax; }
        void SetScaleTbStep(double scaleTbStep) { fScaleTbStep = scaleTbStep; }
        void SetTbStepCut(double scaleTbStep) { fTbStepCut = scaleTbStep; }

        double* GetBuffer() { return fBuffer; }

    private:

        LKPulse*     fPulse = nullptr; ///< Pulse pointer
        double       fBuffer[512]; ///< Copied buffer from data
        int          fNumHits = 0; ///< Number of hits found after Analyze()
        vector<double> fTbHitArray; ///< Vector holding fit TB
        vector<double> fAmplitudeArray; ///< Vector holding fit amplitude

        // tb
        int          fTbMax = 350; ///< Maximum TB in buffer. Must be set with SetTbMax()
        int          fTbStart = 1; ///< Starting TB-position for analysis. Must be set with SetTbStart()
        int          fTbStartCut = -1; ///< Pulse TB-position cannot be larger than fTbStartCut. Automatically set from pulse: fTbStartCut = fTbMax - fNDFFit. Can be set with SetTbStartCut()
        int          fNumTbAcendingCut = 5; ///< Peak will be recognize if number-of-TBs-acending >= fNumTbAcendingCut. Automatically set from pulse: fNumTbAcendingCut = int(fWidthLeading*2/3).Can be set with SetNumTbAcendingCut()

        // y
        int          fDynamicRange = 4096; ///< Dynamic range of buffer. Used for recognizing saturation. Must be set with SetDynamicRange()
        int          fThreshold = 50; ///< Threshold for recognizing pulse peak. Must be set with SetThreshold()
        int          fThresholdOneStep = 2; ///< Threshold for counting number-of-TBs-acending (y-current > y-previous). Must be set with SetThresholdOneTbStep()

        // peak finding
        int          fTbStepIfFoundHit = 10; ///< [Peak finding] TB-step after hit was found. Automatically set from pulse: fTbStepIfFoundHit = fNDFFit.
        int          fTbStepIfSaturated = 15; ///< [Peak finding] TB-step after saturation was found. Automatically set from pulse: fTbStepIfSaturated = int(fWidth*1.2);
        int          fTbSeparationWidth = 10; ///< [Peak finding] Estimation of # of TB that can separate two different pulse. Automatically set from pulse: fTbSeparationWidth = fNDFFit.
        int          fNumTbsCorrection = 50; ///< [Peak finding] Number of TBs to subtract from the found pulse. Automatically set from pulse: fNumTbsCorrection = int(numPulsePoints);

        // fitting
        int          fNDFFit = 0; ///< [Pulse-fitting] Number of points to use. Related to fitting speed. Automatically set from pulse: fNDFFit = fWidthLeading + fFWHM/4. Can be set with Set SetNDFPulse()
        int          fIterMax = 15; ///< [Pulse-fitting] Maximum iteration cut. Must be set with SetIterMax()
        double       fTbStepCut = 0.01; ///< [Pulse-fitting] Break from loop if fit-TB-step < fTbStepCut. Related to fitting speed and resolution. Can be set with SetTbStepCut().
        double       fScaleTbStep = 0.5; ///< [Pulse-fitting] (~0.2) Scale factor for choosing fit-TB-step for next TB candidate. Related to fitting speed and iteration #. Must be set with SetScaleTbStep()

        double       fFWHM; ///< [Pulse] Full Width Half Maximum of reference pulse. Automatically set from pulse.
        double       fFloorRatio; ///< [Pulse] Ratio regard to the full pulse height where pulse width was measured. Automatically set from pulse.
        double       fWidth; ///< [Pulse] Width of the pulse. Automatically set from pulse.
        double       fWidthLeading; ///< [Pulse] Width measured from leading edge to TB-at-peak. Automatically set from pulse.
        double       fWidthTrailing; ///< [Pulse] Width measured from TB-at-peak to trailing edge. Automatically set from pulse.

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
