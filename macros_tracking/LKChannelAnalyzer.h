#ifndef LKCHANNELANALYZER_HH
#define LKCHANNELANALYZER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TVirtualPad.h"

class LKChannelAnalyzer : public TObject
{
    public:
        LKChannelAnalyzer(const char* name);
        LKChannelAnalyzer() : LKChannelAnalyzer(0) { ; }
        virtual ~LKChannelAnalyzer() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        double* GetAverageData() { return fAverageData; }
        double* GetChannelData() { return fChannelData; }
        TCanvas* GetCvsAverage() const  { return fCvsAverage; }
        TH1D* GetHistAverage() const  { return fHistAverage; }

        int GetTbMax() const  { return fTbMax; }
        int GetChannelMax() const  { return fChannelMax; }
        int GetThreshold() const  { return fThreshold; }
        int GetPulseHeightMin() const  { return fPulseHeightMin; }
        int GetPulseHeightMax() const  { return fPulseHeightMax; }
        int GetPulseTbMin() const  { return fPulseTbMin; }
        int GetPulseTbMax() const  { return fPulseTbMax; }
        int GetPulseWidthAtThresholdMin() const  { return fPulseWidthAtThresholdMin; }
        int GetPulseWidthAtThresholdMax() const  { return fPulseWidthAtThresholdMax; }

        void SetInvertChannel(bool value) { fInvertChannel = value; }
        void SetTbMax(int value) { fTbMax = value; }
        void SetChannelMax(int value) { fChannelMax = value; }
        void SetThreshold(int value) { fThreshold = value; }
        void SetPulseHeightCuts(int min, int max) {
            fPulseHeightMin = min;
            fPulseHeightMax = max;
        }
        void SetPulseTbCuts(int min, int max) {
            fPulseTbMin = min;
            fPulseTbMax = max;
        }
        void SetPulseWidthCuts(int min, int max) {
            fPulseWidthAtThresholdMin = min;
            fPulseWidthAtThresholdMax = max;
        }
        void SetCvsGroup(int w, int h, int x, int y) {
            fWGroup = w;
            fHGroup = h;
            fXGroup = x;
            fYGroup = y;
        }
        void SetCvsAverage(int w, int h) {
            fWAverage = w;
            fHAverage = h;
        }

        TCanvas* GetGroupCanvas() { return fCvsGroup; }
        int IsGoodChannel() const { return fIsGoodChannel; }
        int GetNumGoodChannels() const { return fCountGoodChannels; }
        int GetNumHistChannel() const { return fCountHistChannel; }
        int GetNumHistLocal() const { return fCountHistLocal; }
        int GetNumCvsGroup() const { return fCountCvsGroup; }

        void AddChannel(int *data) { AddChannel(-1,data); }
        void AddChannel(int channelID, int *data);
        bool DrawChannel();
        TCanvas* DrawAverage(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawWidth(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawHeight(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawPulseTb(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawPedestal(TVirtualPad* pad=(TVirtualPad*)nullptr);

        double FullWidthRatioMaximum(TH1D *hist, double ratioFromMax, double numSplitBin, double &x0, double &x1, double &error);
        double FullWidthRatioMaximum(TH1D *hist, double ratioFromMax) {
            double dummy;
            return FullWidthRatioMaximum(hist, ratioFromMax, 4, dummy, dummy, dummy);
        }

    private:
        const char*  fName = 0;

        // single channel cuts
        int          fTbMax = 350;
        int          fChannelMax = 4096;
        int          fThreshold = 1000;
        int          fPulseHeightMin = 1500;
        int          fPulseHeightMax = 2500;
        int          fPulseTbMin = 100;
        int          fPulseTbMax = 200;
        int          fPulseWidthAtThresholdMin = 4;
        int          fPulseWidthAtThresholdMax = 30;

        // single channel parameters
        int          fMaxValue = 0;
        int          fTbAtMaxValue = 0;
        int          fFirstPulseTb = -1;
        int          fPreValue = 0;
        int          fCurValue = 0;
        int          fCountTbWhileAbove = 0;
        int          fCountPulse = 0;
        int          fCountWidePulse = 0;
        int          fChannelID = 0;
        bool         fInvertChannel = false;
        bool         fIsGoodChannel = false;
        bool         fValueIsAboveThreshold = false;
        double       fPedestal = 0;
        double       fChannelData[512];

        // single channel draw
        int          fCountHistChannel = 0;
        int          fCountHistLocal = 0;
        int          fCountCvsGroup = 0;
        TCanvas*     fCvsGroup = nullptr;
        int          fWGroup = 1000;
        int          fHGroup = 800;
        int          fXGroup = 3;
        int          fYGroup = 2;

        // all channel
        int          fCountGoodChannels = 0;
        double       fAverageData[512];

        // all channel draw
        TCanvas*     fCvsAverage = nullptr;
        TH1D*        fHistAverage = nullptr;
        int          fWAverage = 600;
        int          fHAverage = 500;

        // extra
        TCanvas*     fCvsWidth = nullptr;
        TH1D*        fHistWidth = nullptr;
        TCanvas*     fCvsHeight = nullptr;
        TH1D*        fHistHeight = nullptr;
        TCanvas*     fCvsPulseTb = nullptr;
        TH1D*        fHistPulseTb = nullptr;
        TCanvas*     fCvsPedestal = nullptr;
        TH1D*        fHistPedestal = nullptr;

    ClassDef(LKChannelAnalyzer,1);
};

#endif
