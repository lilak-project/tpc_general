#include "LKPulseExtractionTask.h"

ClassImp(LKPulseExtractionTask);

LKPulseExtractionTask::LKPulseExtractionTask()
{
    fName = "LKPulseExtractionTask";
}

bool LKPulseExtractionTask::Init()
{
    // Put intialization todos here which are not iterative job though event
    lk_info << "Initializing LKPulseExtractionTask" << std::endl;

    return true;
}

void LKPulseExtractionTask::Exec(Option_t *option)
{
    lk_info << "LKPulseExtractionTask" << std::endl;
}

bool LKPulseExtractionTask::EndOfRun()
{
    return true;
}

