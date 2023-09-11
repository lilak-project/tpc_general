#include "LKHoughTransformTracker.h"

ClassImp(LKHoughTransformTracker);

LKHoughTransformTracker::LKHoughTransformTracker()
{
    fLineFitter = LKODRFitter::GetFitter();
    fImagePoint = new LKImagePoint();
    fHoughPoint = new LKHoughPointRT();
    fImagePointArray = new TClonesArray("LKImagePoint",100);
    fTrackArray = new TClonesArray("LKLinearTrack",20);
    Clear();
}

bool LKHoughTransformTracker::Init()
{
    return true;
}

void LKHoughTransformTracker::Clear(Option_t *option)
{
    fRangeHoughSpace[0][0] = fRangeHoughSpaceInit[0][0];
    fRangeHoughSpace[0][1] = fRangeHoughSpaceInit[0][1];
    fRangeHoughSpace[1][0] = fRangeHoughSpaceInit[1][0];
    fRangeHoughSpace[1][1] = fRangeHoughSpaceInit[1][1];

    fBinSizeHoughSpace[0] = (fRangeHoughSpace[0][1]-fRangeHoughSpace[0][0])/fNumBinsHoughSpace[0];
    fBinSizeHoughSpace[1] = (fRangeHoughSpace[1][1]-fRangeHoughSpace[1][0])/fNumBinsHoughSpace[1];
    fBinSizeMaxHoughSpace = sqrt(fBinSizeHoughSpace[0]*fBinSizeHoughSpace[0] + fBinSizeHoughSpace[1]*fBinSizeHoughSpace[1]);

    fIdxSelectedR = 0;
    fIdxSelectedT = 0;
    fNumLinearTracks = 0;
    fTrackArray -> Clear("C");

    if (fUsingImagePointArray) {
        fNumImagePoints = 0;
        fImagePointArray -> Clear("C");
    }
    else {
        if (fInitializedImageData) {
            for(int i = 0; i < fNumBinsImageSpace[0]; ++i)
                for(int j = 0; j < fNumBinsImageSpace[1]; ++j)
                    fImageData[i][j] = 0;
        }
    }

    if (fInitializedHoughData) {
        for(int i = 0; i < fNumBinsHoughSpace[0]; ++i)
            for(int j = 0; j < fNumBinsHoughSpace[1]; ++j)
                fHoughData[i][j] = 0;
    }
}

void LKHoughTransformTracker::Print(Option_t *option) const
{
}

void LKHoughTransformTracker::SetImageData(double** imageData)
{
    if (fInitializedImageData==false) {
        fImageData = new double*[fNumBinsImageSpace[0]];
        for(int i = 0; i < fNumBinsImageSpace[0]; ++i) {
            fImageData[i] = new double[fNumBinsImageSpace[1]];
            for(int j = 0; j < fNumBinsImageSpace[1]; ++j)
                fImageData[i][j] = 0;
        }
        fInitializedImageData = true;
    }

    for (auto ix=0; ix<fNumBinsImageSpace[0]; ++ix)
        for (auto iy=0; iy<fNumBinsImageSpace[0]; ++iy)
            fImageData[ix][iy] = imageData[ix][iy];

    fNumImagePoints = fNumBinsImageSpace[0] * fNumBinsImageSpace[1];
}

