#include "LKHTLineTracker.h"

ClassImp(LKHTLineTracker);

LKHTLineTracker::LKHTLineTracker()
{
    fLineFitter = LKODRFitter::GetFitter();
    fImagePoint = new LKImagePoint();
    fParamPoint = new LKParamPointRT();
    fImagePointArray = new TClonesArray("LKImagePoint",100);
    fTrackArray = new TClonesArray("LKLinearTrack",20);
    Clear();
}

bool LKHTLineTracker::Init()
{
    return true;
}

void LKHTLineTracker::Reset()
{
    fRangeParamSpace[0][0] = fRangeParamSpaceInit[0][0];
    fRangeParamSpace[0][1] = fRangeParamSpaceInit[0][1];
    fRangeParamSpace[1][0] = fRangeParamSpaceInit[1][0];
    fRangeParamSpace[1][1] = fRangeParamSpaceInit[1][1];

    fBinSizeParamSpace[0] = (fRangeParamSpace[0][1]-fRangeParamSpace[0][0])/fNumBinsParamSpace[0];
    fBinSizeParamSpace[1] = (fRangeParamSpace[1][1]-fRangeParamSpace[1][0])/fNumBinsParamSpace[1];
    fBinSizeMaxParamSpace = sqrt(fBinSizeParamSpace[0]*fBinSizeParamSpace[0] + fBinSizeParamSpace[1]*fBinSizeParamSpace[1]);

    fIdxSelectedR = 0;
    fIdxSelectedT = 0;

    if (fInitializedParamData) {
        for(int i = 0; i < fNumBinsParamSpace[0]; ++i)
            for(int j = 0; j < fNumBinsParamSpace[1]; ++j)
                fParamData[i][j] = 0;
    }
}

void LKHTLineTracker::Clear(Option_t *option)
{
    fRangeParamSpace[0][0] = fRangeParamSpaceInit[0][0];
    fRangeParamSpace[0][1] = fRangeParamSpaceInit[0][1];
    fRangeParamSpace[1][0] = fRangeParamSpaceInit[1][0];
    fRangeParamSpace[1][1] = fRangeParamSpaceInit[1][1];

    fBinSizeParamSpace[0] = (fRangeParamSpace[0][1]-fRangeParamSpace[0][0])/fNumBinsParamSpace[0];
    fBinSizeParamSpace[1] = (fRangeParamSpace[1][1]-fRangeParamSpace[1][0])/fNumBinsParamSpace[1];
    fBinSizeMaxParamSpace = sqrt(fBinSizeParamSpace[0]*fBinSizeParamSpace[0] + fBinSizeParamSpace[1]*fBinSizeParamSpace[1]);

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

    if (fInitializedParamData) {
        for(int i = 0; i < fNumBinsParamSpace[0]; ++i)
            for(int j = 0; j < fNumBinsParamSpace[1]; ++j)
                fParamData[i][j] = 0;
    }
}

void LKHTLineTracker::Print(Option_t *option) const
{
    lk_info << "param1 = (" << fRangeImageSpace[0][0] << ", " << fRangeImageSpace[0][1] << ") / " << fNumBinsImageSpace[0] << endl;
    lk_info << "param2 = (" << fRangeImageSpace[1][0] << ", " << fRangeImageSpace[1][1] << ") / " << fNumBinsImageSpace[1] << endl;
    lk_info << "image1 = (" << fRangeParamSpace[0][0] << ", " << fRangeParamSpace[0][1] << ") / " << fNumBinsParamSpace[0] << endl;
    lk_info << "image2 = (" << fRangeParamSpace[1][0] << ", " << fRangeParamSpace[1][1] << ") / " << fNumBinsParamSpace[1] << endl;
    lk_info << "Number of image points = " << fNumImagePoints << endl;
}

