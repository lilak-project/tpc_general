#include "LKHoughTransformTracker.h"

ClassImp(LKHoughTransformTracker);

LKHoughTransformTracker::LKHoughTransformTracker()
{
    Clear();
}

bool LKHoughTransformTracker::Init()
{
    return true;
}

void LKHoughTransformTracker::Clear(Option_t *option)
{
    TObject::Clear(option);
    fTransformCenter = TVector3();
    fNumBinsImageSpace[0] = 10;
    fNumBinsImageSpace[1] = 10;
    fNumBinsHoughSpace[0] = 10;
    fNumBinsHoughSpace[1] = 10;
    fBinSizeImageSpace[0] = 0;
    fBinSizeImageSpace[1] = 0;
    fBinSizeHoughSpace[0] = 0;
    fBinSizeHoughSpace[1] = 0;
    fRangeImageSpace[0][0] = 0;
    fRangeImageSpace[0][1] = 0;
    fRangeImageSpace[1][0] = 0;
    fRangeImageSpace[1][1] = 0;
    fRangeHoughSpace[0][0] = 0;
    fRangeHoughSpace[0][1] = 0;
    fRangeHoughSpace[1][0] = 0;
    fRangeHoughSpace[1][1] = 0;
    fImageIsPoints = true;
    fNumImagePoints = 0;
    fImagePointArray.clear();
    //fHoughData;
    //fImageData;
}

void LKHoughTransformTracker::Print(Option_t *option) const
{
}

void LKHoughTransformTracker::SetImageData(double** imageData)
{
    for (auto ix=0; ix<fNumBinsImageSpace[0]; ++ix)
        for (auto iy=0; iy<fNumBinsImageSpace[0]; ++iy)
            fImageData[ix][iy] = imageData[ix][iy];
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
}

void LKHoughTransformTracker::SetHoughSpaceBins(int nr, int nt)
{
    double r1 = DBL_MAX;
    double r2 = -DBL_MAX;
    double t1 = DBL_MAX;
    double t2 = -DBL_MAX;
    double x0, y0, r0, t0;
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
    fNumBinsHoughSpace[0] = nr;
    fBinSizeHoughSpace[0] = (r2-r1)/nr;
    fRangeHoughSpace[0][0] = r1;
    fRangeHoughSpace[0][1] = r2;

    fNumBinsHoughSpace[1] = nt;
    fBinSizeHoughSpace[1] = (t2-t1)/nt;
    fRangeHoughSpace[1][0] = t1;
    fRangeHoughSpace[1][1] = t2;

    fHoughData = new double*[fNumBinsHoughSpace[0]];
    for(int i = 0; i < fNumBinsHoughSpace[0]; ++i) {
        fHoughData[i] = new double[fNumBinsHoughSpace[1]];
        for(int j = 0; j < fNumBinsHoughSpace[1]; ++j)
            fHoughData[i][j] = 0; 
    }
}

void LKHoughTransformTracker::AddImagePoint(double x, double xError, double y, double yError, double weight)
{
    fImageIsPoints = true;
    double x1 = x-xError;
    double x2 = x+xError;
    double y1 = y-yError;
    double y2 = y+yError;
    //lk_debug << x1 << " " << x2 << " " << y1 << " " << y2 << endl;
    fImagePointArray.push_back(LKImagePoint(x1,y1,x2,y2,weight));
    fNumImagePoints = fImagePointArray.size();
}

void LKHoughTransformTracker::AddImagePointBox(double x1, double y1, double x2, double y2, double weight)
{
    fImageIsPoints = true;
    fImagePointArray.push_back(LKImagePoint(x1,y1,x2,y2,weight));
    fNumImagePoints = fImagePointArray.size();
}

LKImagePoint LKHoughTransformTracker::GetImagePoint(int i)
{
    if (fImageIsPoints)
        return fImagePointArray[i];
    else {
        // TODO
        int count = 0;
        for (auto ix=0; ix<fNumBinsImageSpace[0]; ++ix) {
            for (auto iy=0; iy<fNumBinsImageSpace[1]; ++iy) {
                if (count==i) {
                    double x1 = fRangeImageSpace[0][0] + ix*fBinSizeImageSpace[0];
                    double x2 = fRangeImageSpace[0][0] + (ix+1)*fBinSizeImageSpace[0];
                    double y1 = fRangeImageSpace[1][0] + iy*fBinSizeImageSpace[1];
                    double y2 = fRangeImageSpace[1][0] + (iy+1)*fBinSizeImageSpace[1];
                    return LKImagePoint(x1,y1,x2,y2,fImageData[ix][iy]);
                }
                ++count;
            }
        }
    }
    return LKImagePoint();
}

