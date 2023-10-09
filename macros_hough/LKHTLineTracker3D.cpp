#include "LKHTTRacker3D.h"

ClassImp(LKHTLineTracker3D);

LKHTLineTracker3D::LKHTLineTracker3D()
{
    fLineFitter = LKODRFitter::GetFitter();
    fImagePoint = new LKImagePoint();
    fParamPoint = new LKParamPointRT();
    fImagePointArray = new TClonesArray("LKImagePoint",100);
    fTrackArray = new TClonesArray("LKLinearTrack",20);
    Clear();
}

bool LKHTLineTracker3D::Init()
{
    return true;
}

void LKHTLineTracker3D::Reset()
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

void LKHTLineTracker3D::Clear(Option_t *option)
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

    fNumImagePoints = 0;
    fImagePointArray -> Clear("C");

    if (fInitializedParamData) {
        for(int i = 0; i < fNumBinsParamSpace[0]; ++i)
            for(int j = 0; j < fNumBinsParamSpace[1]; ++j)
                fParamData[i][j] = 0;
    }
}

void LKHTLineTracker3D::Print(Option_t *option) const
{
}

void SetImageSpaceRange(int nx, double x1, double x2, int ny, double y1, double y2, int nz, double z1, double z2)
{
    fNumBinsImageSpace[0] = nx;
    fBinSizeImageSpace[0] = (x2-x1)/nx;
    fRangeImageSpace[0][0] = x1;
    fRangeImageSpace[0][1] = x2;

    fNumBinsImageSpace[1] = ny;
    fBinSizeImageSpace[1] = (y2-y1)/ny;
    fRangeImageSpace[1][0] = y1;
    fRangeImageSpace[1][1] = y2;

    fNumBinsImageSpace[2] = nz;
    fBinSizeImageSpace[2] = (z2-z1)/nz;
    fRangeImageSpace[2][0] = z1;
    fRangeImageSpace[2][1] = z2;

    double sumBinSizes = 0;
    for (auto i : {0,1,2,3})
        sumBinSizes = fBinSizeImageSpace[0]*fBinSizeImageSpace[0];
    fBinSizeMaxImageSpace = sqrt(sumBinSizes);
    if (fMaxWeightingDistance==0)
        fMaxWeightingDistance = fBinSizeMaxImageSpace;
}

void LKHTLineTracker3D::SetParamSpaceBins(int ny, int nz, int nt, int np);
{
    if (fInitializedParamData) {
        e_warning << "hough data is already initialized!" << endl;
        return;
    }

    double y1 = DBL_MAX;
    double y2 = -DBL_MAX;
    double z1 = DBL_MAX;
    double z2 = -DBL_MAX;
    double t1 = DBL_MAX;
    double t2 = -DBL_MAX;
    double p1 = DBL_MAX;
    double p2 = -DBL_MAX;
    double x0, y0, z0;
    //double r0;
    TVector3 v3Diff;
    for (auto iCorner : {0,1,2,3,4,5,6,7})
    {
        if (iCorner==0) { x0 = fRangeImageSpace[0][0]; y0 = fRangeImageSpace[1][0]; z0 = fRangeImageSpace[2][0]; }
        if (iCorner==1) { x0 = fRangeImageSpace[0][0]; y0 = fRangeImageSpace[1][1]; z0 = fRangeImageSpace[2][0]; }
        if (iCorner==2) { x0 = fRangeImageSpace[0][1]; y0 = fRangeImageSpace[1][0]; z0 = fRangeImageSpace[2][0]; }
        if (iCorner==3) { x0 = fRangeImageSpace[0][1]; y0 = fRangeImageSpace[1][1]; z0 = fRangeImageSpace[2][0]; }
        if (iCorner==4) { x0 = fRangeImageSpace[0][0]; y0 = fRangeImageSpace[1][0]; z0 = fRangeImageSpace[2][1]; }
        if (iCorner==5) { x0 = fRangeImageSpace[0][0]; y0 = fRangeImageSpace[1][1]; z0 = fRangeImageSpace[2][1]; }
        if (iCorner==6) { x0 = fRangeImageSpace[0][1]; y0 = fRangeImageSpace[1][0]; z0 = fRangeImageSpace[2][1]; }
        if (iCorner==7) { x0 = fRangeImageSpace[0][1]; y0 = fRangeImageSpace[1][1]; z0 = fRangeImageSpace[2][1]; }
        if (y0<y1) y1 = y0;
        if (y0>y2) y2 = y0;
        if (z0<z1) z1 = z0;
        if (z0>z2) z2 = z0;
        //if (t0<t1) t1 = t0;
        //if (t0>t2) t2 = t0;
        //if (p0<p1) p1 = p0;
        //if (p0>p2) p2 = p0;
    }
    t1 = 0;
    t2 = +180;
    p1 = 0;
    p2 = +360;

    SetParamSpaceRange(ny,y1,y2, nz,z1,z2, nt,t1,t2, np,p1,p2);
}