void LKHTLineTracker::SetImageData(double** imageData)
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
        for (auto iy=0; iy<fNumBinsImageSpace[1]; ++iy)
            fImageData[ix][iy] = imageData[ix][iy];

    fNumImagePoints = fNumBinsImageSpace[0] * fNumBinsImageSpace[1];
}

void LKHTLineTracker::SetCorrelatePointBand()
{
    fCorrelateType = kCorrelatePointBand;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFConst();
}

void LKHTLineTracker::SetCorrelateBoxLine()
{
    fCorrelateType = kCorrelateBoxLine;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFConst();
}

void LKHTLineTracker::SetCorrelateBoxRibbon()
{
    fCorrelateType = kCorrelateBoxRibbon;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFConst();
}

void LKHTLineTracker::SetCorrelateBoxBand()
{
    fCorrelateType = kCorrelateBoxBand;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFConst();
}

void LKHTLineTracker::SetImageSpaceRange(int nx, double x1, double x2, int ny, double y1, double y2)
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

void LKHTLineTracker::SetParamSpaceBins(int nr, int nt)
{
    if (fInitializedParamData) {
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
    r1 = -r2;
    //t1 = -180;
    //t2 = +180;
    t1 = 0;
    t2 = +180;

    SetParamSpaceRange(nr, r1, r2, nt, t1, t2);
}

void LKHTLineTracker::SetParamSpaceRange(int nr, double r1, double r2, int nt, double t1, double t2)
{
    if (fInitializedParamData) {
        e_warning << "hough data is already initialized!" << endl;
        return;
    }

    e_info << "Initializing hough space with "
        << "r = (" << nr << " | " <<  r1 << ", " << r2 << ") "
        << "t = (" << nt << " | " <<  t1 << ", " << t2 << ")" << endl;

    fRangeParamSpaceInit[0][0] = r1;
    fRangeParamSpaceInit[0][1] = r2;
    fRangeParamSpaceInit[1][0] = t1;
    fRangeParamSpaceInit[1][1] = t2;

    fNumBinsParamSpace[0] = nr;
    fBinSizeParamSpace[0] = (r2-r1)/nr;
    fRangeParamSpace[0][0] = r1;
    fRangeParamSpace[0][1] = r2;

    fNumBinsParamSpace[1] = nt;
    fBinSizeParamSpace[1] = (t2-t1)/nt;
    fRangeParamSpace[1][0] = t1;
    fRangeParamSpace[1][1] = t2;

    fBinSizeMaxParamSpace = sqrt(fBinSizeParamSpace[0]*fBinSizeParamSpace[0] + fBinSizeParamSpace[1]*fBinSizeParamSpace[1]);

    fParamData = new double*[fNumBinsParamSpace[0]];
    for(int i = 0; i < fNumBinsParamSpace[0]; ++i) {
        fParamData[i] = new double[fNumBinsParamSpace[1]];
        for(int j = 0; j < fNumBinsParamSpace[1]; ++j)
            fParamData[i][j] = 0;
    }

    fInitializedParamData = true;
}

void LKHTLineTracker::AddImagePoint(double x, double xError, double y, double yError, double weight)
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

void LKHTLineTracker::AddImagePointBox(double x1, double y1, double x2, double y2, double weight)
{
    fUsingImagePointArray = true;
    auto imagePoint = (LKImagePoint*) fImagePointArray -> ConstructedAt(fNumImagePoints);
    imagePoint -> SetPoint(x1,y1,x2,y2,weight);
    ++fNumImagePoints;
}

LKImagePoint* LKHTLineTracker::GetImagePoint(int i)
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
        return fImagePoint;
    }
    fImagePoint -> Clear();
    return fImagePoint;
}

LKImagePoint* LKHTLineTracker::PopImagePoint(int i)
{
    if (fUsingImagePointArray) {
        auto imagePoint = (LKImagePoint*) fImagePointArray -> At(i);
        fImagePointArray -> RemoveAt(i);
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
        fImageData[ix][iy] = 0;
        return fImagePoint;
    }
    fImagePoint -> Clear();
    return fImagePoint;
}