LKHoughPointRT LKHoughTransformTracker::GetHoughPoint(int i)
{
    // TODO
    double dr = (fRangeHoughSpace[0][0] + fRangeHoughSpace[0][1]) / fNumBinsHoughSpace[0];
    double dt = (fRangeHoughSpace[1][0] + fRangeHoughSpace[1][1]) / fNumBinsHoughSpace[1];
    int count = 0;
    for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it) {
            if (count==i) {
                double r1 = fRangeHoughSpace[0][0] + ir*fBinSizeImageSpace[0];
                double r2 = fRangeHoughSpace[0][0] + (ir+1)*fBinSizeImageSpace[0];
                double t1 = fRangeHoughSpace[1][0] + it*fBinSizeImageSpace[1];
                double t2 = fRangeHoughSpace[1][0] + (it+1)*fBinSizeImageSpace[1];
                return LKHoughPointRT(fTransformCenter[0],fTransformCenter[1],r1,t1,r2,t2,fImageData[ir][it]);
            }
            ++count;
        }
    }
    return LKHoughPointRT();
}

LKHoughPointRT LKHoughTransformTracker::GetHoughPoint(int ir, int it)
{
    double r1 = fRangeHoughSpace[0][0] + ir*fBinSizeHoughSpace[0];
    double r2 = fRangeHoughSpace[0][0] + (ir+1)*fBinSizeHoughSpace[0];
    double t1 = fRangeHoughSpace[1][0] + it*fBinSizeHoughSpace[1];
    double t2 = fRangeHoughSpace[1][0] + (it+1)*fBinSizeHoughSpace[1];
    return LKHoughPointRT(fTransformCenter[0],fTransformCenter[1],r1,t1,r2,t2,fHoughData[ir][it]);
}

void LKHoughTransformTracker::Transform()
{
    if (fImageIsPoints) {
        int iHough = 0;
        for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir)
        {
            for (auto it=0; it<fNumBinsHoughSpace[1]; ++it)
            {
                auto houghPoint = GetHoughPoint(ir,it);

                for (int iImage=0; iImage<fNumImagePoints; ++iImage)
                {
                    auto imagePoint = GetImagePoint(iImage);
                    //auto weight = CorrelateImageToHough(imagePoint, houghPoint);
                    auto weight = CorrelateBoxBand(imagePoint, houghPoint);
                    if (weight>0) {
                        fHoughData[ir][it] = fHoughData[ir][it] + weight * imagePoint.fWeight;
                    }
                }
                ++iHough;
            }
        }
    }
    else {
        //TODO
    }
}

LKHoughPointRT LKHoughTransformTracker::FindNextMaximumHoughPoint()
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
        LKHoughPointRT houghPoint(fTransformCenter[0],fTransformCenter[1],-1,0,-1,0,-1);
        return houghPoint;
    }

    auto houghPoint = GetHoughPoint(fIdxSelectedR,fIdxSelectedT);
    return houghPoint;
}

