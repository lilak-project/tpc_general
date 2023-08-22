#include "LKPulse.cpp"
//#include "LKPulse.h"

void drawPulse()
{
    auto pulse = new LKPulse("data/pulseReference_MMCenter1.root");
    pulse -> GetPulseGraph(0,100) -> Draw("apl");
}