void LKHoughTransformTracker::SetCorrelatePointBand()
{
    fCorrelateType = kCorrelatePointBand;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHoughTransformTracker::SetCorrelateBoxLine()
{
    fCorrelateType = kCorrelateBoxLine;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHoughTransformTracker::SetCorrelateBoxBand()
{
    fCorrelateType = kCorrelateBoxBand;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHoughTransformTracker::SetCorrelateDistance()
{
    fCorrelateType = kCorrelateDistance;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHoughTransformTracker::SetImageSpaceRange(int nx, double x1, double x2, int ny, double y1, double y2)
{
    fNumBinsImageSpace[0] = nx;
    fBinSizeImageSpace[0] = (x2-x1)/nx;
    fRangeImageSpace[0][0] = x1;
    fRangeImageSpace[0][1] = x2;

    fNumBinsImageSpace[1] = ny;
    fBinSizeImageSpace[1] = (y2-y1)/ny;
    fRangeImageSpace[1][0] = y1;
    fRangeImageSpace[1][1] = y2;

    fBinSizeMaxImageSpace = sqrt(fBinSizeImageSpace[0]*fBinSizeImageSpace[0] + fBinSizeImageSpace[1]*fBinSizeImageSpace[1]);
    if (fMaxWeightingDistance==0)
        fMaxWeightingDistance = fBinSizeMaxImageSpace;
}

void LKHoughTransformTracker::SetHoughSpaceBins(int nr, int nt)
{
    if (fInitializedHoughData) {
        e_warning << "hough data is already initialized!" << endl;
        return;
    }

    double r1 = DBL_MAX;
    double r2 = -DBL_MAX;
    double t1 = DBL_MAX;
    double t2 = -DBL_MAX;
    double x0, y0, r0;//, t0;
    TVector3 v3Diff;
    for (auto iCorner : {0,1,2,3})
    {
        if (iCorner==0) { x0 = fRangeImageSpace[0][0]; y0 = fRangeImageSpace[0][0]; }
        if (iCorner==1) { x0 = fRangeImageSpace[0][0]; y0 = fRangeImageSpace[1][1]; }
        if (iCorner==2) { x0 = fRangeImageSpace[0][1]; y0 = fRangeImageSpace[1][0]; }
        if (iCorner==3) { x0 = fRangeImageSpace[0][1]; y0 = fRangeImageSpace[1][1]; }
        v3Diff = (TVector3(x0,y0,0) - fTransformCenter);
        r0 = v3Diff.Mag();
        if (r0<r1) r1 = r0;
        if (r0>r2) r2 = r0;
        //if (t0<t1) t1 = t0;
        //if (t0>t2) t2 = t0;
    }
    if (fTransformCenter.X()>fRangeImageSpace[0][0]&&fTransformCenter.X()<fRangeImageSpace[0][1] && 
        fTransformCenter.Y()>fRangeImageSpace[1][0]&&fTransformCenter.Y()<fRangeImageSpace[1][1]) {
    }
    r1 = 0;
    t1 = -180;
    t2 = +180;

    SetHoughSpaceRange(nr, r1, r2, nt, t1, t2);
}

void LKHoughTransformTracker::SetHoughSpaceRange(int nr, double r1, double r2, int nt, double t1, double t2)
{
    if (fInitializedHoughData) {
        e_warning << "hough data is already initialized!" << endl;
        return;
    }

    e_info << "Initializing hough space with "
        << "r = (" << nr << " | " <<  r1 << ", " << r2 << ") "
        << "t = (" << nt << " | " <<  t1 << ", " << t2 << ")" << endl;

    fRangeHoughSpaceInit[0][0] = r1;
    fRangeHoughSpaceInit[0][1] = r2;
    fRangeHoughSpaceInit[1][0] = t1;
    fRangeHoughSpaceInit[1][1] = t2;

    fNumBinsHoughSpace[0] = nr;
    fBinSizeHoughSpace[0] = (r2-r1)/nr;
    fRangeHoughSpace[0][0] = r1;
    fRangeHoughSpace[0][1] = r2;

    fNumBinsHoughSpace[1] = nt;
    fBinSizeHoughSpace[1] = (t2-t1)/nt;
    fRangeHoughSpace[1][0] = t1;
    fRangeHoughSpace[1][1] = t2;

    fBinSizeMaxHoughSpace = sqrt(fBinSizeHoughSpace[0]*fBinSizeHoughSpace[0] + fBinSizeHoughSpace[1]*fBinSizeHoughSpace[1]);

    fHoughData = new double*[fNumBinsHoughSpace[0]];
    for(int i = 0; i < fNumBinsHoughSpace[0]; ++i) {
        fHoughData[i] = new double[fNumBinsHoughSpace[1]];
        for(int j = 0; j < fNumBinsHoughSpace[1]; ++j)
            fHoughData[i][j] = 0;
    }

    fInitializedHoughData = true;
}

void LKHoughTransformTracker::AddImagePoint(double x, double xError, double y, double yError, double weight)
{
    fUsingImagePointArray = true;
    double x1 = x-xError;
    double x2 = x+xError;
    double y1 = y-yError;
    double y2 = y+yError;
    auto imagePoint = (LKImagePoint*) fImagePointArray -> ConstructedAt(fNumImagePoints);
    imagePoint -> SetPoint(x1,y1,x2,y2,weight);
    ++fNumImagePoints;
}

void LKHoughTransformTracker::AddImagePointBox(double x1, double y1, double x2, double y2, double weight)
{
    fUsingImagePointArray = true;
    auto imagePoint = (LKImagePoint*) fImagePointArray -> ConstructedAt(fNumImagePoints);
    imagePoint -> SetPoint(x1,y1,x2,y2,weight);
    ++fNumImagePoints;
}

LKImagePoint* LKHoughTransformTracker::GetImagePoint(int i)
{
    if (fUsingImagePointArray) {
        auto imagePoint = (LKImagePoint*) fImagePointArray -> At(i);
        return imagePoint;
    }
    else {
        int iy = int(i/fNumBinsImageSpace[1]);
        int ix = i - fNumBinsImageSpace[1] * iy;

        double x1 = fRangeImageSpace[0][0] + ix*fBinSizeImageSpace[0];
        double x2 = fRangeImageSpace[0][0] + (ix+1)*fBinSizeImageSpace[0];
        double y1 = fRangeImageSpace[1][0] + iy*fBinSizeImageSpace[1];
        double y2 = fRangeImageSpace[1][0] + (iy+1)*fBinSizeImageSpace[1];
        fImagePoint -> SetPoint(x1,y1,x2,y2,fImageData[ix][iy]);
    }
    fImagePoint -> Clear();
    return fImagePoint;
}

LKHoughPointRT* LKHoughTransformTracker::GetHoughPoint(int i)
{
    // TODO
    double dr = (fRangeHoughSpace[0][0] + fRangeHoughSpace[0][1]) / fNumBinsHoughSpace[0];
    double dt = (fRangeHoughSpace[1][0] + fRangeHoughSpace[1][1]) / fNumBinsHoughSpace[1];
    int count = 0;
    for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it) {
            if (count==i) {
                double r1 = fRangeHoughSpace[0][0] + ir*fBinSizeHoughSpace[0];
                double r2 = fRangeHoughSpace[0][0] + (ir+1)*fBinSizeHoughSpace[0];
                double t1 = fRangeHoughSpace[1][0] + it*fBinSizeHoughSpace[1];
                double t2 = fRangeHoughSpace[1][0] + (it+1)*fBinSizeHoughSpace[1];
                fHoughPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],r1,t1,r2,t2,fHoughData[ir][it]);
                return fHoughPoint;
            }
            ++count;
        }
    }
    fHoughPoint -> Clear();
    return fHoughPoint;
}

