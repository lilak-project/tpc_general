/// This macro is macro for running LKElectronicsTask without compiling LKElectronicsTask
/// LKElectronicsTask.cpp, LKElectronicsTask.h and LKElectronicsTask.mac files should be placed in the same directory
#include "LKElectronicsTask.cpp"
#include "LKElectronicsTask.h"

void run_LKElectronicsTask()
{
    auto run = new LKRun();
    run -> AddDetector(new TexAT2());

    run -> AddInputFile(please_put_input_data);
    run -> AddPar("LKElectronicsTask.mac");
    run -> SetTag("lkelectronicstask");
    run -> Add(new LKElectronicsTask());

    run -> Init();
    run -> Run(0,10);
}
