#include "LKPulse.h"

ClassImp(LKPulse);

LKPulse::LKPulse()
{
    ;
}

LKPulse::LKPulse(const char *fileName)
{
    auto file = new TFile(fileName);
    auto graph = (TGraph *) file -> Get("pulse");
    SetPulse(graph);
    file -> Close();
}

bool LKPulse::Init()
{
    return true;
}

void LKPulse::Clear(Option_t *option)
{
}

void LKPulse::Print(Option_t *option) const
{
}

void LKPulse::SetPulse(TGraph* graph)
{
    fGraph = graph;
    fNumPoints = graph -> GetN();
}

double LKPulse::Eval(double tb)
{
    return fGraph -> Eval(tb);
}

TGraph *LKPulse::GetPulseGraph(double tb, double amplitude)
{
    auto graphPulse = new TGraph();

    double x, y;
    for (auto iPoint=0; iPoint<fNumPoints; ++iPoint)
    {
        fGraph -> GetPoint(iPoint,x,y);
        graphPulse -> SetPoint(iPoint,x+tb,y*amplitude);
    }
    graphPulse -> SetLineColor(kRed);
    graphPulse -> SetMarkerColor(kRed);
    return graphPulse;
}