void LKHTLineTracker3D::SetParamSpaceRange(int ny, double y1, double y2, int nz, double z1, double z2, int nt, double t1, double t2, int np, double p1, double p2);
{
    if (fInitializedParamData) {
        e_warning << "hough data is already initialized!" << endl;
        return;
    }

    e_info << "Initializing hough space with "
        << "y = (" << ny << " | " <<  y1 << ", " << y2 << ") "
        << "z = (" << nz << " | " <<  z1 << ", " << z2 << ") "
        << "t = (" << nt << " | " <<  t1 << ", " << t2 << ") "
        << "p = (" << np << " | " <<  p1 << ", " << p2 << ")" << endl;

    fRangeParamSpaceInit[0][0] = r1;
    fRangeParamSpaceInit[0][1] = r2;
    fRangeParamSpaceInit[1][0] = t1;
    fRangeParamSpaceInit[1][1] = t2;
    fRangeParamSpaceInit[2][0] = p1;
    fRangeParamSpaceInit[2][1] = p2;

    fNumBinsParamSpace[0] = ny;
    fBinSizeParamSpace[0] = (y2-y1)/ny;
    fRangeParamSpace[0][0] = y1;
    fRangeParamSpace[0][1] = y2;

    fNumBinsParamSpace[1] = nz;
    fBinSizeParamSpace[1] = (z2-z1)/nz;
    fRangeParamSpace[1][0] = z1;
    fRangeParamSpace[1][1] = z2;

    fNumBinsParamSpace[2] = nt;
    fBinSizeParamSpace[2] = (t2-t1)/nt;
    fRangeParamSpace[2][0] = t1;
    fRangeParamSpace[2][1] = t2;

    fNumBinsParamSpace[3] = np;
    fBinSizeParamSpace[3] = (p2-p1)/np;
    fRangeParamSpace[3][0] = p1;
    fRangeParamSpace[3][1] = p2;

    double sumBinSizes = 0;
    for (auto i : {0,1,2,3})
        sumBinSizes = fBinSizeParamSpace[0]*fBinSizeParamSpace[0];
    fBinSizeMaxParamSpace = sqrt(sumBinSizes);

    fParamData = new double***[fNumBinsParamSpace[0]];
    for (int i = 0; i < fNumBinsParamSpace[0]; i++) {
        fParamData[i] = new double**[fNumBinsParamSpace[1]];
        for (int j = 0; j < fNumBinsParamSpace[1]; j++) {
            fParamData[i][j] = new double*[fNumBinsParamSpace[2]];
            for (int k = 0; k < fNumBinsParamSpace[2]; k++) {
                fParamData[i][j][k] = new double[fNumBinsParamSpace[3]];
                for(int l = 0; l < fNumBinsParamSpace[3]; ++l) {
                    fParamData[i][j][k][l] = 0;
                }
            }
        }
    }

    fInitializedParamData = true;
}