LKParamPointRT* LKHTLineTracker::GetParamPoint(int i)
{
    // TODO
    double dr = (fRangeParamSpace[0][0] + fRangeParamSpace[0][1]) / fNumBinsParamSpace[0];
    double dt = (fRangeParamSpace[1][0] + fRangeParamSpace[1][1]) / fNumBinsParamSpace[1];
    int count = 0;
    for (auto ir=0; ir<fNumBinsParamSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsParamSpace[1]; ++it) {
            if (count==i) {
                double r1 = fRangeParamSpace[0][0] + ir*fBinSizeParamSpace[0];
                double r2 = fRangeParamSpace[0][0] + (ir+1)*fBinSizeParamSpace[0];
                double t1 = fRangeParamSpace[1][0] + it*fBinSizeParamSpace[1];
                double t2 = fRangeParamSpace[1][0] + (it+1)*fBinSizeParamSpace[1];
                fParamPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],r1,t1,r2,t2,fParamData[ir][it]);
                return fParamPoint;
            }
            ++count;
        }
    }
    fParamPoint -> Clear();
    return fParamPoint;
}

LKParamPointRT* LKHTLineTracker::GetParamPoint(int ir, int it)
{
    double r1 = fRangeParamSpace[0][0] + ir*fBinSizeParamSpace[0];
    double r2 = fRangeParamSpace[0][0] + (ir+1)*fBinSizeParamSpace[0];
    double t1 = fRangeParamSpace[1][0] + it*fBinSizeParamSpace[1];
    double t2 = fRangeParamSpace[1][0] + (it+1)*fBinSizeParamSpace[1];
    fParamPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],r1,t1,r2,t2,fParamData[ir][it]);
    return fParamPoint;
}

