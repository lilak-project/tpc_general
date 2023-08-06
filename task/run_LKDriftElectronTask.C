/// This macro is macro for running LKDriftElectronTask without compiling LKDriftElectronTask
/// LKDriftElectronTask.cpp, LKDriftElectronTask.h and LKDriftElectronTask.mac files should be placed in the same directory
#include "LKDriftElectronTask.cpp"
#include "LKDriftElectronTask.h"

void run_LKDriftElectronTask()
{
    auto run = new LKRun();
    run -> AddDetector(new TexAT2());

    run -> AddInputFile(please_put_input_data);
    run -> AddPar("LKDriftElectronTask.mac");
    run -> SetTag("lkdriftelectrontask");
    run -> Add(new LKDriftElectronTask());

    run -> Init();
    run -> Run(0,10);
}
