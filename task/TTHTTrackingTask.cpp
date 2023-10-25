#include "TTHTTrackingTask.h"

ClassImp(TTHTTrackingTask);

TTHTTrackingTask::TTHTTrackingTask()
{
    fName = "TTHTTrackingTask";
}

bool TTHTTrackingTask::Init()
{
    lk_info << "Initializing TTHTTrackingTask" << std::endl;

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
    fTrackArray = fRun -> GetBranchA("Track");

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
    fStripHitCollection = new TObjArray();
    fChainHitCollection = new TObjArray();

    return true;
}

bool TTHTTrackingTask::TransformAndSelectHits(LKHTLineTracker* trackerXY, LKHTLineTracker* trackerZY)
{
    trackerXY -> Transform();
    trackerZY -> Transform();
    auto paramPointXY = trackerXY -> FindNextMaximumParamPoint();
    auto paramPointZY = trackerZY -> FindNextMaximumParamPoint();
    trackerXY -> SelectHits(paramPointXY);
    trackerZY -> SelectHits(paramPointZY);
    auto hitArrayXY = trackerXY -> GetSelectedHitArray();
    auto hitArrayZY = trackerZY -> GetSelectedHitArray();
    trackerXY -> ClearHits();
    trackerZY -> ClearHits();
    TIter nextXY(hitArrayXY);
    TIter nextZY(hitArrayZY);
    while ((auto hit1 = (LKHit*) nextXY())) {
        while ((auto hit2 = (LKHit*) nextZY())) {
            if (hit1==hit2) {
                fCrossHitCollection -> Add(hit1);
                trackerXY -> AddHit(hit,LKVector3::kX,LKVector3::kY);
                trackerZY -> AddHit(hit,LKVector3::kZ,LKVector3::kY);
            }
        }
    }

    TIter nextStrip(fStripHitCollection); while ((auto hit = (LKHit*) nextStrip())) hit -> SetSortValue(-1);
    TIter nextChain(fChainHitCollection); while ((auto hit = (LKHit*) nextChain())) hit -> SetSortValue(+1);
    fTrackXY = trackerXY -> FitTrackWithParamPoint(paramPointXY);

    TIter nextStrip(fStripHitCollection); while ((auto hit = (LKHit*) nextStrip())) hit -> SetSortValue(+1);
    TIter nextChain(fChainHitCollection); while ((auto hit = (LKHit*) nextChain())) hit -> SetSortValue(-1);
    fTrackZY = trackerZY -> FitTrackWithParamPoint(paramPointZY);
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
        auto track = (LKLinearTrack*) fTrackArray -> ConstructedAt(fNumTracks);
        track -> SetTrack(vertex1, vertex2);
        track -> SetTrackID(fNumTracks);
        auto hitArray = trackXY -> GetHitArray();
        TIter next(hitArray);
        while ((auto hit = (LKHit*)next()))
            track -> AddHit(hit);
        ++fNumTracks;
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
        if (fHitArray[iStrip] -> GetEntries() > fNumHitsCutForTransform && fHitArray[iChain] -> GetEntries() > fNumHitsCutForTransform)
        {
            fStripHitCollection -> Clear();
            fChainHitCollection -> Clear();
            for (auto iRegion : {iStrip ,iChain}) {
                auto numHits = fHitArray[iRegion] -> GetEntries();
                for (auto iHit=0; iHit<numHits; ++iHit) {
                    auto hit = (LKHit*) fHitArray[iRegion] -> At(iHit);
                    ftracker[kviewxy][ileftright] -> AddHit(hit,LKVector3::kX,LKVector3::kY);
                    fTracker[kViewZY][iLeftRight] -> AddHit(hit,LKVector3::kZ,LKVector3::kY);
                    if (iRegion==iStrip) fStripHitCollection -> Add(hit);
                    if (iRegion==iChain) fChainHitCollection -> Add(hit);
                }
            }
            TransformAndSelectHits(ftracker[kviewxy][ileftright], fTracker[kViewZY][iLeftRight]);
            auto goodTrack = MakeTrack(fTrackXY,fTrackZY,x1,x2);
        }
    }

    // propagate track cand ------------------------------------------------------------------------
    // if hit was used for fitting inside the tracker, track cand "0" would have been added.
    for (auto iRegion : {kCenter, kLStrip, kLChain, kRStrip, kRChain}) {
        auto numHits = fHitArray[iRegion] -> GetEntries();
        for (auto iHit=0; iHit<numHits; ++iHit) {
            auto hit = (LKHit*) fHitArray[iRegion] -> At(iHit);
            auto numCands = hit -> GetNumTrackCands();
            if (numCands>0)
                hit -> SetTrackID
        }
    }

    lk_debug << "Found " << fNumTracks << " tracks" << endl;
}

bool TTHTTrackingTask::EndOfRun()
{
    return true;
}