void LKHTLineTracker::Transform()
{
    //if (fUsingImagePointArray)
    {
        for (auto it=0; it<fNumBinsParamSpace[1]; ++it)
        {
            double theta0 = fRangeParamSpace[1][0] + (it+0.5)*fBinSizeParamSpace[1];
            double theta1 = fRangeParamSpace[1][0] + it*fBinSizeParamSpace[1];
            double theta2 = fRangeParamSpace[1][0] + (it+1)*fBinSizeParamSpace[1];

            for (int iImage=0; iImage<fNumImagePoints; ++iImage)
            {
                auto imagePoint = GetImagePoint(iImage);

                if (fCorrelateType==kCorrelatePointBand)
                {
                    auto radius = imagePoint -> EvalR(0,theta0,fTransformCenter[0],fTransformCenter[1]);
                    int ir = floor( (radius-fRangeParamSpace[0][0])/fBinSizeParamSpace[0] );
                    auto paramPoint = GetParamPoint(ir,it);
                    auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,paramPoint);
                    if (weight>0) {
                        fIdxSelectedR = ir;
                        fIdxSelectedT = it;
                        fParamData[ir][it] = fParamData[ir][it] + weight;
                    }
                }

                else if (fCorrelateType==kCorrelateBoxBand)
                {
                    int irMax = -INT_MAX;
                    int irMin = INT_MAX;
                    for (auto iImageCorner : {1,2,3,4})
                    {
                        auto radius = imagePoint -> EvalR(iImageCorner,theta0,fTransformCenter[0],fTransformCenter[1]);
                        int ir = floor( (radius-fRangeParamSpace[0][0])/fBinSizeParamSpace[0] );
                        if (irMax<ir) irMax = ir;
                        if (irMin>ir) irMin = ir;
                    }
                    if (irMax>=fNumBinsParamSpace[0]) irMax = fNumBinsParamSpace[0] - 1;
                    if (irMin<0) irMin = 0;
                    for (int ir=irMin; ir<=irMax; ++ir) {
                        auto paramPoint = GetParamPoint(ir,it);
                        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,paramPoint);
                        if (weight>0) {
                            fIdxSelectedR = ir;
                            fIdxSelectedT = it;
                            fParamData[ir][it] = fParamData[ir][it] + weight;
                        }
                    }
                }

                else if (fCorrelateType==kCorrelateBoxRibbon)
                {
                    int irMax = -INT_MAX;
                    int irMin = INT_MAX;
                    for (auto iImageCorner : {1,2,3,4})
                    {
                        for (auto theta : {theta1, theta2})
                        {
                            auto radius = imagePoint -> EvalR(iImageCorner,theta,fTransformCenter[0],fTransformCenter[1]);
                            int ir = floor( (radius-fRangeParamSpace[0][0])/fBinSizeParamSpace[0] );
                            if (irMax<ir) irMax = ir;
                            if (irMin>ir) irMin = ir;
                        }
                    }
                    if (irMax>=fNumBinsParamSpace[0]) irMax = fNumBinsParamSpace[0] - 1;
                    if (irMin<0) irMin = 0;
                    for (int ir=irMin; ir<=irMax; ++ir) {
                        auto paramPoint = GetParamPoint(ir,it);
                        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,paramPoint);
                        if (weight>0) {
                            fIdxSelectedR = ir;
                            fIdxSelectedT = it;
                            fParamData[ir][it] = fParamData[ir][it] + weight;
                        }
                    }
                }

                else if (fCorrelateType==kCorrelateBoxLine)
                {
                    int irMax = -INT_MAX;
                    int irMin = INT_MAX;
                    for (auto iImageCorner : {1,2,3,4}) {
                        auto radius = imagePoint -> EvalR(iImageCorner,theta0,fTransformCenter[0],fTransformCenter[1]);
                        int ir = floor( (radius-fRangeParamSpace[0][0])/fBinSizeParamSpace[0] );
                        if (irMax<ir) irMax = ir;
                        if (irMin>ir) irMin = ir;
                    }
                    if (irMax>=fNumBinsParamSpace[0]) irMax = fNumBinsParamSpace[0] - 1;
                    if (irMin<0) irMin = 0;
                    for (int ir=irMin; ir<=irMax; ++ir) {
                        auto paramPoint = GetParamPoint(ir,it);
                        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,paramPoint);
                        if (weight>0) {
                            fIdxSelectedR = ir;
                            fIdxSelectedT = it;
                            fParamData[ir][it] = fParamData[ir][it] + weight;
                        }
                    }
                }

            } // image

        }
    }
    //else {
    //}
}

//#define DEBUG_SAME_MAX

LKParamPointRT* LKHTLineTracker::FindNextMaximumParamPoint()
{
    fIdxSelectedR = -1;
    fIdxSelectedT = -1;
    double maxValue = -1;
    for (auto ir=0; ir<fNumBinsParamSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsParamSpace[1]; ++it) {
            if (maxValue<fParamData[ir][it]) {
                fIdxSelectedR = ir;
                fIdxSelectedT = it;
                maxValue = fParamData[ir][it];
            }
        }
    }
#ifdef DEBUG_SAME_MAX
    lk_debug << "max value is " << maxValue << endl;
    int countMaxValue = 0;
    for (auto ir=0; ir<fNumBinsParamSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsParamSpace[1]; ++it) {
            if (maxValue==fParamData[ir][it])
                countMaxValue++;
        }
    }
    lk_debug << "count same max value : " << countMaxValue << " (" << maxValue << ")" << endl;
#endif
    if (fIdxSelectedR<0) {
        fParamPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],-1,0,-1,0,-1);
        return fParamPoint;
    }

    auto paramPoint = GetParamPoint(fIdxSelectedR,fIdxSelectedT);
    return paramPoint;
}