LKHoughPointRT* LKHoughTransformTracker::GetHoughPoint(int ir, int it)
{
    double r1 = fRangeHoughSpace[0][0] + ir*fBinSizeHoughSpace[0];
    double r2 = fRangeHoughSpace[0][0] + (ir+1)*fBinSizeHoughSpace[0];
    double t1 = fRangeHoughSpace[1][0] + it*fBinSizeHoughSpace[1];
    double t2 = fRangeHoughSpace[1][0] + (it+1)*fBinSizeHoughSpace[1];
    fHoughPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],r1,t1,r2,t2,fHoughData[ir][it]);
    return fHoughPoint;
}

void LKHoughTransformTracker::Transform()
{
    //if (fUsingImagePointArray)
    {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it)
        {
            double theta0 = fRangeHoughSpace[1][0] + (it+0.5)*fBinSizeHoughSpace[1];
            double theta1 = fRangeHoughSpace[1][0] + it*fBinSizeHoughSpace[1];
            double theta2 = fRangeHoughSpace[1][0] + (it+1)*fBinSizeHoughSpace[1];

            for (int iImage=0; iImage<fNumImagePoints; ++iImage)
            {
                auto imagePoint = GetImagePoint(iImage);

                if (fCorrelateType==kCorrelateBoxBand)
                {
                    int irMax = -INT_MAX;
                    int irMin = INT_MAX;
                    for (auto iImageCorner : {1,2,3,4})
                    {
                        for (auto theta : {theta1, theta2})
                        {
                            auto radius = imagePoint -> EvalR(iImageCorner,theta,fTransformCenter[0],fTransformCenter[1]);
                            int ir = floor( (radius-fRangeHoughSpace[0][0])/fBinSizeHoughSpace[0] );
                            if (irMax<ir) irMax = ir;
                            if (irMin>ir) irMin = ir;
                        }
                    }
                    if (irMax>=fNumBinsHoughSpace[0]) irMax = fNumBinsHoughSpace[0] - 1;
                    if (irMin<0) irMin = 0;
                    for (int ir=irMin; ir<=irMax; ++ir) {
                        auto houghPoint = GetHoughPoint(ir,it);
                        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,houghPoint);
                        if (weight>0)
                            fHoughData[ir][it] = fHoughData[ir][it] + weight;
                    }
                }

                else if (fCorrelateType==kCorrelatePointBand)
                {
                    auto radius = imagePoint -> EvalR(0,theta0,fTransformCenter[0],fTransformCenter[1]);
                    int ir = floor( (radius-fRangeHoughSpace[0][0])/fBinSizeHoughSpace[0] );
                    if (ir>=fNumBinsHoughSpace[0] || ir<0) continue;
                    auto houghPoint = GetHoughPoint(ir,it);
                    auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,houghPoint);
                    if (weight>0)
                        fHoughData[ir][it] = fHoughData[ir][it] + weight;
                }

                else if (fCorrelateType==kCorrelateBoxLine)
                {
                    int irMax = -INT_MAX;
                    int irMin = INT_MAX;
                    for (auto iImageCorner : {1,2,3,4}) {
                        auto radius = imagePoint -> EvalR(iImageCorner,theta0,fTransformCenter[0],fTransformCenter[1]);
                        int ir = floor( (radius-fRangeHoughSpace[0][0])/fBinSizeHoughSpace[0] );
                        if (irMax<ir) irMax = ir;
                        if (irMin>ir) irMin = ir;
                    }
                    if (irMax>=fNumBinsHoughSpace[0]) irMax = fNumBinsHoughSpace[0] - 1;
                    if (irMin<0) irMin = 0;
                    for (int ir=irMin; ir<=irMax; ++ir) {
                        auto houghPoint = GetHoughPoint(ir,it);
                        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,houghPoint);
                        if (weight>0)
                            fHoughData[ir][it] = fHoughData[ir][it] + weight;
                    }
                }

                else if (fCorrelateType==kCorrelateDistance)
                {
                    auto radius = imagePoint -> EvalR(0,theta0,fTransformCenter[0],fTransformCenter[1]);
                    int ir0 = floor( (radius-fRangeHoughSpace[0][0])/fBinSizeHoughSpace[0] );
                    int rangeR = int(fMaxWeightingDistance/fBinSizeMaxImageSpace);
                    int debug_count_ir1 = 0;
                    int debug_count_ir2 = 0;
                    if (ir0>=0)
                    {
                        for (auto irOff=0; irOff>=-rangeR; --irOff) {
                            int ir = ir0 + irOff;
                            if (ir<0||ir>=fNumBinsHoughSpace[0])
                                continue;
                            auto houghPoint = GetHoughPoint(ir,it);
                            auto distance = houghPoint -> DistanceToImagePoint(imagePoint);
                            if (distance>0) {
                                auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,houghPoint);
                                if (weight>0)
                                    fHoughData[ir][it] = fHoughData[ir][it] + weight;
                            }
                            else
                                break;
                        }
                    }
                    if (ir0<=fNumBinsHoughSpace[0]-1)
                    {
                        for (auto irOff=0; irOff<=rangeR; ++irOff) {
                            int ir = ir0 + irOff;
                            if (ir<0||ir>=fNumBinsHoughSpace[0])
                                continue;
                            auto houghPoint = GetHoughPoint(ir,it);
                            auto distance = houghPoint -> DistanceToImagePoint(imagePoint);
                            if (distance>0) {
                                auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,houghPoint);
                                if (weight>0)
                                    fHoughData[ir][it] = fHoughData[ir][it] + weight;
                            }
                            else
                                break;
                        }
                    }
                }

            } // image

        }
    }
    //else {
    //}
}