void LKHTLineTracker3D::AddImagePoint(double x, double xError, double y, double yError, double z, double zError, double weight)
{
    double x1 = x-xError;
    double x2 = x+xError;
    double y1 = y-yError;
    double y2 = y+yError;
    double z1 = z-zError;
    double z2 = z+zError;
    auto imagePoint = (LKImagePoint3D*) fImagePointArray -> ConstructedAt(fNumImagePoints);
    imagePoint -> SetPoint(x1,y1,x2,y2,z1,z2,weight);
    ++fNumImagePoints;
}

void LKHTLineTracker3D::AddImagePointBox(double x1, double y1, double x2, double y2, double z1, double z2, double weight)
{
    auto imagePoint = (LKImagePoint3D*) fImagePointArray -> ConstructedAt(fNumImagePoints);
    imagePoint -> SetPoint(x1,y1,x2,y2,z1,z2,weight);
    ++fNumImagePoints;
}

TString LKHTLineTracker3D::GetCorrelatorName()
{
    TString correlatorName;
    if (fCorrelateType==kCorrelatePointBand)  correlatorName = "Point-Band";
    else if (fCorrelateType==kCorrelateBoxLine)  correlatorName = "Box-Line";
    else if (fCorrelateType==kCorrelateBoxBand)  correlatorName = "Box-Band";
    else if (fCorrelateType==kCorrelateBoxRBand)  correlatorName = "Box-RBand";
    else if (fCorrelateType==kCorrelateDistance) correlatorName = "Distance";
    return correlatorName;
}

