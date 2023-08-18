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

    fMultiplicity   = ((TParameter<int>*) file -> Get("multiplicity")) -> GetVal();
    fThreshold      = ((TParameter<int>*) file -> Get("threshold")) -> GetVal();
    fHeightMin      = ((TParameter<int>*) file -> Get("yMin")) -> GetVal();
    fHeightMax      = ((TParameter<int>*) file -> Get("yMax")) -> GetVal();
    fTbMin          = ((TParameter<int>*) file -> Get("xMin")) -> GetVal();
    fTbMax          = ((TParameter<int>*) file -> Get("xMax")) -> GetVal();
    fFWHM           = ((TParameter<double>*) file -> Get("FWHM")) -> GetVal();
    fFloorRatio     = ((TParameter<double>*) file -> Get("ratio")) -> GetVal();
    fRefWidth       = ((TParameter<double>*) file -> Get("width")) -> GetVal();
    fWidthLeading   = ((TParameter<double>*) file -> Get("widthLeading")) -> GetVal();
    fWidthTrailing  = ((TParameter<double>*) file -> Get("widthTrailing")) -> GetVal();

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

double LKPulse::Eval(double tb, double tb0, double amplitude)
{
    return amplitude * fGraphPulse -> Eval(tb-tb0);
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
    }
    graphNew -> SetLineColor(kRed);
    graphNew -> SetMarkerColor(kRed);
    return graphNew;
}
