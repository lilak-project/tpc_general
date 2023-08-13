#include "LKChannelAnalyzer.cpp"
#include "LKChannelAnalyzer.h"
//#include "LKPulse.cpp"
#include "LKPulse.h"
#include "buffer.h"

void test()
{
    auto pulse = new LKPulse("data/pulseReference_MMCenter.root");
    for (auto i=0; i<40; ++i) {
        cout << pulse -> Eval(i) << endl;
    }

    auto hist = new TH1D("hist","",350,0,350);
    for (auto i=0; i<350; ++i) {
        hist -> SetBinContent(i+1,buffer[i]);
    }

    auto cvs = new TCanvas();
    hist -> Draw();
    //pulse -> GetPulseGraph(34,2230) -> Draw("apl");
    e_add(cvs); 

    auto ana = new LKChannelAnalyzer();
    ana -> Init();
    double chi2 = 0;
    double amplitude;
    //for (double tb0=40; tb0<60; ++tb0)
    //for (double tb0=49; tb0<=51; ++tb0)
    for (double tb0 : {50})
    { 
        ana -> LeastSquareFitAtGivenTb(buffer, tb0, 30, amplitude, chi2);
        ana -> GetPulse() -> GetPulseGraph(tb0, amplitude) -> Draw("samel");
        cout << tb0 << " " << amplitude << " " << chi2 << endl;
    }
}
