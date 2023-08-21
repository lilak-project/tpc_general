#include "LKChannelSimulator.cpp"
#include "ejungwooA.h"

void drawChannel()
{
    gRandom -> SetSeed(time(0));

    auto sim = new LKChannelSimulator();
    sim -> SetPulse("dataExample/pulseReference_MMCenter1.root");
    sim -> SetYMax(4096);
    sim -> SetTbMax(350);
    sim -> SetNumSmoothing(2);
    sim -> SetSmoothingLength(2);
    sim -> SetPedestalFluctuationLength(5);
    sim -> SetPedestalFluctuationScale(0.4);
    sim -> SetPulseErrorScale(0.2);
    sim -> SetBackGroundLevel(500);

    int buffer[350] = {0};

    auto cvs = e_cvs("","",3000,2000,4,4);
    for (auto iSim=0; iSim<16; ++iSim)
    {
        memset(buffer, 0, sizeof(buffer));

        sim -> SetFluctuatingPedestal(buffer);
        sim -> AddHit(buffer,104,2077);

        auto hist = new TH1D(Form("histSimulation_%d",iSim),";tb;y",350,0,350);
        for (auto tb=0; tb<350; ++tb)
            hist -> SetBinContent(tb+1,buffer[tb]);

        hist -> SetMaximum(4096);
        hist -> SetMinimum(-200);

        cvs -> cd(iSim+1);
        hist -> Draw();
    }
}
