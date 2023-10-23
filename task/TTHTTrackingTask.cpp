#include "TTHTTrackingTask.h"

ClassImp(TTHTTrackingTask);

TTHTTrackingTask::TTHTTrackingTask()
{
    fName = "TTHTTrackingTask";
}

bool TTHTTrackingTask::Init()
{
    lk_info << "Initializing TTHTTrackingTask" << std::endl;

    if (fPar -> CheckPar("TTHTTrackingTask/HTMaxCutXZPlane"))      fHTMaxCutXZPlane      = fPar -> GetParDouble("TTHTTrackingTask/HTMaxCutXZPlane");
    if (fPar -> CheckPar("TTHTTrackingTask/HTMaxCutZYPlane"))      fHTMaxCutZYPlane      = fPar -> GetParDouble("TTHTTrackingTask/HTMaxCutZYPlane");
    if (fPar -> CheckPar("TTHTTrackingTask/HTMaxCutXYPlane"))      fHTMaxCutXYPlane      = fPar -> GetParDouble("TTHTTrackingTask/HTMaxCutXYPlane");
    if (fPar -> CheckPar("TTHTTrackingTask/nHitsTrackCutXZPlane")) fNHitsTrackCutXZPlane = fPar -> GetParDouble("TTHTTrackingTask/nHitsTrackCutXZPlane");
    if (fPar -> CheckPar("TTHTTrackingTask/nHitsTrackCutZYPlane")) fNHitsTrackCutZYPlane = fPar -> GetParDouble("TTHTTrackingTask/nHitsTrackCutZYPlane");
    if (fPar -> CheckPar("TTHTTrackingTask/nHitsTrackCutXYPlane")) fNHitsTrackCutXYPlane = fPar -> GetParDouble("TTHTTrackingTask/nHitsTrackCutXYPlane");
    if (fPar -> CheckPar("TTHTTrackingTask/nBinsRadius")) {
        for (auto iCLR : {kMMCenter,kMMLeft,kMMRight}) {
            fNR[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/nBinsRadius",iCLR);
            fNT[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/nBinsTheta",iCLR);
        }
    }
    if (fPar -> CheckPar("TTHTTrackingTask/transform_center_in_XY_plane")) {
        for (auto iCLR : {kMMCenter,kMMLeft,kMMRight}) {
            fTCXInXY[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/transform_center_in_XY_plane",iCLR);
            fTCYInXY[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/transform_center_in_XY_plane",iCLR);
            fTCXInZY[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/transform_center_in_ZY_plane",iCLR);
            fTCYInZY[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/transform_center_in_ZY_plane",iCLR);
            fTCXInXZ[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/transform_center_in_XZ_plane",iCLR);
            fTCYInXZ[iCLR] = fPar -> GetParDouble("TTHTTrackingTask/transform_center_in_XZ_plane",iCLR);
        }
    }

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
    fEventHeaderHolder = fRun -> KeepBranchA("EventHeader");

    auto SetTracker = [](LKHTLineTracker* tk, double tx, double ty, int nx, double x1, double x2, int ny, double y1, double y2, int nr, int nt) {
        tk -> SetTransformCenter(tx, ty);
        tk -> SetImageSpaceRange(nx, x1, x2, ny, y1, y2);
        tk -> SetParamSpaceBins(nr, nt);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();
    };

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight}) {
        for (auto iSCA : {kStrip,kChain,kStripAndChain}) {
            fTrackerXY[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerXY[iCLR][iSCA], fTCXInXY[iCLR], fTCYInXY[iCLR], fNX, fX1, fX2, fNY, fY1, fY2, fNR[iCLR], fNT[iCLR]);
            fTrackerZY[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerZY[iCLR][iSCA], fTCXInZY[iCLR], fTCYInZY[iCLR], fNZ, fZ1, fZ2, fNY, fY1, fY2, fNR[iCLR], fNT[iCLR]);
            fTrackerXZ[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerXZ[iCLR][iSCA], fTCXInXZ[iCLR], fTCYInXZ[iCLR], fNX, fX1, fX2, fNZ, fZ1, fZ2, fNR[iCLR], fNT[iCLR]);
        }
    }

    return true;
}

auto TTHTTrackingTask::TransformAndFit(int iView, int iCLR, LKHTLineTracker* tk[3][3])
{
    int iSCA = ((iView==kFrontView)?kChain:kStrip);
    auto tkAll = tk[iCLR][kStripAndChain];
    auto tkFit = tk[iCLR][iSCA];

    tkAll -> Transform();
    auto paramPoint = tkAll -> FindNextMaximumParamPoint();

    LKLinearTrack* track;

         if (iView==kTopView   && paramPoint->GetWeight()<fNHitsTrackCutXZPlane) return track;
    else if (iView==kSideView  && paramPoint->GetWeight()<fNHitsTrackCutZYPlane) return track;
    else if (iView==kFrontView && paramPoint->GetWeight()<fNHitsTrackCutXYPlane) return track;

    if (iCLR!=kMMCenter && iView!=kTopView)
        track = tkFit -> FitTrackWithParamPoint(paramPoint);

    return track;
}

void TTHTTrackingTask::Exec(Option_t *option)
{
    fTrackArray -> Clear("C");

    auto numHitsAll = 0;
    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight})
        for (auto iSCA : {kStrip,kChain})
            numHitsAll += fHitArray[iCLR][iSCA] -> GetEntries();

    auto eventHeader = (TTEventHeader*) fEventHeaderHolder -> At(0);
    auto goodEvent = eventHeader -> IsGoodEvent();
    if (numHitsAll==0||goodEvent==false)
        return;

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight}) {
        for (auto iSCA : {kStrip,kChain,kStripAndChain})
        {
            fTrackerXY[iCLR][iSCA] -> Clear();
            fTrackerZY[iCLR][iSCA] -> Clear();
            fTrackerXZ[iCLR][iSCA] -> Clear();
        }
    }

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight}) {
        for (auto iSCA : {kStrip,kChain})
        {
            auto numHits = fHitArray[iCLR][iSCA] -> GetEntries();
            for (auto iHit=0; iHit<numHits; ++iHit)
            {
                auto hit = (LKHit*) fHitArray[iCLR][iSCA] -> At(iHit);
                auto x = hit -> X();
                auto y = 350 - (hit -> Y());
                auto z = hit -> Z();
                auto dx = hit->GetDX();
                auto dy = hit->GetDY();
                auto dz = hit->GetDZ();

                fTrackerXY[iCLR][iSCA] -> AddImagePoint(x,dx,y,dy,1);
                fTrackerZY[iCLR][iSCA] -> AddImagePoint(z,dz,y,dy,1);
                fTrackerXZ[iCLR][iSCA] -> AddImagePoint(x,dx,z,dz,1);
                fTrackerXY[iCLR][kStripAndChain] -> AddImagePoint(x,dx,y,dy,1);
                fTrackerZY[iCLR][kStripAndChain] -> AddImagePoint(z,dz,y,dy,1);
                fTrackerXZ[iCLR][kStripAndChain] -> AddImagePoint(x,dx,z,dz,1);
            }
        }
    }

    TransformAndFit(kTopView,   kMMCenter, fTrackerXZ);
    TransformAndFit(kTopView,   kMMLeft,   fTrackerXZ);
    TransformAndFit(kTopView,   kMMRight,  fTrackerXZ);
    TransformAndFit(kFrontView, kMMCenter, fTrackerXY);
    TransformAndFit(kSideView,  kMMCenter, fTrackerZY);

    auto trackFL = TransformAndFit(kFrontView, kMMLeft,  fTrackerXY);
    auto trackSL = TransformAndFit(kSideView,  kMMLeft,  fTrackerZY);
    auto trackFR = TransformAndFit(kFrontView, kMMRight, fTrackerXY);
    auto trackSR = TransformAndFit(kSideView,  kMMRight, fTrackerZY);

    if (trackFL!=nullptr && trackSL!=nullptr)
    {
        auto point1 = trackFL -> GetPoint1();
        auto point2 = trackSL -> GetPoint2();
        point1.SetXYZ(point1.X(), point1.Y(), 0);
        point2.SetXYZ(0, point2.Y(), point2.X());
        auto directionF = trackFL -> Direction();
        auto directionS = trackSL -> Direction();
        TVector3 normal1(-directionF.Y(), directionF.X(), 0);
        TVector3 normal2(0, directionS.X(), -directionS.Y());
        LKGeoPlaneWithCenter plane1(point1, normal1);
        LKGeoPlaneWithCenter plane2(point2, normal2);
        LKGeoLine lineL = plane1.GetCrossSectionLine(plane2);
        auto vertex1 = lineL.GetPointAtX(0);
        auto vertex2 = lineL.GetPointAtX(fX1);
        auto track = (LKLinearTrack*) fTrackArray -> ConstructedAt(fTrackArray->GetEntries());
        track -> SetTrack(vertex1, vertex2);
    }
    if (trackFR!=nullptr && trackSR!=nullptr)
    {
        auto point1 = trackFR -> GetPoint1();
        auto point2 = trackSR -> GetPoint2();
        point1.SetXYZ(point1.X(), point1.Y(), 0);
        point2.SetXYZ(0, point2.Y(), point2.X());
        auto directionF = trackFR -> Direction();
        auto directionS = trackSR -> Direction();
        TVector3 normal1(-directionF.Y(), directionF.X(), 0);
        TVector3 normal2(0, directionS.X(), -directionS.Y());
        LKGeoPlaneWithCenter plane1(point1, normal1);
        LKGeoPlaneWithCenter plane2(point2, normal2);
        LKGeoLine lineR = plane1.GetCrossSectionLine(plane2);
        auto vertex1 = lineR.GetPointAtX(0);
        auto vertex2 = lineR.GetPointAtX(fX2);
        auto track = (LKLinearTrack*) fTrackArray -> ConstructedAt(fTrackArray->GetEntries());
        track -> SetTrack(vertex1, vertex2);
    }

    lk_debug << "Found " << fTrackArray->GetEntries() << " tracks" << endl;
}

bool TTHTTrackingTask::EndOfRun()
{
    return true;
}
