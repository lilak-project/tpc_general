#ifndef TTHTTRACKINGTASK_HH
#define TTHTTRACKINGTASK_HH

#include "TClonesArray.h"
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

        auto TransformAndFit(int iView, int iCLR, LKHTLineTracker* tk[3][3]);

    private:
        //TClonesArray *fHitArray[3][2];
        TClonesArray *fHitArray[6];
        TClonesArray *fTrackArray = nullptr;
        TClonesArray* fEventHeaderHolder = nullptr;

        int    fNX = 143;
        double fX1 = -120;
        double fX2 = 120;
        int    fNY = 110;
        double fY1 = 0;
        double fY2 = 330;
        int    fNZ = 141;
        double fZ1 = 150;
        double fZ2 = 500;
        int    fNR[3] = {80, 80, 80};
        int    fNT[3] = {100,100,100};

        double fTCXInXY[3] = {fX1,fX1,fX1};
        double fTCYInXY[3] = {fY1,fY1,fY1};
        double fTCXInZY[3] = {fZ1,fZ1,fZ1};
        double fTCYInZY[3] = {fY1,fY1,fY1};
        double fTCXInXZ[3] = {fX1,fX1,fX1};
        double fTCYInXZ[3] = {fZ1,fZ1,fZ1};

        LKHTLineTracker*     fTracker[2][2];
        //LKHTLineTracker*     fTrackerXZ[3][3];
        //LKHTLineTracker*     fTrackerZY[3][3];
        //LKHTLineTracker*     fTrackerXY[3][3];

        LKImagePoint* fImagePointXY = nullptr;
        LKImagePoint* fImagePointZY = nullptr;

        //LKHitArray* fHitArrayLeft = nullptr;
        //LKHitArray* fHitArrayRight = nullptr;

        const int    kCenter = 0;
        const int    kLStrip = 1;
        const int    kLChain = 2;
        const int    kRStrip = 3;
        const int    kRChain = 4;
        const int    kOthers = 5;

        const int    kStripAndChain = 2;
        const int    kStrip = 0;
        const int    kChain = 1;
        const int    kMMCenter = 0;
        const int    kMMLeft = 1;
        const int    kMMRight = 2;
        const int    kFrontView = 0;
        const int    kSideView = 1;
        const int    kTopView = 2;

        double       fHTMaxCutXZPlane = 5;
        double       fHTMaxCutZYPlane = 5;
        double       fHTMaxCutXYPlane = 5;
        double       fNHitsTrackCutXZPlane = 3;
        double       fNHitsTrackCutZYPlane = 3;
        double       fNHitsTrackCutXYPlane = 3;

    ClassDef(TTHTTrackingTask,1);
};

#endif