LKHoughPointRT* LKHoughTransformTracker::FindNextMaximumHoughPoint()
{
    fIdxSelectedR = -1;
    fIdxSelectedT = -1;
    double maxValue = -1;
    for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it) {
            if (maxValue<fHoughData[ir][it]) {
                fIdxSelectedR = ir;
                fIdxSelectedT = it;
                maxValue = fHoughData[ir][it];
            }
        }
    }
    if (fIdxSelectedR<0) {
        fHoughPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],-1,0,-1,0,-1);
        return fHoughPoint;
    }

    auto houghPoint = GetHoughPoint(fIdxSelectedR,fIdxSelectedT);
    return houghPoint;
}

void LKHoughTransformTracker::CleanLastHoughPoint(double rWidth, double tWidth)
{
    if (rWidth<0) rWidth = (fRangeHoughSpace[0][1]-fRangeHoughSpace[0][0])/15;
    if (tWidth<0) tWidth = (fRangeHoughSpace[1][1]-fRangeHoughSpace[1][0])/15;;
    int numBinsHalfR = std::floor(rWidth/fBinSizeHoughSpace[0]/2.);
    int numBinsHalfT = std::floor(tWidth/fBinSizeHoughSpace[1]/2.);
    if (rWidth==0) numBinsHalfR = 0;
    if (tWidth==0) numBinsHalfT = 0;

    for (int iOffR=-numBinsHalfR; iOffR<=numBinsHalfR; ++iOffR) {
        int ir = fIdxSelectedR + iOffR;
        if (ir<0||ir>=fNumBinsHoughSpace[0])
            continue;
        for (int iOffT=-numBinsHalfT; iOffT<=numBinsHalfT; ++iOffT) {
            int it = fIdxSelectedT + iOffT;
            if (it<0||it>=fNumBinsHoughSpace[1])
                continue;
            fHoughData[ir][it] = 0;
        }
    }
}

