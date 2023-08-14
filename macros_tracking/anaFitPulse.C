#include "LKChannelAnalyzer.cpp"
#include "LKChannelAnalyzer.h"
//#include "LKPulse.cpp"
#include "LKPulse.h"

void anaFitPulse()
{
    auto pulse = new LKPulse("data/pulseReference_MMCenter.root");

    double buffer[350] = {0};
    ifstream file_buffer("data/buffer_MMLeftCenter_22002.dat");
    int iTb = 0;
    double value;
    while (file_buffer>>value) {
        buffer[iTb] = value;
        ++iTb;
    }

    auto hist = new TH1D("hist","",350,0,350);
    for (auto i=0; i<350; ++i)
        hist -> SetBinContent(i+1,buffer[i]);
    auto cvs = e_cvs();
    hist -> Draw();

    auto ana = new LKChannelAnalyzer();
    ana -> Init();
    double chi2 = 0;
    double amplitude;

    //for (double tb0=40; tb0<60; ++tb0)
    for (double tb0=49; tb0<=51; ++tb0)
    //for (double tb0 : {50})
    { 
        ana -> LeastSquareFitAtGivenTb(buffer, tb0, 30, amplitude, chi2);
        ana -> GetPulse() -> GetPulseGraph(tb0, amplitude) -> Draw("samel");
        cout << tb0 << " " << amplitude << " " << chi2 << endl;
    }
}
