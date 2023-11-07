#include "TTHTTrackingTask.h"

ClassImp(TTHTTrackingTask);

TTHTTrackingTask::TTHTTrackingTask()
{
    fName = "TTHTTrackingTask";
}

bool TTHTTrackingTask::Init()
{
    lk_info << "Initializing TTHTTrackingTask" << std::endl;

    if (fPar->CheckPar("TTHTTrackingTask/use_transform_with_chain_strip_combination"))
        fUseTransformCSCombination = fPar -> GetParBool("TTHTTrackingTask/use_transform_with_chain_strip_combination");

    if (fUseTransformCSCombination) lk_info << "Flag, HT transform using Chain Strip combination is ON!" << endl;
    else lk_info << "Flag, HT transform using Chain Strip combination is OFF!" << endl;

    fHitArray[kCenter] = nullptr;
    fHitArray[kLStrip] = nullptr;
    fHitArray[kLChain] = nullptr;
    fHitArray[kRStrip] = nullptr;
    fHitArray[kRChain] = nullptr;
    fHitArray[kOthers] = nullptr;

    fHitArray[kCenter] = fRun -> GetBranchA("HitCenter");
    fHitArray[kLStrip] = fRun -> GetBranchA("HitLStrip");
    fHitArray[kLChain] = fRun -> GetBranchA("HitLChain");
    fHitArray[kRStrip] = fRun -> GetBranchA("HitRStrip");
    fHitArray[kRChain] = fRun -> GetBranchA("HitRChain");
    fHitArray[kOthers] = fRun -> GetBranchA("HitOthers");
    fEventHeaderHolder = fRun -> KeepBranchA("EventHeader");

    fTrackArray = fRun -> RegisterBranchA("Track","LKLinearTrack",100);

    auto SetTracker = [](LKHTLineTracker* tk, double tx, double ty, int nx, double x1, double x2, int ny, double y1, double y2, int nr, int nt) {
        tk -> SetTransformCenter(tx, ty);
        tk -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
        tk -> SetParamSpaceBins(nr, nt);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();
    };

    fTracker[kViewXY][kLeft] = new LKHTLineTracker();  SetTracker(fTracker[kViewXY][kLeft],  fX1, fY1, fNX, fX1, fX2, fNY, fY1, fY2, fNR, fNT);
    fTracker[kViewZY][kLeft] = new LKHTLineTracker();  SetTracker(fTracker[kViewZY][kLeft],  fZ1, fY1, fNZ, fZ1, fZ2, fNY, fY1, fY2, fNR, fNT);
    fTracker[kViewXY][kRight] = new LKHTLineTracker(); SetTracker(fTracker[kViewXY][kRight], fX1, fY1, fNX, fX1, fX2, fNY, fY1, fY2, fNR, fNT);
    fTracker[kViewZY][kRight] = new LKHTLineTracker(); SetTracker(fTracker[kViewZY][kRight], fZ1, fY1, fNZ, fZ1, fZ2, fNY, fY1, fY2, fNR, fNT);

    fCrossHitCollection = new TObjArray();

    return true;
}

bool TTHTTrackingTask::TransformAndSelectHits(LKHTLineTracker* trackerXY, LKHTLineTracker* trackerZY)
{
    trackerXY -> Transform();
    trackerZY -> Transform();
    auto paramPointXY = trackerXY -> FindNextMaximumParamPoint();
    auto paramPointZY = trackerZY -> FindNextMaximumParamPoint();

    if (fUseTransformCSCombination)
    {
        trackerXY -> SelectPoints(paramPointXY);
        trackerZY -> SelectPoints(paramPointZY);
        auto hitArrayXY = trackerXY -> GetSelectedHitArray();
        auto hitArrayZY = trackerZY -> GetSelectedHitArray();
        TIter nextXY(hitArrayXY);
        TIter nextZY(hitArrayZY);

        LKHit *hit1, *hit2, *hit;

        auto numCrossHits = 0;
        while ((hit1 = (LKHit*) nextXY())) {
            nextZY.Reset();
            while ((hit2 = (LKHit*) nextZY())) {
                if (hit1->GetHitID()==hit2->GetHitID()) {
                    fCrossHitCollection -> Add(hit1);
                    ++numCrossHits;
                }
            }
        }
        if (numCrossHits<=fNumHitsCutForTransform)
            return false;

        trackerXY -> ClearPoints();
        trackerZY -> ClearPoints();
        TIter nextCross(fCrossHitCollection);
        while ((hit = (LKHit*) nextCross())) {
            if (hit->GetSortValue()>0) trackerXY -> AddHit(hit,LKVector3::kX,LKVector3::kY);
            if (hit->GetSortValue()<0) trackerZY -> AddHit(hit,LKVector3::kZ,LKVector3::kY);
        }
    }

    fTrackXY = trackerXY -> FitTrackWithParamPoint(paramPointXY);
    fTrackZY = trackerZY -> FitTrackWithParamPoint(paramPointZY);

    return true;
}