LKHoughPointRT* LKHoughTransformTracker::ReinitializeFromLastHoughPoint()
{
    auto ir1 = fIdxSelectedR - 1;
    auto ir2 = fIdxSelectedR + 1;
    auto it1 = fIdxSelectedT - 1;
    auto it2 = fIdxSelectedT + 1;
    if (ir2>=fNumBinsHoughSpace[0]) ir2 = fNumBinsHoughSpace[0] - 1;
    if (ir1<0) ir1 = 0;
    if (it2>=fNumBinsHoughSpace[1]) it2 = fNumBinsHoughSpace[1] - 1;
    if (it1<0) ir1 = 0;

    double rMin = DBL_MAX;
    double rMax = -DBL_MAX;
    double tMin = DBL_MAX;
    double tMax = -DBL_MAX;
    double weightTotal = 0.;

    for (auto ir=ir1; ir<=ir2; ++ir) {
        for (auto it=it1; it<=it2; ++it) {
            auto houghPoint = GetHoughPoint(ir,it);
            if (rMin>houghPoint->fRadius1) rMin = houghPoint -> fRadius1;
            if (rMax<houghPoint->fRadius2) rMax = houghPoint -> fRadius2;
            if (tMin>houghPoint->fTheta1 ) tMin = houghPoint -> fTheta1;
            if (tMax<houghPoint->fTheta2 ) tMax = houghPoint -> fTheta2;
            weightTotal += houghPoint -> fWeight;
        }
    }

    fRangeHoughSpace[0][0] = rMin;
    fRangeHoughSpace[0][1] = rMax;
    fRangeHoughSpace[1][0] = tMin;
    fRangeHoughSpace[1][1] = tMax;
    fBinSizeHoughSpace[0] = (fRangeHoughSpace[0][1]-fRangeHoughSpace[0][0])/fNumBinsHoughSpace[0];
    fBinSizeHoughSpace[1] = (fRangeHoughSpace[1][1]-fRangeHoughSpace[1][0])/fNumBinsHoughSpace[1];
    fBinSizeMaxHoughSpace = sqrt(fBinSizeHoughSpace[0]*fBinSizeHoughSpace[0] + fBinSizeHoughSpace[1]*fBinSizeHoughSpace[1]);

    fHoughPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],rMin,tMin,rMax,tMax,weightTotal);

    if (fInitializedHoughData) {
        for(int i = 0; i < fNumBinsHoughSpace[0]; ++i)
            for(int j = 0; j < fNumBinsHoughSpace[1]; ++j)
                fHoughData[i][j] = 0;
    }

    return fHoughPoint;
}

