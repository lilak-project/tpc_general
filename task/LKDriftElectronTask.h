#ifndef LKDRIFTELECTRONTASK_HH
#define LKDRIFTELECTRONTASK_HH

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

class LKDriftElectronTask : public LKTask
{
    public:
        LKDriftElectronTask();
        virtual ~LKDriftElectronTask() { ; }

        bool Init();
        void Exec(Option_t *option="");
        bool EndOfRun();

        int fNumTPCs;
        int fMCStepArray;

        TClonesArray* fMCStepArray[4];
        TClonesArray* fPadArray[4]
        LKPadPlane* fPadPlane[4];
        LKElectronDriftTool* fElectronDriftTool[4];
        

    ClassDef(LKDriftElectronTask,1);
};

#endif
