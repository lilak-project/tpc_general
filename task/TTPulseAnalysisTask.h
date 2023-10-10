#ifndef TTPULSEANALYSISTASK_HH
#define TTPULSEANALYSISTASK_HH

#include "TClonesArray.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"
#include "LKRun.h"
#include "LKTask.h"
#include "TTEventHeader.h"
#include "TexAT2.h"
//#include "LKHit.h"
//#include "MMChannel.h"
//#include "TTEventHeader.h"

/*
 * Remove this comment block after reading it through
 * or use print_example_comments=False option to omit printing
 *
 * # Example LILAK task class
 *
 * - Write Init() method.
 * - Write Exec() or/and EndOfRun() method.
 */

class TTPulseAnalysisTask : public LKTask
{
    public:
        TTPulseAnalysisTask();
        virtual ~TTPulseAnalysisTask() { ; }

        bool Init();
        void Exec(Option_t *option="");
        bool EndOfRun();

    private:
        TClonesArray *fChannelArray = nullptr;
        TClonesArray *fHitArrayCenter = nullptr;
        TClonesArray *fHitArrayLStrip = nullptr;
        TClonesArray *fHitArrayLChain = nullptr;
        TClonesArray *fHitArrayRStrip = nullptr;
        TClonesArray *fHitArrayRChain = nullptr;
        TClonesArray *fHitArrayOthers = nullptr;

        TTEventHeader* fEventHeader = nullptr;

        TexAT2 *fDetector;

        int fITypeLStrip;
        int fITypeRStrip;
        int fITypeLChain;
        int fITypeRChain;
        int fITypeLCenter;
        int fITypeHCenter;

        int fCountHitCenter = 0;
        int fCountHitLStrip = 0;
        int fCountHitLChain = 0;
        int fCountHitRStrip = 0;
        int fCountHitRChain = 0;
        int fCountHitOthers = 0;

    ClassDef(TTPulseAnalysisTask,1);
};

#endif