/*
LKParamPointRT* LKHTLineTracker::FindNextMaximumParamPoint2()
{
    if (fGraphPathToMaxWeight==nullptr)
        fGraphPathToMaxWeight = new TGraph();
    else
        fGraphPathToMaxWeight -> Set(0);
    fGraphPathToMaxWeight -> SetPoint(fGraphPathToMaxWeight->GetN(),
            fRangeParamSpace[1][0]+(fIdxSelectedT+0.5)*fBinSizeParamSpace[1],
            fRangeParamSpace[0][0]+(fIdxSelectedR+0.5)*fBinSizeParamSpace[0]);
    double maxValue = fParamData[fIdxSelectedR][fIdxSelectedT];
    lk_debug << fIdxSelectedT << " " << fIdxSelectedR << " "
             << fRangeParamSpace[1][0]+(fIdxSelectedT+0.5)*fBinSizeParamSpace[1] << " "
             << fRangeParamSpace[0][0]+(fIdxSelectedR+0.5)*fBinSizeParamSpace[0] << " " << maxValue << endl;
    double val;
    int irAtMax, itAtMax, irNew, itNew;
    bool foundNewMaximum = false;
    while (true)
    {
        foundNewMaximum = false;
        for (auto irOff : {-1,0,1}) {
            for (auto itOff : {-1,0,1})
            {
                if (irOff==0&&itOff==0) continue;
                irNew = fIdxSelectedR + irOff;
                itNew = fIdxSelectedT + itOff;
                if (irNew<0 || irNew>=fNumBinsParamSpace[0]) continue;
                if (itNew==-1) itNew = fNumBinsParamSpace[1] - 1;
                else if (itNew==fNumBinsParamSpace[1]) itNew = 0;

                val = fParamData[irNew][itNew];
                lk_debug << irNew << " " << itNew << " " << val << endl;
                if (val>maxValue) {
                    foundNewMaximum = true;
                    irAtMax = irNew;
                    itAtMax = itNew;
                    maxValue = val;
                }
            }
        }
        if (foundNewMaximum) {
            fIdxSelectedR = irAtMax;
            fIdxSelectedT = itAtMax;
            lk_debug << fIdxSelectedT << " " << fIdxSelectedR << " "
                     << fRangeParamSpace[1][0]+(fIdxSelectedT+0.5)*fBinSizeParamSpace[1] << " "
                     << fRangeParamSpace[0][0]+(fIdxSelectedR+0.5)*fBinSizeParamSpace[0] << " " << maxValue << endl;
            fGraphPathToMaxWeight -> SetPoint(fGraphPathToMaxWeight->GetN(),
                    fRangeParamSpace[1][0]+(fIdxSelectedT+0.5)*fBinSizeParamSpace[1],
                    fRangeParamSpace[0][0]+(fIdxSelectedR+0.5)*fBinSizeParamSpace[0]);
        }
        else
            break;
    }

    auto paramPoint = GetParamPoint(fIdxSelectedR,fIdxSelectedT);
    return paramPoint;
}
*/

void LKHTLineTracker::CleanLastParamPoint(double rWidth, double tWidth)
{
    if (rWidth<0) rWidth = (fRangeParamSpace[0][1]-fRangeParamSpace[0][0])/20;
    if (tWidth<0) tWidth = (fRangeParamSpace[1][1]-fRangeParamSpace[1][0])/20;;
    int numBinsHalfR = std::floor(rWidth/fBinSizeParamSpace[0]/2.);
    int numBinsHalfT = std::floor(tWidth/fBinSizeParamSpace[1]/2.);
    if (rWidth==0) numBinsHalfR = 0;
    if (tWidth==0) numBinsHalfT = 0;

    for (int iOffR=-numBinsHalfR; iOffR<=numBinsHalfR; ++iOffR) {
        int ir = fIdxSelectedR + iOffR;
        if (ir<0||ir>=fNumBinsParamSpace[0])
            continue;
        for (int iOffT=-numBinsHalfT; iOffT<=numBinsHalfT; ++iOffT) {
            int it = fIdxSelectedT + iOffT;
            if (it<0||it>=fNumBinsParamSpace[1])
                continue;
            fParamData[ir][it] = 0;
        }
    }
}

