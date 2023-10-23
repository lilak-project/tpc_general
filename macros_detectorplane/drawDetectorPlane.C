#include "TTMicromegas.cpp"

void drawDetectorPlane()
{
    //auto run = new LKRun();
    //auto tt = new TexAT2();
    auto mm = new TTMicromegas();
    mm -> Init();
    mm -> Draw();
    //tt -> AddPlane(mm);
    //run -> AddDetector(tt);
    //run -> AddPar("config.mac");
    //run -> Init();
}
