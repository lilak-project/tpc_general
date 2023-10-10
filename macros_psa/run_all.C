#include "LKLogger.h"

void run_all(int runID=0, int splitID=0, const char *inputFile="/home/ejungwoo/data/texat/run_0824.dat.19-03-23_23h42m36s.38.root")
{
    //lk_logger("log_runall.log");

    auto run = new LKRun();
    run -> AddPar("config.mac");
    run -> AddDetector(new TexAT2());

    run -> SetDataPath("data");
    run -> SetOutputFile(Form("texat%d.%d.root",runID,splitID));

    run -> SetTag("all");
    auto conv = new TTRootConversionTask();
    conv -> SetInputFileName(inputFile);
    run -> Add(conv);
    run -> Add(new TTEventSelectionTask());
    run -> Add(new TTGetPedestalTask());
    run -> Add(new TTBeamSelectionTask());
    //run -> Add(new TTDrawRawWaveformTask());

    run -> Init();
    run -> Run();
}
