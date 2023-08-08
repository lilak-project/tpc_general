#include "LKLogger.h"

void run_all()
{
    //lk_logger("log_runall.log");

    auto run = new LKRun();
    run -> AddPar("config.mac");
    run -> AddDetector(new TexAT2());

    run -> SetTag("all");
    run -> Add(new TTRootConversionTask());
    run -> Add(new TTEventSelectionTask());
    run -> Add(new TTGetPedestalTask());
    run -> Add(new TTBeamSelectionTask());
    //run -> Add(new TTDrawRawWaveformTask());

    run -> Init();
    //run -> Run(0,100);
    run -> Run();
}
