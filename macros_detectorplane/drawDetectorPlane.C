#include "TTMicromegas.cpp"

void drawDetectorPlane()
{
    auto run = new LKRun();
    run -> AddInputFile("/home/ejungwoo/data/texat/reco/texat_0801.all.root");
    auto tt = new TexAT2();
    auto mm = new TTMicromegas();
    //mm -> Init();
    //mm -> Draw();
    tt -> AddPlane(mm);
    run -> AddDetector(tt);
    //run -> AddPar("config.mac");
    run -> Add(new LKEveTask);
    run -> Init();
    //run -> RunEvent();
}