void LKHoughTransformTracker::RetransformFromLastHoughPoint()
{
    ReinitializeFromLastHoughPoint();
    Transform();
}

LKLinearTrack* LKHoughTransformTracker::FitTrackWithHoughPoint(LKHoughPointRT* houghPoint, double weightCut)
{
    if (weightCut==-1) {
        weightCut = 0.2;
    }
    auto track = (LKLinearTrack*) fTrackArray -> ConstructedAt(fNumLinearTracks);
    ++fNumLinearTracks;

    fLineFitter -> Reset();

    for (int iImage=0; iImage<fNumImagePoints; ++iImage) {
        auto imagePoint = GetImagePoint(iImage);
        imagePoint->fWeight = 1;
        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,houghPoint);
        if (weight > weightCut) {
            fLineFitter -> PreAddPoint(imagePoint->fX0,imagePoint->fY0,0,imagePoint->fWeight);
        }
    }
    for (int iImage=0; iImage<fNumImagePoints; ++iImage) {
        auto imagePoint = GetImagePoint(iImage);
        imagePoint->fWeight = 1;
        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,houghPoint);
        if (weight > weightCut) {
            fLineFitter -> AddPoint(imagePoint->fX0,imagePoint->fY0,0,imagePoint->fWeight);
        }
    }
    if (fLineFitter->GetNumPoints()<fCutNumTrackHits)
        return track;

    bool fitted = fLineFitter -> FitLine();
    if (fitted==false)
        return track;

    auto centroid = fLineFitter -> GetCentroid();

    auto dx = fRangeImageSpace[1][0] - fRangeImageSpace[0][0];
    auto dy = fRangeImageSpace[1][0] - fRangeImageSpace[0][0];
    auto size = sqrt(dx*dx + dy*dy)/2;

    track -> SetLine(centroid - size*fLineFitter->GetDirection(), centroid + size*fLineFitter->GetDirection());
    track -> SetRMS(fLineFitter->GetRMSLine());

    return track;
}

