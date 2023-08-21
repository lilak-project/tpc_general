#ifndef LKCHANNELSIMULATOR_HH
#define LKCHANNELSIMULATOR_HH

#include "TObject.h"
#include "LKLogger.h"
#include "LKPulse.cpp"
#include "LKPulse.h"

class LKChannelSimulator : public TObject
{
    public:
        LKChannelSimulator();
        virtual ~LKChannelSimulator() { ; }

        void SetPulse(const char* fileName);
        void SetYMax(int yMax) { fYMax = yMax; }
        void SetTbMax(int yMax) { fTbMax = yMax; }
        void SetNumSmoothing(int num) { fNumSmoothing = num; }
        void SetSmoothingLength(int length) { fSmoothingLength = length; }
        void SetPedestalFluctuationScale(double scale) { fPedestalFluctuationScale = scale; }
        void SetPedestalFluctuationLength(int length) { fPedestalFluctuationLength = length; }
        void SetPulseErrorScale(double scale) { fPulseErrorScale = scale; }
        void SetBackGroundLevel(double value) { fBackGroundLevel = value; }

        void SetPedestal(int* buffer);
        void SetFluctuatingPedestal(int* buffer);
        void AddHit(int* buffer, double tb0, double amplitude);

    public:
        void Smoothing(int* buffer, int n, int smoothLevel, int numSmoothing);

    private:
        LKPulse*     fPulse = nullptr;

        int          fYMax = 4096;
        int          fTbMax = 512;
        int          fNumSmoothing = 2;
        int          fSmoothingLength = 4;
        double       fBackGroundLevel = 400;
        double       fPedestalFluctuationLevel = 1;
        double       fPedestalFluctuationScale = 1; ///< scale=0 will draw flat background distribution where scale=1 will draw background with standard error
        int          fPedestalFluctuationLength = 4;
        double       fPulseErrorScale = 0.05;

    ClassDef(LKChannelSimulator,1);
};

#endif
