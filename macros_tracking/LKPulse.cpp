#include "LKPulse.h"

ClassImp(LKPulse);

LKPulse::LKPulse()
{
    ;
}

LKPulse::LKPulse(const char *fileName)
{
    auto file = new TFile(fileName);

    fGraphPulse = (TGraphErrors *) file -> Get("pulse");
    fGraphError = (TGraph*) file -> Get("error");
    fGraphError0 = (TGraph*) file -> Get("error0");
    fNumPoints = fGraphPulse -> GetN();

    fNumAnalyzedChannels = ((TParameter<int>*) file -> Get("numAnaChannels")) -> GetVal();
    fThreshold           = ((TParameter<int>*) file -> Get("threshold")) -> GetVal();
    fHeightMin           = ((TParameter<int>*) file -> Get("yMin")) -> GetVal();
    fHeightMax           = ((TParameter<int>*) file -> Get("yMax")) -> GetVal();
    fTbMin               = ((TParameter<int>*) file -> Get("xMin")) -> GetVal();
    fTbMax               = ((TParameter<int>*) file -> Get("xMax")) -> GetVal();
    fFWHM                = ((TParameter<double>*) file -> Get("FWHM")) -> GetVal();
    fFloorRatio          = ((TParameter<double>*) file -> Get("ratio")) -> GetVal();
    fRefWidth            = ((TParameter<double>*) file -> Get("width")) -> GetVal();
    fWidthLeading        = ((TParameter<double>*) file -> Get("widthLeading")) -> GetVal();
    fWidthTrailing       = ((TParameter<double>*) file -> Get("widthTrailing")) -> GetVal();
    fPulseRefTbMin       = ((TParameter<int>*) file -> Get("pulseRefTbMin")) -> GetVal();
    fPulseRefTbMax       = ((TParameter<int>*) file -> Get("pulseRefTbMax")) -> GetVal();
    fBackGroundLevel     = ((TParameter<double>*) file -> Get("backGroundLevel")) -> GetVal();
    fBackGroundError     = ((TParameter<double>*) file -> Get("backGroundError")) -> GetVal();
    fFluctuationLevel    = ((TParameter<double>*) file -> Get("fluctuationLevel")) -> GetVal();

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
    e_info << "fNumAnalyzedChannels : " << fNumAnalyzedChannels << endl;
    e_info << "fThreshold           : " << fThreshold           << endl;
    e_info << "fHeightMin           : " << fHeightMin           << endl;
    e_info << "fHeightMax           : " << fHeightMax           << endl;
    e_info << "fTbMin               : " << fTbMin               << endl;
    e_info << "fTbMax               : " << fTbMax               << endl;
    e_info << "fFWHM                : " << fFWHM                << endl;
    e_info << "fFloorRatio          : " << fFloorRatio          << endl;
    e_info << "fRefWidth            : " << fRefWidth            << endl;
    e_info << "fWidthLeading        : " << fWidthLeading        << endl;
    e_info << "fWidthTrailing       : " << fWidthTrailing       << endl;
    e_info << "fBackGroundLevel     : " << fBackGroundLevel     << endl;
    e_info << "fBackGroundError     : " << fBackGroundError     << endl;
}

double LKPulse::Eval  (double tb, double tb0, double amplitude) { return (amplitude * fGraphPulse  -> Eval(tb-tb0)); }
double LKPulse::Error (double tb, double tb0, double amplitude) { return (amplitude * fGraphError  -> Eval(tb-tb0)); }
double LKPulse::Error0(double tb, double tb0, double amplitude) { return (amplitude * fGraphError0 -> Eval(tb-tb0)); }

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
