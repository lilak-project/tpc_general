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
        double Eval(double tb);
        double Error(double tb);

        TGraphErrors *GetPulseGraph(double tb, double amplitude);

        int GetNDF() const { return fNumPoints; }

    private:

        int fNumPoints = 0;
        TGraphErrors* fGraphPulse = nullptr;
        TGraph*       fGraphError = nullptr;

    ClassDef(LKPulse,1);
};

#endif