LKParamPointRT* LKHTLineTracker::ReinitializeFromLastParamPoint()
{
    auto ir1 = fIdxSelectedR - 1;
    auto ir2 = fIdxSelectedR + 1;
    auto it1 = fIdxSelectedT - 1;
    auto it2 = fIdxSelectedT + 1;
    if (ir2>=fNumBinsParamSpace[0]) ir2 = fNumBinsParamSpace[0] - 1;
    if (ir1<0) ir1 = 0;
    if (it2>=fNumBinsParamSpace[1]) it2 = fNumBinsParamSpace[1] - 1;
    if (it1<0) ir1 = 0;

    double rMin = DBL_MAX;
    double rMax = -DBL_MAX;
    double tMin = DBL_MAX;
    double tMax = -DBL_MAX;
    double weightTotal = 0.;

    for (auto ir=ir1; ir<=ir2; ++ir) {
        for (auto it=it1; it<=it2; ++it) {
            auto paramPoint = GetParamPoint(ir,it);
            if (rMin>paramPoint->fRadius1) rMin = paramPoint -> fRadius1;
            if (rMax<paramPoint->fRadius2) rMax = paramPoint -> fRadius2;
            if (tMin>paramPoint->fTheta1 ) tMin = paramPoint -> fTheta1;
            if (tMax<paramPoint->fTheta2 ) tMax = paramPoint -> fTheta2;
            weightTotal += paramPoint -> fWeight;
        }
    }

    fRangeParamSpace[0][0] = rMin;
    fRangeParamSpace[0][1] = rMax;
    fRangeParamSpace[1][0] = tMin;
    fRangeParamSpace[1][1] = tMax;
    fBinSizeParamSpace[0] = (fRangeParamSpace[0][1]-fRangeParamSpace[0][0])/fNumBinsParamSpace[0];
    fBinSizeParamSpace[1] = (fRangeParamSpace[1][1]-fRangeParamSpace[1][0])/fNumBinsParamSpace[1];
    fBinSizeMaxParamSpace = sqrt(fBinSizeParamSpace[0]*fBinSizeParamSpace[0] + fBinSizeParamSpace[1]*fBinSizeParamSpace[1]);

    fParamPoint -> SetPoint(fTransformCenter[0],fTransformCenter[1],rMin,tMin,rMax,tMax,weightTotal);

    if (fInitializedParamData) {
        for(int i = 0; i < fNumBinsParamSpace[0]; ++i)
            for(int j = 0; j < fNumBinsParamSpace[1]; ++j)
                fParamData[i][j] = 0;
    }

    return fParamPoint;
}

void LKHTLineTracker::RetransformFromLastParamPoint()
{
    ReinitializeFromLastParamPoint();
    Transform();
}

