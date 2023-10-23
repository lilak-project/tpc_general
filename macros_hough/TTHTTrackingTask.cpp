#include "TTHTTrackingTask.h"

ClassImp(TTHTTrackingTask);

TTHTTrackingTask::TTHTTrackingTask()
{
    fName = "TTHTTrackingTask";
}

bool TTHTTrackingTask::Init()
{
    // Put intialization todos here which are not iterative job though event
    lk_info << "Initializing TTHTTrackingTask" << std::endl;

    fNbinsXZPlane = fPar -> GetParDouble("TTHTTrackingTask/nbinsXZPlane");
    fNbinsZYPlane = fPar -> GetParDouble("TTHTTrackingTask/nbinsZYPlane");
    fNbinsXYPlane = fPar -> GetParDouble("TTHTTrackingTask/nbinsXYPlane");
    fNHitsTrackCutXZPlane = fPar -> GetParDouble("TTHTTrackingTask/nHitsTrackCutXZPlane");
    fNHitsTrackCutZYPlane = fPar -> GetParDouble("TTHTTrackingTask/nHitsTrackCutZYPlane");
    fNHitsTrackCutXYPlane = fPar -> GetParDouble("TTHTTrackingTask/nHitsTrackCutXYPlane");

    fHitArray[0][0] = nullptr;
    fHitArray[0][1] = nullptr;
    fHitArray[1][0] = nullptr;
    fHitArray[1][1] = nullptr;
    fHitArray[2][0] = nullptr;
    fHitArray[2][1] = nullptr;
    fHitArray[0][0] = fRun -> GetBranchA("HitCenter");
    fHitArray[0][1] = fRun -> GetBranchA("HitOthers");
    fHitArray[1][0] = fRun -> GetBranchA("HitLStrip");
    fHitArray[1][1] = fRun -> GetBranchA("HitLChain");
    fHitArray[2][0] = fRun -> GetBranchA("HitRStrip");
    fHitArray[2][1] = fRun -> GetBranchA("HitRChain");
    fTrackArray = fRun -> GetBranchA("Track");
    fEventHeader = (TTEventHeader*) fRun -> KeepBranch("EventHeader");

    auto SetTracker = [](LKHTLineTracker* tk, double tx, double ty, int nx, double x1, double x2, int ny, double y1, double y2, int nr, int nt);
        tk -> SetTransformCenter(tx, ty);
        tk -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
        tk -> SetParamSpaceBins(nr, nt);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();
    };

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight}) {
        for (auto iSCA : {kStrip,kChain,kStripAndChain}) {
            fTrackerXY[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerXY[iCLR][iSCA], fXTransformCenter, fYTransformCenter, fNX, fX1, fX2, fNY, fY1, fY2, fNR, fNT);
            fTrackerZY[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerZY[iCLR][iSCA], fXTransformCenter, fYTransformCenter, fNZ, fZ1, fZ2, fNY, fY1, fY2, fNR, fNT);
            fTrackerXZ[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerXZ[iCLR][iSCA], fXTransformCenter, fYTransformCenter, fNX, fX1, fX2, fNZ, fZ1, fZ2, fNR, fNT);
        }
    }

    return true;
}

void TTHTTrackingTask::Exec(Option_t *option)
{

    // Construct (new) hitArray00_single from fHitArray00 and set data value
    int numHitArray00_single = fHitArray00 -> GetEntriesFast();
    for (int iHitArray00_single = 0; iHitArray00_single < numHitArray00_single; ++iHitArray00_single)
    {
        auto hitArray00_single = (LKHit *) fHitArray00 -> At(iHitArray00_single);
        // hitArray00_single -> SetData(value);
    }

    // Construct (new) hitArray01_single from fHitArray01 and set data value
    int numHitArray01_single = fHitArray01 -> GetEntriesFast();
    for (int iHitArray01_single = 0; iHitArray01_single < numHitArray01_single; ++iHitArray01_single)
    {
        auto hitArray01_single = (LKHit *) fHitArray01 -> At(iHitArray01_single);
        // hitArray01_single -> SetData(value);
    }

    // Construct (new) hitArray10_single from fHitArray10 and set data value
    int numHitArray10_single = fHitArray10 -> GetEntriesFast();
    for (int iHitArray10_single = 0; iHitArray10_single < numHitArray10_single; ++iHitArray10_single)
    {
        auto hitArray10_single = (LKHit *) fHitArray10 -> At(iHitArray10_single);
        // hitArray10_single -> SetData(value);
    }

    // Construct (new) hitArray11_single from fHitArray11 and set data value
    int numHitArray11_single = fHitArray11 -> GetEntriesFast();
    for (int iHitArray11_single = 0; iHitArray11_single < numHitArray11_single; ++iHitArray11_single)
    {
        auto hitArray11_single = (LKHit *) fHitArray11 -> At(iHitArray11_single);
        // hitArray11_single -> SetData(value);
    }

    // Construct (new) hitArray20_single from fHitArray20 and set data value
    int numHitArray20_single = fHitArray20 -> GetEntriesFast();
    for (int iHitArray20_single = 0; iHitArray20_single < numHitArray20_single; ++iHitArray20_single)
    {
        auto hitArray20_single = (LKHit *) fHitArray20 -> At(iHitArray20_single);
        // hitArray20_single -> SetData(value);
    }

    // Construct (new) hitArray21_single from fHitArray21 and set data value
    int numHitArray21_single = fHitArray21 -> GetEntriesFast();
    for (int iHitArray21_single = 0; iHitArray21_single < numHitArray21_single; ++iHitArray21_single)
    {
        auto hitArray21_single = (LKHit *) fHitArray21 -> At(iHitArray21_single);
        // hitArray21_single -> SetData(value);
    }

    // Construct (new) track from fTrackArray and set data value
    int numTrack = fTrackArray -> GetEntriesFast();
    for (int iTrack = 0; iTrack < numTrack; ++iTrack)
    {
        auto track = (LKLinearTrack *) fTrackArray -> At(iTrack);
        // track -> SetData(value);
    }
    //fEventHeader -> SetData(value);
    //auto data = fEventHeader -> GetData(value);
    lk_info << "TTHTTrackingTask" << std::endl;
}

bool TTHTTrackingTask::EndOfRun()
{
    return true;
}