void LKHTLineTracker3D::SetCorrelatePointBand()
{
    fCorrelateType = kCorrelatePointBand;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHTLineTracker3D::SetCorrelateBoxLine()
{
    fCorrelateType = kCorrelateBoxLine;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHTLineTracker3D::SetCorrelateBoxRBand()
{
    fCorrelateType = kCorrelateBoxRBand;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHTLineTracker3D::SetCorrelateBoxBand()
{
    fCorrelateType = kCorrelateBoxBand;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}

void LKHTLineTracker3D::SetCorrelateDistance()
{
    fCorrelateType = kCorrelateDistance;
    if (fWeightingFunction==nullptr)
        fWeightingFunction = new LKHoughWFInverse();
}


LKImagePoint* LKHTLineTracker3D::GetImagePoint(int i)
{
    auto imagePoint = (LKImagePoint*) fImagePointArray -> At(i);
    return imagePoint;
}

LKImagePoint* LKHTLineTracker3D::PopImagePoint(int i)
{
    auto imagePoint = (LKImagePoint*) fImagePointArray -> At(i);
    fImagePointArray -> RemoveAt(i);
    return imagePoint;
}

LKParamPointRT* LKHTLineTracker3D::GetParamPoint(int iy, int iz, int it, int ip);
{
    double y1 = fRangeParamSpace[0][0] + ir*fBinSizeParamSpace[0];
    double y2 = fRangeParamSpace[0][0] + (ir+1)*fBinSizeParamSpace[0];
    double z1 = fRangeParamSpace[1][0] + ir*fBinSizeParamSpace[1];
    double z2 = fRangeParamSpace[1][0] + (ir+1)*fBinSizeParamSpace[1];
    double t1 = fRangeParamSpace[2][0] + it*fBinSizeParamSpace[2];
    double t2 = fRangeParamSpace[2][0] + (it+1)*fBinSizeParamSpace[2];
    double p1 = fRangeParamSpace[3][0] + ip*fBinSizeParamSpace[3];
    double p2 = fRangeParamSpace[3][0] + (ip+1)*fBinSizeParamSpace[3];
    fParamPoint -> SetPoint(fTransformCenter, y1,z1,t1,p1, y2,z2,t2,p2, fParamData[iy][iz][it][ip]);
    return fParamPoint;
}

void LKHTLineTracker3D::Transform()
{
    for (auto it=0; it<fNumBinsParamSpace[1]; ++it)
    {
        double theta0 = fRangeParamSpace[1][0] + (it+0.5)*fBinSizeParamSpace[1];
        double theta1 = fRangeParamSpace[1][0] + it*fBinSizeParamSpace[1];
        double theta2 = fRangeParamSpace[1][0] + (it+1)*fBinSizeParamSpace[1];

        for (int iImage=0; iImage<fNumImagePoints; ++iImage)
        {
            auto imagePoint = GetImagePoint(iImage);

            if (fCorrelateType==kCorrelateBoxRBand)
            {
                /*
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
                */
            }
        } // image

    }
}

LKParamPointRT* LKHTLineTracker3D::FindNextMaximumParamPoint()
{
    fIdxSelectedY = -1;
    fIdxSelectedZ = -1;
    fIdxSelectedT = -1;
    fIdxSelectedP = -1;
    double maxValue = -1;
    for (auto iy=0; iy<fNumBinsParamSpace[0]; ++iy)
    for (auto iz=0; iz<fNumBinsParamSpace[1]; ++iz)
    for (auto it=0; it<fNumBinsParamSpace[2]; ++it)
    for (auto ip=0; ip<fNumBinsParamSpace[3]; ++ip)
    {
        if (maxValue<fParamData[iy][iz][it][ip]) {
            fIdxSelectedY = iy;
            fIdxSelectedZ = iz;
            fIdxSelectedT = it;
            fIdxSelectedP = ip;
            maxValue = fParamData[iy][iz][it][ip];
        }
    }
    if (fIdxSelectedY<0) {
        fParamPoint -> SetPoint(fTransformCenter, 0,0,0,0, 0,0,0,0, -1);
        return fParamPoint;
    }

    auto paramPoint = GetParamPoint(fIdxSelectedY,fIdxSelectedZ,fIdxSelectedT,fIdxSelectedP);
    return paramPoint;
}

/*
LKParamPointRT* LKHTLineTracker3D::FindNextMaximumParamPoint2()
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

void LKHTLineTracker3D::CleanLastParamPoint(double yWidth, double zWidth, double tWidth, double pWidth)
{
    double widths[] = {yWidth, zWidth, tWidth, pWidth};
    int numBinsHalf[i] = {0};
    for (auto i : {0,1,2,3})
    {
        if (widths[i]<0) widths[i] = (fRangeParamSpace[i][1]-fRangeParamSpace[i][0])/20;
        numBinsHalf[i] = std::floor(yWidth/fBinSizeParamSpace[0]/2.);
        if (widths[i]==0) numBinsHalf[i] = 0;
    }

    for (int iOff0=-numBinsHalf[0]; iOff0<=numBinsHalf[0]; ++iOff0) { int i0 = fIdxSelectedY + iOff0; if (ir<0||ir>=fNumBinsParamSpace[0]) continue;
    for (int iOff1=-numBinsHalf[1]; iOff1<=numBinsHalf[1]; ++iOff1) { int i1 = fIdxSelectedZ + iOff1; if (ir<0||ir>=fNumBinsParamSpace[0]) continue;
    for (int iOff2=-numBinsHalf[2]; iOff2<=numBinsHalf[2]; ++iOff2) { int i2 = fIdxSelectedT + iOff2; if (ir<0||ir>=fNumBinsParamSpace[0]) continue;
    for (int iOff3=-numBinsHalf[3]; iOff3<=numBinsHalf[3]; ++iOff3) { int i3 = fIdxSelectedP + iOff3; if (it<0||it>=fNumBinsParamSpace[1]) continue;
            fParamData[iy][iz][it][ip] = 0;
    }
    }
    }
    }
}

LKParamPointRT* LKHTLineTracker3D::ReinitializeFromLastParamPoint()
{
    auto iy1 = fIdxSelectedY - 1;
    auto iy2 = fIdxSelectedY + 1;
    auto iz1 = fIdxSelectedZ - 1;
    auto iz2 = fIdxSelectedZ + 1;
    auto it1 = fIdxSelectedT - 1;
    auto it2 = fIdxSelectedT + 1;
    auto ip1 = fIdxSelectedP - 1;
    auto ip2 = fIdxSelectedP + 1;
    if (iy2>=fNumBinsParamSpace[0]) iy2 = fNumBinsParamSpace[0] - 1;
    if (iz2>=fNumBinsParamSpace[1]) iz2 = fNumBinsParamSpace[1] - 1;
    if (it2>=fNumBinsParamSpace[2]) it2 = fNumBinsParamSpace[2] - 1;
    if (ip2>=fNumBinsParamSpace[3]) ip2 = fNumBinsParamSpace[3] - 1;
    if (iy1<0) iy1 = 0;
    if (iz1<0) iz1 = 0;
    if (it1<0) it1 = 0;
    if (ip1<0) ip1 = 0;

    double yMin = +DBL_MAX;
    double yMax = -DBL_MAX;
    double zMin = +DBL_MAX;
    double zMax = -DBL_MAX;
    double tMin = +DBL_MAX;
    double tMax = -DBL_MAX;
    double pMin = +DBL_MAX;
    double pMax = -DBL_MAX;
    double weightTotal = 0.;

    for (auto iy=iy1; iy<=iy2; ++iy)
        for (auto iz=iz1; iz<=iz2; ++iz)
            for (auto it=it1; it<=it2; ++it)
                for (auto ip=ip1; ip<=ip2; ++ip)
                {
                    auto paramPoint = GetParamPoint(iy,iz,it,ip);
                    if (yMin>paramPoint->fY1) yMin = paramPoint -> fY1;
                    if (yMax<paramPoint->fY2) yMax = paramPoint -> fY2;
                    if (zMin>paramPoint->fZ1) zMin = paramPoint -> fZ1;
                    if (zMax<paramPoint->fZ2) zMax = paramPoint -> fZ2;
                    if (tMin>paramPoint->fT1) tMin = paramPoint -> fk1;
                    if (tMax<paramPoint->fT2) tMax = paramPoint -> fT2;
                    if (pMin>paramPoint->fP1) pMin = paramPoint -> fP1;
                    if (pMax<paramPoint->fP2) pMax = paramPoint -> fP2;
                    weightTotal += paramPoint -> fWeight;
                }

    fRangeParamSpace[0][0] = yMin;
    fRangeParamSpace[0][1] = yMax;
    fRangeParamSpace[1][0] = zMin;
    fRangeParamSpace[1][1] = zMax;
    fRangeParamSpace[2][0] = tMin;
    fRangeParamSpace[2][1] = tMax;
    fRangeParamSpace[3][0] = pMin;
    fRangeParamSpace[3][1] = pMax;
    fBinSizeParamSpace[0] = (fRangeParamSpace[0][1]-fRangeParamSpace[0][0])/fNumBinsParamSpace[0];
    fBinSizeParamSpace[1] = (fRangeParamSpace[1][1]-fRangeParamSpace[1][0])/fNumBinsParamSpace[1];
    fBinSizeParamSpace[2] = (fRangeParamSpace[2][1]-fRangeParamSpace[2][0])/fNumBinsParamSpace[2];
    fBinSizeParamSpace[3] = (fRangeParamSpace[3][1]-fRangeParamSpace[3][0])/fNumBinsParamSpace[3];

    double sumBinSizes = 0;
    for (auto i : {0,1,2,3})
        sumBinSizes = fBinSizeParamSpace[0]*fBinSizeParamSpace[0];
    fBinSizeMaxParamSpace = sqrt(sumBinSizes);

    fParamPoint -> SetPoint(fTransformCenter, yMin,zMin,tMin,pMin, yMax,zMax,tMax,pMax, weightTotal);

    for (int i = 0; i < fNumBinsParamSpace[0]; i++) {
        for (int j = 0; j < fNumBinsParamSpace[1]; j++) {
            for (int k = 0; k < fNumBinsParamSpace[2]; k++) {
                for(int l = 0; l < fNumBinsParamSpace[3]; ++l) {
                    fParamData[i][j][k][l] = 0;
                }
            }
        }
    }

    return fParamPoint;
}

void LKHTLineTracker3D::RetransformFromLastParamPoint()
{
    ReinitializeFromLastParamPoint();
    Transform();
}

/*
LKLinearTrack* LKHTLineTracker3D::FitTrackWithParamPoint(LKParamPointRT* paramPoint, double weightCut)
{
    if (weightCut==-1) {
        weightCut = 0.2;
    }
    //lk_debug << weightCut << endl;
    auto track = (LKLinearTrack*) fTrackArray -> ConstructedAt(fNumLinearTracks);
    ++fNumLinearTracks;

    fLineFitter -> Reset();

    vector<int> vImagePointIdxs;
    for (int iImage=0; iImage<fNumImagePoints; ++iImage) {
        auto imagePoint = GetImagePoint(iImage);
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

    fImagePointArray -> Compress();
    fNumImagePoints = fImagePointArray -> GetEntriesFast();

    return track;
}
*/

/*
TGraphErrors* LKHTLineTracker3D::GetDataGraphImageSapce()
{
    auto graph = new TGraphErrors();
    for (auto iPoint=0; iPoint<fNumImagePoints; ++iPoint) {
        auto imagePoint = (LKImagePoint*) fImagePointArray -> At(iPoint);
        graph -> SetPoint(iPoint, imagePoint->GetCenterX(), imagePoint->GetCenterY());
        graph -> SetPointError(iPoint, (imagePoint->GetX2()-imagePoint->GetX1())/2., (imagePoint->GetY2()-imagePoint->GetY1())/2.);
    }
    graph -> SetMarkerSize(0.5);
    return graph;
}

TH2D* LKHTLineTracker3D::GetHistImageSpace(TString name, TString title)
{
    if (name.IsNull()) name = "histImageSpace";
    TString correlatorName = GetCorrelatorName();
    if (title.IsNull()) title = Form("%s (%dx%d), TC (x,y) = (%.2f, %.2f);x;y", correlatorName.Data(), fNumBinsImageSpace[0], fNumBinsImageSpace[1], fTransformCenter[0],fTransformCenter[1]);
    auto hist = new TH2D(name,title, fNumBinsImageSpace[0],fRangeImageSpace[0][0],fRangeImageSpace[0][1],fNumBinsImageSpace[1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
    for (auto iPoint=0; iPoint<fNumImagePoints; ++iPoint) {
        auto imagePoint = (LKImagePoint*) fImagePointArray -> At(iPoint);
        hist -> Fill(imagePoint->GetCenterX(), imagePoint->GetCenterY(), imagePoint->fWeight);
    }
    hist -> GetXaxis() -> SetTitleOffset(1.25);
    hist -> GetYaxis() -> SetTitleOffset(1.60);
    return hist;
}

TH2D* LKHTLineTracker3D::GetHistParamSpace(TString name, TString title)
{
    if (name.IsNull()) name = "histParamSpace";
    TString correlatorName = GetCorrelatorName();
    if (title.IsNull()) title = Form("%s (%dx%d), TC (x,y) = (%.2f, %.2f);#theta (degrees);Radius", correlatorName.Data(), fNumBinsParamSpace[0], fNumBinsParamSpace[1], fTransformCenter[0],fTransformCenter[1]);
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

void LKHTLineTracker3D::DrawToPads(TVirtualPad* padImage, TVirtualPad* padParam)
{
    fPadImage = (TPad *) padImage -> cd();
    fPadParam = (TPad *) padParam -> cd();

    fPadImage -> cd();
    fHistImage = GetHistImageSpace("histImageSpace");
    fHistImage -> Draw("colz");
    auto graph = GetDataGraphImageSapce();
    graph -> Draw("samepz");

    fPadParam -> cd();
    //fPadParam -> AddExec("ex", "LKHTLineTracker3D::ClickPadParam()");
    fHistParam = GetHistParamSpace("histParamSpace");
    fHistParam -> Draw("colz");
}

void LKHTLineTracker3D::ClickPadParam(int iPlane)
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
*/