void LKHoughTransformTracker::ClearLastMaximumHoughPoint(double rWidth, double tWidth)
{
    if (rWidth<0 || tWidth) {
        fHoughData[fIdxSelectedR][fIdxSelectedT] = 0;
        return;
    }

    if (rWidth==0) rWidth = (fRangeHoughSpace[0][1]-fRangeHoughSpace[0][0])/15;
    if (tWidth==0) tWidth = (fRangeHoughSpace[1][1]-fRangeHoughSpace[1][0])/15;;
    int numBinsHalfR = std::floor(rWidth/fBinSizeHoughSpace[0]/2.);
    int numBinsHalfT = std::floor(tWidth/fBinSizeHoughSpace[1]/2.);

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

double LKHoughTransformTracker::CorrelateBoxLine(LKImagePoint imagePoint, LKHoughPointRT houghPoint)
{
    for (auto iHoughCorner : {0})
    {
        bool existAboveLine = false;
        bool existBelowLine = false;
        for (auto iImageCorner : {1,2,3,4})
        {
            auto point = imagePoint.GetCorner(iImageCorner);
            auto y = houghPoint.EvalY(iHoughCorner, point.X());
            if (y<point.Y()) existAboveLine = true;
            if (y>point.Y()) existBelowLine = true;
        }
        if (existAboveLine&&existBelowLine) {
            auto geoLine = houghPoint.GetGeoLine(0,fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
            auto distance = geoLine.DistanceToLine(imagePoint.GetCenter());
            /// TODO @todo
            auto distanceMax = sqrt(fBinSizeImageSpace[0]*fBinSizeImageSpace[0] + fBinSizeHoughSpace[0]*fBinSizeHoughSpace[0]);
            double weightMax = 10;
            double weightMin = 1;
            double weight = weightMax - (weightMax-weightMin)/distanceMax*distance;
            //lk_debug << distance << " " << distanceMax << " " << weight << endl;
            return weight;
        }
    }
    return -999;
}

double LKHoughTransformTracker::CorrelateBoxBand(LKImagePoint imagePoint, LKHoughPointRT houghPoint)
{
    double weightCorners[4] = {0};
    int includedBelowOrAbove[4] = {0};

    for (auto iHoughCorner : {1,2,3,4})
    {
        bool existAboveLine = false;
        bool existBelowLine = false;

        for (auto iImageCorner : {1,2,3,4})
        {
            auto point = imagePoint.GetCorner(iImageCorner);
            auto y = houghPoint.EvalY(iHoughCorner, point.X());
            //lk_debug << "corner: " << iImageCorner << " evaly=" << y << " pointy=" << point.Y() << endl;
            if (y<point.Y()) existAboveLine = true;
            if (y>point.Y()) existBelowLine = true;
        }

        if (existAboveLine&&existBelowLine)
            includedBelowOrAbove[iHoughCorner-1] = 0;

        auto geoLine = houghPoint.GetGeoLine(0,fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
        auto distance = geoLine.DistanceToLine(imagePoint.GetCenter());
        /// TODO @todo
        auto distanceMax = sqrt(fBinSizeImageSpace[0]*fBinSizeImageSpace[0] + fBinSizeHoughSpace[0]*fBinSizeHoughSpace[0]);
        double weightMax = 10;
        double weightMin = 1;
        double weight = weightMax - (weightMax-weightMin)/distanceMax*distance;
        weightCorners[iHoughCorner-1] = weight;

        if (existAboveLine) {
            includedBelowOrAbove[iHoughCorner-1] = 1;
        }
        else {
            includedBelowOrAbove[iHoughCorner-1] = 2;
        }
    }
    bool existAbove = false;
    bool existBelow = false;
    for (auto i : {0,1,2,3}) {
        if (includedBelowOrAbove[i]==0)
            return 1;
        else if (includedBelowOrAbove[i]==1)
            existAbove = true;
        else if (includedBelowOrAbove[i]==2)
            existBelow = true;
    }
    if (existAbove&&existBelow)
        return 1;

    return -999;
}

void LKHoughTransformTracker::Analyze(int i)
{
    ;
}

TH2D* LKHoughTransformTracker::GetHistImageSpace(TString name)
{
    if (name.IsNull()) name = "histImageSpace";
    auto hist = new TH2D(name,Form("Transform center (x,y) = (%.2f, %.2f);x;y",fTransformCenter[0],fTransformCenter[1]),
                         fNumBinsImageSpace[0],fRangeImageSpace[0][0],fRangeImageSpace[0][1],fNumBinsImageSpace[1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
    if (fImageIsPoints) {
        for (auto iPoint=0; iPoint<fNumImagePoints; ++iPoint) {
            auto point = fImagePointArray[iPoint];
            hist -> Fill(point.GetCenterX(), point.GetCenterY(), point.fWeight);
        }
    }
    return hist;
}

TH2D* LKHoughTransformTracker::GetHistHoughSpace(TString name)
{
    if (name.IsNull()) name = "histHoughSpace";
    //auto hist = new TH2D(name,Form("Transform center (x,y) = (%.2f, %.2f);Radius;#theta (degrees)",fTransformCenter[0],fTransformCenter[1]),
    //                     fNumBinsHoughSpace[0],fRangeHoughSpace[0][0],fRangeHoughSpace[0][1],fNumBinsHoughSpace[1],fRangeHoughSpace[1][0],fRangeHoughSpace[1][1]);
    //for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir) {
    //    for (auto it=0; it<fNumBinsHoughSpace[1]; ++it) {
    //        if (fHoughData[ir][it]>0) {
    //            hist -> SetBinContent(ir+1,it+1,fHoughData[ir][it]);
    //        }
    //    }
    //}
    auto hist = new TH2D(name,Form("Transform center (x,y) = (%.2f, %.2f);#theta (degrees);Radius;",fTransformCenter[0],fTransformCenter[1]),
                         fNumBinsHoughSpace[1],fRangeHoughSpace[1][0],fRangeHoughSpace[1][1],fNumBinsHoughSpace[0],fRangeHoughSpace[0][0],fRangeHoughSpace[0][1]);
    for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir) {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it) {
            if (fHoughData[ir][it]>0) {
                hist -> SetBinContent(it+1,ir+1,fHoughData[ir][it]);
            }
        }
    }
    return hist;
}


void LKHoughTransformTracker::DrawAllHoughLines()
{
    for (auto ir=0; ir<fNumBinsHoughSpace[0]; ++ir)
    {
        for (auto it=0; it<fNumBinsHoughSpace[1]; ++it)
        {
            auto houghPoint = GetHoughPoint(ir,it);
            auto geoLine = houghPoint.GetGeoLine(0,fRangeImageSpace[0][0],fRangeImageSpace[0][1],fRangeImageSpace[1][0],fRangeImageSpace[1][1]);
            geoLine.DrawArrowXY(0) -> Draw();
        }
    }
}
