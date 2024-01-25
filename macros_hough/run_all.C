#include "LKLogger.h"

void run_all()
{
    auto run = new LKRun();//"texat",801,"ht");
    run -> SetTag("ht");
    run -> AddInputFile("/home/ejungwoo/data/texat/reco/texat_0801.all.root");
    run -> AddPar("config_ht.mac");
    run -> AddDetector(new TexAT2());
    run -> Add(new TTHTTrackingTask());

    run -> Init();
    run -> Run();
}
