/// This macro is macro for running LKPulseExtractionTask without compiling LKPulseExtractionTask
/// LKPulseExtractionTask.cpp, LKPulseExtractionTask.h and LKPulseExtractionTask.mac files should be placed in the same directory
#include "LKPulseExtractionTask.cpp"
#include "LKPulseExtractionTask.h"

void run_LKPulseExtractionTask()
{
    auto run = new LKRun();
    run -> AddDetector(new TexAT2());

    run -> AddInputFile(please_put_input_data);
    run -> AddPar("LKPulseExtractionTask.mac");
    run -> SetTag("lkpulseextractiontask");
    run -> Add(new LKPulseExtractionTask());

    run -> Init();
    run -> Run(0,10);
}
