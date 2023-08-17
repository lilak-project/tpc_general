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

        double GetWidth(int i=0) {
            if (i==1) return fWidth1;
            if (i==2) return fWidth2;
            return fWidth;
        }

    private:

        int fNumPoints = 0;
        TGraphErrors* fGraphPulse = nullptr;
        TGraph*       fGraphError = nullptr;
        double        fWidth;
        double        fWidth1;
        double        fWidth2;
    ClassDef(LKPulse,1);
};

#endif
