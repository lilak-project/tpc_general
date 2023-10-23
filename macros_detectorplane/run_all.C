#include "LKLogger.h"

void run_all()
{
    lk_logger("data/run_all.log");

    auto run = new LKRun();
    run -> SetNumPrintMessage(50);
    run -> AddPar("config.mac");
    run -> AddDetector(new TexAT2());
    run -> Add(new TTRootConversionTask());
    //run -> Add(new TTEventNumberSelectionTask("list_eventNo_801.txt"));
    run -> Add(new TTPulseAnalysisTask());

    run -> Init();
    run -> Run();
}
