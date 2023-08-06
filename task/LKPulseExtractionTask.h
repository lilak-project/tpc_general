#ifndef LKPULSEEXTRACTIONTASK_HH
#define LKPULSEEXTRACTIONTASK_HH

#include "TClonesArray.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"
#include "LKRun.h"
#include "LKTask.h"

/*
 * Remove this comment block after reading it through
 * or use print_example_comments=False option to omit printing
 *
 * # Example LILAK task class
 *
 * - Write Init() method.
 * - Write Exec() or/and EndOfRun() method.
 */

class LKPulseExtractionTask : public LKTask
{
    public:
        LKPulseExtractionTask();
        virtual ~LKPulseExtractionTask() { ; }

        bool Init();
        void Exec(Option_t *option="");
        bool EndOfRun();

    ClassDef(LKPulseExtractionTask,1);
};

#endif
