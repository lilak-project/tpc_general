#include "ejungwooA.h"
#include "LKPulse.cpp"
//#include "LKPulse.h"

void drawPulse()
{
    auto pulse = new LKPulse("data100/pulseReference_MMCenterCenterA0.root");
    e_cvs("cvs");
    pulse -> GetPulseGraph(0,100) -> Draw("apl");
}