LKLinearTrack* LKHTLineTracker::FitTrackWithParamPoint(LKParamPointRT* paramPoint, double weightCut)
{
    if (weightCut==-1) {
        weightCut = 0.2;
    }
    auto track = (LKLinearTrack*) fTrackArray -> ConstructedAt(fNumLinearTracks);
    ++fNumLinearTracks;

    fLineFitter -> Reset();

    vector<int> vImagePointIdxs;
    for (int iImage=0; iImage<fNumImagePoints; ++iImage)
    {
        auto imagePoint = GetImagePoint(iImage);

        double distance = 0;
        if (fCorrelateType==kCorrelateBoxBand)
            distance = paramPoint -> CorrelateBoxBand(imagePoint);
        if (distance<0)
            continue;
        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,paramPoint);
        if (weight > weightCut) {
            vImagePointIdxs.push_back(iImage);
            fLineFitter -> PreAddPoint(imagePoint->fX0,imagePoint->fY0,0,imagePoint->fWeight);
        }
    }
    for (int iImage : vImagePointIdxs)
    {
        auto imagePoint = PopImagePoint(iImage);
        auto weight = fWeightingFunction -> EvalFromPoints(imagePoint,paramPoint);
        fLineFitter -> AddPoint(imagePoint->fX0,imagePoint->fY0,0,imagePoint->fWeight);
    }
    if (fLineFitter->GetNumPoints()<fCutNumTrackHits) {
        lk_debug << "return because " << fLineFitter->GetNumPoints() << " < " << fCutNumTrackHits << endl;
        return track;
    }

    bool fitted = fLineFitter -> FitLine();
    if (fitted==false) {
        lk_debug << "return from fitter" << endl;
        return track;
    }

    auto centroid = fLineFitter -> GetCentroid();

    //auto dx = fRangeImageSpace[1][0] - fRangeImageSpace[0][0];
    //auto dy = fRangeImageSpace[1][0] - fRangeImageSpace[0][0];
    auto dx = fRangeImageSpace[0][0] - fRangeImageSpace[0][1];
    auto dy = fRangeImageSpace[1][0] - fRangeImageSpace[1][1];
    auto size = sqrt(dx*dx + dy*dy)/2;
    auto direction = fLineFitter->GetDirection();

    track -> SetLine(centroid - size*direction, centroid + size*direction);
    //track -> SetLine(centroid, centroid + size*direction);
    track -> SetRMS(fLineFitter->GetRMSLine());

    //lk_debug << fRangeImageSpace[1][0] << " " << fRangeImageSpace[0][0] << endl;
    //lk_debug << fNumImagePoints << " " << dx << " " << dy << endl;
    //direction.Print();
    //(centroid - size*direction).Print();
    if (fUsingImagePointArray) {
        fImagePointArray -> Compress();
        fNumImagePoints = fImagePointArray -> GetEntriesFast();
    }

    return track;
}

