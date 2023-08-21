#include "LKPulse.cpp"
//#include "LKPulse.h"

void drawPulse()
{
    auto pulse = new LKPulse("data100/pulseReference_MMCenter1.root");
    pulse -> GetPulseGraph(10,100) -> Draw("apl");
}
