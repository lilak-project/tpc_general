#ifndef TTHTTRACKINGTASK_HH
#define TTHTTRACKINGTASK_HH

#include "TClonesArray.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"
#include "LKRun.h"
#include "LKTask.h"
//#include "LKHit.h"
//#include "LKLinearTrack.h"
//#include "TTEventHeader.h"
#include "EventHeader"

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

        double GetNbinsXZPlane[2]() const  { return fNbinsXZPlane; }
        double GetNbinsZYPlane[2]() const  { return fNbinsZYPlane; }
        double GetNbinsXYPlane[2]() const  { return fNbinsXYPlane; }
        double GetNHitsTrackCutXZPlane() const  { return fNHitsTrackCutXZPlane; }
        double GetNHitsTrackCutZYPlane() const  { return fNHitsTrackCutZYPlane; }
        double GetNHitsTrackCutXYPlane() const  { return fNHitsTrackCutXYPlane; }

        void SetNbinsXZPlane[2](double nbinsXZPlane) { fNbinsXZPlane = nbinsXZPlane; }
        void SetNbinsZYPlane[2](double nbinsZYPlane) { fNbinsZYPlane = nbinsZYPlane; }
        void SetNbinsXYPlane[2](double nbinsXYPlane) { fNbinsXYPlane = nbinsXYPlane; }
        void SetNHitsTrackCutXZPlane(double nHitsTrackCutXZPlane) { fNHitsTrackCutXZPlane = nHitsTrackCutXZPlane; }
        void SetNHitsTrackCutZYPlane(double nHitsTrackCutZYPlane) { fNHitsTrackCutZYPlane = nHitsTrackCutZYPlane; }
        void SetNHitsTrackCutXYPlane(double nHitsTrackCutXYPlane) { fNHitsTrackCutXYPlane = nHitsTrackCutXYPlane; }

    private:

        TClonesArray *fHitArray00 = nullptr;
        TClonesArray *fHitArray01 = nullptr;
        TClonesArray *fHitArray10 = nullptr;
        TClonesArray *fHitArray11 = nullptr;
        TClonesArray *fHitArray20 = nullptr;
        TClonesArray *fHitArray21 = nullptr;
        TClonesArray *fTrackArray = nullptr;
        TTEventHeader* fEventHeader = nullptr;

        int fNX = 143;
        double fX1 = -120;
        double fX2 = 120;

        int fNY = 110;
        double fY1 = 0;
        double fY2 = 330;

        int fNZ = 141;
        double fZ1 = 150;
        double fZ2 = 500;

        int fNR = 80;
        int fNT = 100;

        LKHTLineTracker*     fTrackerXZ[3][3];
        LKHTLineTracker*     fTrackerZY[3][3];
        LKHTLineTracker*     fTrackerXY[3][3];
        const int    fkStripAndChain = 2;
        const int    fkStrip = 0;
        const int    fkChain = 1;
        const int    fkMMCenter = 0;
        const int    fkMMLeft = 1;
        const int    fkMMRight = 2;
        const int    fkFrontView = 0;
        const int    fkSideView = 1;
        const int    fkTopView = 2;
        double       fNbinsXZPlane[2];
        double       fNbinsZYPlane[2];
        double       fNbinsXYPlane[2];
        double       fNHitsTrackCutXZPlane;
        double       fNHitsTrackCutZYPlane;
        double       fNHitsTrackCutXYPlane;

    ClassDef(TTHTTrackingTask,1);
};

#endif