bool TTHTTrackingTask::MakeTrack(LKLinearTrack* trackXY, LKLinearTrack* trackZY, double x1, double x2)
{
    if (trackXY!=nullptr && trackZY!=nullptr)
    {
        auto point1 = trackXY -> GetPoint1();
        auto point2 = trackZY -> GetPoint2();
        point1.SetXYZ(point1.X(), point1.Y(), 0);
        point2.SetXYZ(0, point2.Y(), point2.X());
        auto directionXY = trackXY -> Direction();
        auto directionZY = trackZY -> Direction();
        TVector3 normal1(-directionXY.Y(), directionXY.X(), 0);
        TVector3 normal2(0, directionZY.X(), -directionZY.Y());
        LKGeoPlaneWithCenter plane1(point1, normal1);
        LKGeoPlaneWithCenter plane2(point2, normal2);
        LKGeoLine lineL = plane1.GetCrossSectionLine(plane2);
        auto vertex1 = lineL.GetPointAtX(x1);
        auto vertex2 = lineL.GetPointAtX(x2);
        auto track = (LKLinearTrack*) fTrackArray -> ConstructedAt(fNumTracks++);
        track -> SetTrack(vertex1, vertex2);
        track -> SetTrackID(fNumTracks);
        LKHit* hit;
        if (fUseTransformCSCombination) {
            TIter nextXY(trackXY->GetHitArray());
            while ((hit = (LKHit*)nextXY())) track -> AddHit(hit);
        }
        else {
            TIter nextXY(trackXY->GetHitArray());
            while ((hit = (LKHit*)nextXY())) track -> AddHit(hit);
            TIter nextZY(trackZY->GetHitArray());
            while ((hit = (LKHit*)nextZY())) track -> AddHit(hit);
        }
        return true;
    }
    return false;
}

void TTHTTrackingTask::Exec(Option_t *option)
{
    fTrackArray -> Clear("C");
    fNumTracks = 0;

    auto numHitsAll = 0;
    for (auto iRegion : {kLStrip, kLChain, kRStrip, kRChain})
        numHitsAll += fHitArray[iRegion] -> GetEntries();

    // good event?  ------------------------------------------------------------------------
    auto eventHeader = (TTEventHeader*) fEventHeaderHolder -> At(0);
    auto goodEvent = eventHeader -> IsGoodEvent();
    if (numHitsAll==0||goodEvent==false) {
        eventHeader -> SetIsGoodEvent(false);
        lk_info << "Bad event. Skip" << endl;
        return;
    }

    // clear ------------------------------------------------------------------------
    for (auto iView : {kViewXY,kViewZY})
        for (auto iLeftRight : {kLeft,kRight})
            fTracker[iView][iLeftRight] -> Clear();

    // add transform and fit ------------------------------------------------------------------------
    for (auto iLeftRight : {kLeft, kRight})
    {
        double x1 = 0;
        double x2 = fX1;
        int iStrip = kLStrip;
        int iChain = kLChain;
        if (iLeftRight==kRight) {
            x2 = fX2;
            iStrip = kRStrip;
            iChain = kRChain;
        }
        if (fHitArray[iStrip]->GetEntries()>fNumHitsCutForTransform && fHitArray[iChain]->GetEntries()>fNumHitsCutForTransform)
        {
            for (auto iRegion : {iStrip ,iChain}) {
                auto numHits = fHitArray[iRegion] -> GetEntries();
                for (auto iHit=0; iHit<numHits; ++iHit) {
                    auto hit = (LKHit*) fHitArray[iRegion] -> At(iHit);
                    if (fUseTransformCSCombination) {
                        fTracker[kViewXY][iLeftRight] -> AddHit(hit,LKVector3::kX,LKVector3::kY);
                        fTracker[kViewZY][iLeftRight] -> AddHit(hit,LKVector3::kZ,LKVector3::kY);
                        if (iRegion==iChain) hit -> SetSortValue(+1);
                        if (iRegion==iStrip) hit -> SetSortValue(-1);
                    }
                    else {
                        if (iRegion==iChain) fTracker[kViewXY][iLeftRight] -> AddHit(hit,LKVector3::kX,LKVector3::kY);
                        if (iRegion==iStrip) fTracker[kViewZY][iLeftRight] -> AddHit(hit,LKVector3::kZ,LKVector3::kY);
                    }
                }
            }
            TransformAndSelectHits(fTracker[kViewXY][iLeftRight], fTracker[kViewZY][iLeftRight]);
            auto goodTrack = MakeTrack(fTrackXY,fTrackZY,x1,x2);
        }
    }

    lk_info << "Found " << fNumTracks << " tracks" << endl;
}

bool TTHTTrackingTask::EndOfRun()
{
    return true;
}
