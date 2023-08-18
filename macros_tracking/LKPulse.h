#ifndef LKPULSE_HH
#define LKPULSE_HH

#include "TObject.h"
#include "LKLogger.h"
#include "TGraphErrors.h"
#include "TGraph.h"

class LKPulse : public TObject
{
    public:
        LKPulse();
        LKPulse(const char *fileName);
        virtual ~LKPulse() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        void SetPulse(TGraphErrors* graph);
        void SetError(TGraph* graph);
        double Eval(double tb, double tb0, double amplitude);
        double Eval(double tb);
        double Error(double tb);

        TGraphErrors *GetPulseGraph(double tb, double amplitude);

        int GetNDF() const { return fNumPoints; }
        int GetMultiplicity() const  { return fMultiplicity; }
        int GetThresholdC() const  { return fThreshold; }
        int GetHeightMin() const  { return fHeightMin; }
        int GetHeightMax() const  { return fHeightMax; }
        int GetTbMin() const  { return fTbMin; }
        int GetTbMax() const  { return fTbMax; }
        double GetFWHM() const  { return fFWHM; }
        double GetFloorRatio() const  { return fFloorRatio; }
        double GetWidth() const  { return fRefWidth; }
        double GetLeadingWidth() const  { return fWidthLeading; }
        double GetTrailingWidth() const  { return fWidthTrailing; }

    private:

        int fNumPoints = 0;
        TGraphErrors* fGraphPulse = nullptr;
        TGraph*       fGraphError = nullptr;

        int          fMultiplicity;
        int          fThreshold;
        int          fHeightMin;
        int          fHeightMax;
        int          fTbMin;
        int          fTbMax;
        double       fFWHM;
        double       fFloorRatio;
        double       fRefWidth;
        double       fWidthLeading;
        double       fWidthTrailing;



    ClassDef(LKPulse,1);
};

#endif
