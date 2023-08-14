#ifndef LKCHANNELANALYZER_HH
#define LKCHANNELANALYZER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TVirtualPad.h"
#include "TGraph.h"
#include "TObjArray.h"
#include "TFile.h"
#include "TTree.h"

class LKPulseAnalyzer : public TObject
{
    public:
        LKPulseAnalyzer(const char* name, const char *path=".");

        LKPulseAnalyzer() : LKPulseAnalyzer(0) { ; }
        virtual ~LKPulseAnalyzer() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        double* GetAverageData() { return fAverageData; }
        double* GetChannelData() { return fChannelData; }
        TCanvas* GetCvsAverage() const  { return fCvsAverage; }
        TH1D* GetHistAverage() const  { return fHistAverage; }
        TH1D* GetHistMean() const  { return fHistMean; }
        TH2D* GetHistAccumulate() const  { return fHistAccumulate; }
        TTree* GetTree() { return fTree; }

        void SetInvertChannel(bool value) { fInvertChannel = value; }
        void SetTbMax(int value) { fTbMax = value; }
        void SetTbStart(int value) { fTbStart = value; }
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
        void SetFixPedestal(double value) { fFixPedestal = value; }

        TCanvas* GetGroupCanvas() { return fCvsGroup; }
        int IsCollected() const { return fIsCollected; }
        int GetNumGoodChannels() const { return fCountGoodChannels; }
        int GetNumHistChannels() const { return fCountHistChannel; }
        int GetNumHistLocal() const { return fCountHistLocal; }
        int GetNumCvsGroup() const { return fCountCvsGroup; }
        double GetPedestalPry() const { return fPedestalPry; }

        void AddChannel(int *data) { AddChannel(data,-1); }
        void AddChannel(int *data, int event, int cobo, int asad, int aget, int channel);
        void AddChannel(int *data, int channelID);

        void DumpChannel(Option_t *option="");
        void WriteTree();

        bool DrawChannel();
        TCanvas* DrawMean(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawAverage(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawAccumulate(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawWidth(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawHeight(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawPulseTb(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawPedestal(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawResidual(TVirtualPad* pad=(TVirtualPad*)nullptr);
        TCanvas* DrawReference(TVirtualPad *pad=(TVirtualPad*)nullptr);
        TCanvas* DrawHeightWidth(TVirtualPad* pad=(TVirtualPad*)nullptr);

        TObjArray *GetHistArray() { return fHistArray; }

        double FullWidthRatioMaximum(TH1D *hist, double ratioFromMax, double numSplitBin, double &x0, double &x1, double &error);
        double FullWidthRatioMaximum(TH1D *hist, double ratioFromMax, double numSplitBin=4) {
            double dummy;
            return FullWidthRatioMaximum(hist, ratioFromMax, numSplitBin, dummy, dummy, dummy);
        }

        void WriteReferecePulse(int tbOffsetFromHead=0, int tbOffsetFromtail=0);

        void SetCvs(TCanvas *cvs);
        void SetHist(TH1 *hist);

    private:
        const char*  fName = "";
        const char*  fPath = ".";

        // single channel cuts
        int          fTbStart = 0;
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
        bool         fValueIsAboveThreshold = false;
        int          fCountPedestalPry = 0;
        double       fPedestalPry = 0;
        double       fPedestal = 0;
        double       fFixPedestal = -999;
        double       fWidth = 0;
        double       fChannelData[512];
        int          fEventID = -1;
        short        fCobo = -1;
        short        fAsad = -1;
        short        fAget = -1;
        int          fChannel = -1;

        // single channel draw
        int          fCountHistChannel = 0;
        int          fCountHistLocal = 0;
        int          fCountCvsGroup = 0;
        TCanvas*     fCvsGroup = nullptr;
        int          fWGroup = 1000;
        int          fHGroup = 800;
        int          fXGroup = 3;
        int          fYGroup = 2;

        // average
        int          fCountGoodChannels = 0;
        double       fAverageData[512];
        double       fTbAtRefFloor1 = 0;
        double       fTbAtRefFloor2 = 0;
        double       fRefWidth = 0;

        // average draw
        TObjArray*   fHistArray = nullptr;
        TCanvas*     fCvsMean = nullptr;
        TH1D*        fHistMean = nullptr;
        TCanvas*     fCvsAverage = nullptr;
        TH1D*        fHistAverage = nullptr;
        TCanvas*     fCvsReference = nullptr;
        TH2D*        fHistReference = nullptr;
        TGraph*      fGraphReference = nullptr;
        TCanvas*     fCvsAccumulate = nullptr;
        TH2D*        fHistAccumulate = nullptr;
        int          fWAverage = 600;
        int          fHAverage = 500;

        // extra
        TCanvas*     fCvsWidth = nullptr;
        TH1D*        fHistWidth = nullptr;
        TCanvas*     fCvsHeight = nullptr;
        TH1D*        fHistHeight = nullptr;
        TCanvas*     fCvsPulseTb = nullptr;
        TH1D*        fHistPulseTb = nullptr;
        TCanvas*     fCvsResidual = nullptr;
        TH2D*        fHistResidual = nullptr;
        TCanvas*     fCvsPedestal = nullptr;
        TH1D*        fHistPedestal = nullptr;
        TH1D*        fHistReusedData = nullptr;
        TH1D*        fHistPedestalPry = nullptr;
        TCanvas*     fCvsHeightWidth = nullptr;
        TH2D*        fHistHeightWidth = nullptr;

        // data
        TFile*       fFile = nullptr;
        TTree*       fTree = nullptr;
        bool         fIsSinglePulseChannel = false;
        bool         fIsCollected = false;

    ClassDef(LKPulseAnalyzer,1);
};

#endif
