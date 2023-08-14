#include "LKPulse.h"

ClassImp(LKPulse);

LKPulse::LKPulse()
{
    ;
}

LKPulse::LKPulse(const char *fileName)
{
    auto file = new TFile(fileName);
    auto graphPulse = (TGraphErrors *) file -> Get("pulse");
    SetPulse(graphPulse);
    auto graphError = (TGraph*) file -> Get("error");
    SetError(graphError);
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

void LKPulse::SetPulse(TGraphErrors* graph)
{
    fGraphPulse = graph;
    fNumPoints = graph -> GetN();
}

void LKPulse::SetError(TGraph* graph)
{
    fGraphError = graph;
}

double LKPulse::Eval(double tb)
{
    return fGraphPulse -> Eval(tb);
}

double LKPulse::Error(double tb)
{
    return fGraphError -> Eval(tb);
}

TGraphErrors *LKPulse::GetPulseGraph(double tb, double amplitude)
{
    auto graphNew = new TGraphErrors();

    for (auto iPoint=0; iPoint<fNumPoints; ++iPoint)
    {
        auto xValue = fGraphPulse -> GetPointX(iPoint);
        auto yValue = fGraphPulse -> GetPointY(iPoint);
        auto yError = fGraphPulse -> GetErrorY(iPoint);
        graphNew -> SetPoint(iPoint,xValue+tb,yValue*amplitude);
        graphNew -> SetPointError(iPoint,0,yError*amplitude);
        //cout << yError << endl;
    }
    graphNew -> SetLineColor(kRed);
    graphNew -> SetMarkerColor(kRed);
    return graphNew;
}
