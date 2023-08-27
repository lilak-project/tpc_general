#include "ejungwooA.h"
#include "LKPulse.cpp"
//#include "LKPulse.h"

void drawPulse()
{
    auto pulse = new LKPulse("data/pulseReference_MMCenter1.root");
    e_cvs("cvs");
    pulse -> GetPulseGraph(0,100) -> Draw("apl");
}