void LKHoughTransformTracker::DrawAllHoughLines(int i, bool drawRadialLine)
{
    vector<int> corners = {0,1,2,3,4};
    if (i>=0) {
        corners.clear();
        corners.push_back(i);
    }

    for (auto iCorner : corners)
    {
        for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir)
        {
            for (auto it=0; it<fNumBinsHoughSpace[1]; ++it)
            {
                auto houghPoint = GetHoughPoint(ir,it);
                auto graph1 = houghPoint -> GetLineInImageSpace(iCorner,fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
                if (iCorner==0)
                    graph1 -> SetLineStyle(2);
                graph1 -> Draw("samel");
                if (drawRadialLine) {
                    auto graph2 = houghPoint -> GetRadialLineInImageSpace(iCorner,0.25*fBinSizeMaxImageSpace);
                    graph2 -> Draw("samel");
                }
            }
        }
    }
}

void LKHoughTransformTracker::DrawAllHoughBands()
{
    for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir)
    {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it)
        {
            auto houghPoint = GetHoughPoint(ir,it);
            auto graph = houghPoint -> GetBandInImageSpace(fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
            graph -> SetLineColor(kRed);
            graph -> Draw("samel");
        }
    }
}

TH2D* LKHoughTransformTracker::GetHistImageSpace(TString name, TString title)
{
    if (name.IsNull()) name = "histImageSpace";
    TString correlatorName;
         if (fCorrelateType==kCorrelatePointBand)  correlatorName = "Point-Band";
    else if (fCorrelateType==kCorrelateBoxLine)  correlatorName = "Box-Line";
    else if (fCorrelateType==kCorrelateBoxBand)  correlatorName = "Box-Band";
    else if (fCorrelateType==kCorrelateDistance) correlatorName = "Distance";
    if (title.IsNull()) title = Form("%s (%dx%d), TC (x,y) = (%.2f, %.2f);x;y", correlatorName.Data(), fNumBinsImageSpace[0], fNumBinsImageSpace[1], fTransformCenter[0],fTransformCenter[1]);
    auto hist = new TH2D(name,title, fNumBinsImageSpace[0],fRangeImageSpace[0][0],fRangeImageSpace[0][1],fNumBinsImageSpace[1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
    if (fUsingImagePointArray) {
        for (auto iPoint=0; iPoint<fNumImagePoints; ++iPoint) {
            auto imagePoint = (LKImagePoint*) fImagePointArray -> At(iPoint);
            hist -> Fill(imagePoint->GetCenterX(), imagePoint->GetCenterY(), imagePoint->fWeight);
        }
    }
    hist -> GetXaxis() -> SetTitleOffset(1.25);
    hist -> GetYaxis() -> SetTitleOffset(1.60);
    return hist;
}

TH2D* LKHoughTransformTracker::GetHistHoughSpace(TString name, TString title)
{
    if (name.IsNull()) name = "histHoughSpace";
    TString correlatorName;
         if (fCorrelateType==kCorrelatePointBand)  correlatorName = "Point-Band";
    else if (fCorrelateType==kCorrelateBoxLine)  correlatorName = "Box-Line";
    else if (fCorrelateType==kCorrelateBoxBand)  correlatorName = "Box-Band";
    else if (fCorrelateType==kCorrelateDistance) correlatorName = "Distance";
    if (title.IsNull()) title = Form("%s (%dx%d), TC (x,y) = (%.2f, %.2f);#theta (degrees);Radius", correlatorName.Data(), fNumBinsHoughSpace[0], fNumBinsHoughSpace[1], fTransformCenter[0],fTransformCenter[1]);
    auto hist = new TH2D(name,title, fNumBinsHoughSpace[1],fRangeHoughSpace[1][0],fRangeHoughSpace[1][1],fNumBinsHoughSpace[0],fRangeHoughSpace[0][0],fRangeHoughSpace[0][1]);
    for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it) {
            if (fHoughData[ir][it]>0) {
                hist -> SetBinContent(it+1,ir+1,fHoughData[ir][it]);
            }
        }
    }
    hist -> GetXaxis() -> SetTitleOffset(1.25);
    hist -> GetYaxis() -> SetTitleOffset(1.60);
    return hist;
}
