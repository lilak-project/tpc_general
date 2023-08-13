#ifndef LKPULSE_HH
#define LKPULSE_HH

#include "TObject.h"
#include "LKLogger.h"
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

        void SetPulse(TGraph* graph);
        double Eval(double tb);

        TGraph *GetPulseGraph(double tb, double amplitude);

    private:

        int          fNumPoints = 0;
        TGraph*      fGraph = nullptr;

    ClassDef(LKPulse,1);
};

#endif
