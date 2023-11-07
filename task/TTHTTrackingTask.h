#ifndef TTHTTRACKINGTASK_HH
#define TTHTTRACKINGTASK_HH

#include "TClonesArray.h"
#include "TObjArray.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"
#include "LKRun.h"
#include "LKTask.h"
#include "LKHit.h"
#include "LKLinearTrack.h"
#include "LKHTLineTracker.h"
#include "TTEventHeader.h"
//#include "EventHeader"

/*
 * Remove this comment block after reading it through
 * or use print_example_comments=False option to omit printing
 *
 * # Example LILAK task class
 *
 * - Write Init() method.
 * - Write Exec() or/and EndOfRun() method.
 */

class TTHTTrackingTask : public LKTask
{
    public:
        TTHTTrackingTask();
        virtual ~TTHTTrackingTask() { ; }

        bool Init();
        void Exec(Option_t *option="");
        bool EndOfRun();

        bool TransformAndSelectHits(LKHTLineTracker* trackerXY, LKHTLineTracker* trackerZY);
        bool MakeTrack(LKLinearTrack* trackXY, LKLinearTrack* trackZY, double x1, double x2);

        LKHTLineTracker* GetTracker(int i, int j) { return fTracker[i][j]; }

    private:
        //TClonesArray *fHitArray[3][2];
        TClonesArray *fHitArray[6];
        TClonesArray *fTrackArray = nullptr;
        TClonesArray* fEventHeaderHolder = nullptr;

        TObjArray* fCrossHitCollection = nullptr;

        LKHTLineTracker* fTracker[2][2];

        LKLinearTrack* fTrackXY = nullptr;
        LKLinearTrack* fTrackZY = nullptr;

        int          fNumTracks = 0;

        int          fNX = 143;
        double       fX1 = -120;
        double       fX2 = 120;
        int          fNY = 110;
        double       fY1 = 0;
        double       fY2 = 330;
        int          fNZ = 141;
        double       fZ1 = 150;
        double       fZ2 = 500;
        int          fNR = 80;
        int          fNT = 100;

        const int    kCenter = 0;
        const int    kLStrip = 1;
        const int    kLChain = 2;
        const int    kRStrip = 3;
        const int    kRChain = 4;
        const int    kOthers = 5;

        const int    kViewXY = 0;
        const int    kViewZY = 1;
        const int    kViewXZ = 2;

        const int    kStrip = 0;
        const int    kChain = 1;
        //const int    kStripAndChain = 2;

        const int    kLeft = 0;
        const int    kRight = 1;
        //const int    kCenter = 2;

        int          fNumHitsCutForTransform = 3;

        /* par-name: TTHTTrackingTask/use_transform_with_chain_strip_combination
         * If fUseTransformCSCombination is true, both chain- and strip-hits will be used for filling parameter space
         * while fitting will only be done using chain-hits(strip-hits) for XY-plane(ZY-plane).
         * If fUseTransformCSCombination is false, both only chain-hits(strip-hits) will used for filling parameter space of XY-plane(ZY-plane).
         */
        bool         fUseTransformCSCombination = true;

    ClassDef(TTHTTrackingTask,1);
};

#endif