void LKHTLineTracker::DrawAllParamLines(int i, bool drawRadialLine)
{
    vector<int> corners = {0,1,2,3,4};
    if (i>=0) {
        corners.clear();
        corners.push_back(i);
    }

    for (auto iCorner : corners)
    {
        for (auto ir=0; ir<fNumBinsParamSpace[0]; ++ir)
        {
            for (auto it=0; it<fNumBinsParamSpace[1]; ++it)
            {
                auto paramPoint = GetParamPoint(ir,it);
                auto graph1 = paramPoint -> GetLineInImageSpace(iCorner,fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
                if (iCorner==0)
                    graph1 -> SetLineStyle(2);
                graph1 -> Draw("samel");
                if (drawRadialLine) {
                    auto graph2 = paramPoint -> GetRadialLineInImageSpace(iCorner,0.25*fBinSizeMaxImageSpace);
                    graph2 -> Draw("samel");
                }
            }
        }
    }
}

void LKHTLineTracker::DrawAllParamBands()
{
    for (auto ir=0; ir<fNumBinsParamSpace[0]; ++ir)
    {
        for (auto it=0; it<fNumBinsParamSpace[1]; ++it)
        {
            auto paramPoint = GetParamPoint(ir,it);
            auto graph = paramPoint -> GetBandInImageSpace(fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
            graph -> SetLineColor(kRed);
            graph -> Draw("samel");
        }
    }
}

TH2D* LKHTLineTracker::GetHistImageSpace(TString name, TString title)
{
    if (name.IsNull()) name = "histImageSpace";
    TString correlatorName = GetCorrelatorName();
    if (title.IsNull()) title = Form("%s (%dx%d), TC (x,y) = (%.2f, %.2f);x;y", correlatorName.Data(), fNumBinsParamSpace[0], fNumBinsParamSpace[1], fTransformCenter[0],fTransformCenter[1]);
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

TGraphErrors* LKHTLineTracker::GetDataGraphImageSapce()
{
    //if (fGraphImageData==nullptr) {
        fGraphImageData = new TGraphErrors();
        //fGraphImageData -> SetMarkerSize(0.5);
    //}
    //fGraphImageData -> Set(0);
    for (auto iPoint=0; iPoint<fNumImagePoints; ++iPoint) {
        auto imagePoint = (LKImagePoint*) fImagePointArray -> At(iPoint);
        fGraphImageData -> SetPoint(iPoint, imagePoint->GetCenterX(), imagePoint->GetCenterY());
        fGraphImageData -> SetPointError(iPoint, (imagePoint->GetX2()-imagePoint->GetX1())/2., (imagePoint->GetY2()-imagePoint->GetY1())/2.);
    }
    return fGraphImageData;
}

TH2D* LKHTLineTracker::GetHistParamSpace(TString name, TString title)
{
    if (name.IsNull()) name = "histParamSpace";
    TString correlatorName = GetCorrelatorName();
    if (title.IsNull()) title = Form("%s (%dx%d);#theta (degrees);Radius", correlatorName.Data(), fNumBinsParamSpace[0], fNumBinsParamSpace[1]);
    auto hist = new TH2D(name,title, fNumBinsParamSpace[1],fRangeParamSpace[1][0],fRangeParamSpace[1][1],fNumBinsParamSpace[0],fRangeParamSpace[0][0],fRangeParamSpace[0][1]);
    for (auto ir=0; ir<fNumBinsParamSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsParamSpace[1]; ++it) {
            if (fParamData[ir][it]>0) {
                hist -> SetBinContent(it+1,ir+1,fParamData[ir][it]);
            }
        }
    }
    hist -> GetXaxis() -> SetTitleOffset(1.25);
    hist -> GetYaxis() -> SetTitleOffset(1.60);
    return hist;
}

void LKHTLineTracker::DrawToPads(TVirtualPad* padImage, TVirtualPad* padParam)
{
    fPadImage = (TPad *) padImage -> cd();
    fPadParam = (TPad *) padParam -> cd();

    fPadImage -> cd();
    fHistImage = GetHistImageSpace("histImageSpace");
    fHistImage -> Draw("colz");
    auto graph = GetDataGraphImageSapce();
    graph -> Draw("samepz");

    fPadParam -> cd();
    //fPadParam -> AddExec("ex", "LKHTLineTracker::ClickPadParam()");
    fHistParam = GetHistParamSpace("histParamSpace");
    fHistParam -> Draw("colz");
}

void LKHTLineTracker::ClickPadParam(int iPlane)
{
    TObject* select = ((TCanvas*)gPad) -> GetClickSelected();
    if (select == nullptr)
        return;

    bool isNotH2 = !(select -> InheritsFrom(TH2::Class()));
    if (isNotH2)
        return;

    TH2D* hist = (TH2D*) select;

    Int_t xEvent = gPad -> GetEventX();
    Int_t yEvent = gPad -> GetEventY();
    Float_t xAbs = gPad -> AbsPixeltoX(xEvent);
    Float_t yAbs = gPad -> AbsPixeltoY(yEvent);
    Double_t xOnClick = gPad -> PadtoX(xAbs);
    Double_t yOnClick = gPad -> PadtoY(yAbs);

    //Int_t bin = hist -> FindBin(xOnClick, yOnClick);
    //gPad -> SetUniqueID(bin);
    //gPad -> GetCanvas() -> SetClickSelected(NULL);

    //int binx = fHistParam -> GetXaxis() -> FindBin(xClick);
    //int biny = fHistParam -> GetYaxis() -> FindBin(yClick);

    //auto paramPoint = GetParamPoint(binx-1,biny-1);
    //auto graphBand = paramPoint -> GetBandInImageSpace(fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);

    //fPadImage -> cd();
    //graphBand -> Draw("samel");
}