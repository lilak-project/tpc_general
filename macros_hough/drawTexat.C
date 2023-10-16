#include "ejungwooA.h"
#include "LKHTLineTracker.cpp"

//int fDrawEntry = 113;
//int fDrawEntry = 219;
//int fDrawEntry = 535;
//int fDrawEntry = 804;
//int fDrawEntry = 1068;
int fDrawEntry = 1074;
//int fDrawEntry = 0;

LKRun* fRun;
TTree* fTree = nullptr;
TClonesArray* fEventHeaderHolder;
TClonesArray* fHitArray[3][2];
double fYDiffCut = 3;
bool fDrawStripChain = true;
bool fFillIfMXZ = true;

const int kStripAndChain = 2;
const int kStrip = 0;
const int kChain = 1;
const int kMMCenter = 0;
const int kMMLeft = 1;
const int kMMRight = 2;
const int kFrontView = 0;
const int kSideView = 1;
const int kTopView = 2;

double fBeamRangeX1 = -10;
double fBeamRangeX2 = +10;
double fBeamRangeY1 = 150;
double fBeamRangeY2 = 200;

ejungwoo::Binning* fBnnX;
ejungwoo::Binning* fBnnY;
ejungwoo::Binning* fBnnZ;
ejungwoo::Binning* fBnnXZ;
ejungwoo::Binning* fBnnZY;
ejungwoo::Binning* fBnnXY;
ejungwoo::Binning* fBnnZXY;
TCanvas* fCvsAll = nullptr;
TCanvas* fCvsParam = nullptr;
TCanvas* fCvsTest = nullptr;

LKHTLineTracker* fTrackerXZ[3][3];
LKHTLineTracker* fTrackerZY[3][3];
LKHTLineTracker* fTrackerXY[3][3];

TH3D *fHistZXY;
TGraph2DErrors* fGraphZXY[4];
TGraphErrors* fGraphXY;
TGraphErrors* fGraphZY;
TGraphErrors* fGraphXZ;

void DrawEvent(int entry=-1);

void drawTexat()
{
    gStyle -> SetOptStat(0);

    fRun = new LKRun("texat",0,"drawhit");
    //fRun -> AddInputFile("/home/ejungwoo/lilak/texat_reco/macros_tracking/data/texat_0000.psa.root");
    //fRun -> AddInputFile("~/data/texat/reco/texat_0801.psa.root");
    fRun -> AddInputFile("/home/ejungwoo/data/texat/reco/texat_0801.all.root");
    fRun -> Init();
    fEventHeaderHolder = fRun -> GetBranchA("EventHeader");
    fHitArray[0][0] = fRun -> GetBranchA("HitCenter");
    fHitArray[0][1] = fRun -> GetBranchA("HitOthers");
    fHitArray[1][0] = fRun -> GetBranchA("HitLStrip");
    fHitArray[1][1] = fRun -> GetBranchA("HitLChain");
    fHitArray[2][0] = fRun -> GetBranchA("HitRStrip");
    fHitArray[2][1] = fRun -> GetBranchA("HitRChain");
    fTree = fRun -> GetInputTree();

    //fBnnX = new ejungwoo::Binning("xaxis","X", "Hit.fX", 143,-120,120);
    //fBnnY = new ejungwoo::Binning("yaxis","Y (time-axis) (350-tb)", "350-Hit.fY", 110,0,330);
    //fBnnZ = new ejungwoo::Binning("zaxis","Z (beam axis)", "Hit.fZ", 141,150,500);
    fBnnX = new ejungwoo::Binning("xaxis","X", "Hit.fX", 143,-150,150);
    fBnnY = new ejungwoo::Binning("yaxis","Y (time-axis) (350-tb)", "350-Hit.fY", 110,-100,500);
    fBnnZ = new ejungwoo::Binning("zaxis","Z (beam axis)", "Hit.fZ", 141,0,600);
    fBnnXZ = new ejungwoo::Binning(fBnnX,fBnnZ); fBnnXZ -> SetNameMainTitle("top","Top-view");
    fBnnZY = new ejungwoo::Binning(fBnnZ,fBnnY); fBnnZY -> SetNameMainTitle("side","Side-view");
    fBnnXY = new ejungwoo::Binning(fBnnX,fBnnY); fBnnXY -> SetNameMainTitle("front","Front(upstream)-view");
    fBnnZXY = new ejungwoo::Binning(fBnnZ,fBnnX,fBnnY);
    fHistZXY = fBnnZXY -> NewHist3();
    auto bnnXL = new ejungwoo::Binning("xaxis","X", "Hit.fX", 60,-120,0);
    auto bnnXR = new ejungwoo::Binning("xaxis","X", "Hit.fX", 60,0,120);

    auto SetTracker = [](LKHTLineTracker* tk, ejungwoo::Binning* bnnx, ejungwoo::Binning* bnny) {
        tk -> SetTransformCenter(bnnx->X1(),bnny->X1());
        tk -> SetImageSpaceRange(bnnx->NX(),bnnx->X1(),bnnx->X2(),bnny->NX(),bnny->X1(),bnny->X2());
        tk -> SetParamSpaceBins(80,100);
        tk -> SetCorrelateBoxBand();
        tk -> SetWFConst();
    };

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight}) {
        for (auto iSCA : {kStrip,kChain,kStripAndChain}) {
            auto bnnX = fBnnX;
            if (iCLR==1) bnnX = bnnXL;
            if (iCLR==2) bnnX = bnnXR;
            fTrackerXY[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerXY[iCLR][iSCA], bnnX,fBnnY);
            fTrackerZY[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerZY[iCLR][iSCA],fBnnZ,fBnnY);
            fTrackerXZ[iCLR][iSCA] = new LKHTLineTracker(); SetTracker(fTrackerXZ[iCLR][iSCA], bnnX,fBnnZ);
        }
    }
    for (auto bnn : {fBnnXY,fBnnXZ,fBnnZY}) bnn -> NewHist2();

    fCvsParam = ejungwoo::Canvas("cvs_param",90,100,3,2); 
    fCvsAll = ejungwoo::Canvas("cvs_all",60,100,2,2);

    DrawEvent();
}

void DrawEvent(int entry)
{
    if (entry==-1) fDrawEntry = fDrawEntry + 1;
    else fDrawEntry = entry;
    fRun -> GetEntry(fDrawEntry);

    auto numHitsAll = 0;
    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight})
        for (auto iSCA : {kStrip,kChain})
            numHitsAll += fHitArray[iCLR][iSCA] -> GetEntries();

    auto eventHeader = (TTEventHeader*) fEventHeaderHolder -> At(0);
    auto goodEvent = eventHeader -> IsGoodEvent();
    if (numHitsAll==0||goodEvent==false) { DrawEvent(); return; }

    auto eventNumber = eventHeader -> GetEventNumber();
    e_info << ">>>> " << fDrawEntry << " " << eventNumber << " " << numHitsAll << " " << goodEvent << endl;

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight})
    {
        for (auto iSCA : {kStrip,kChain})
        {
            TString titleCLR = "center";
            if (iCLR==1) titleCLR = "left";
            if (iCLR==2) titleCLR = "right";
            TString titleSC = "strip";
            if (iSCA==1) titleSC = "chain";
            if (iCLR==kMMCenter) {
                titleSC = "";
                if (iSCA==1) titleCLR = "others";
            }
            e_info << titleCLR << " " << titleSC << " " << fHitArray[iCLR][iSCA] -> GetEntries() << endl;;
        }
    }

    if (fGraphZXY[0]==nullptr)
    {
        fGraphZXY[0] = new TGraph2DErrors();
        fGraphZXY[1] = new TGraph2DErrors();
        fGraphZXY[2] = new TGraph2DErrors();
        fGraphZXY[3] = new TGraph2DErrors();
        fGraphZXY[0] -> SetMarkerStyle(24);
        fGraphZXY[1] -> SetMarkerStyle(25);
        fGraphZXY[2] -> SetMarkerStyle(25);
        fGraphZXY[3] -> SetMarkerStyle(26);
        fGraphZXY[0] -> SetMarkerColor(kRed);
        fGraphZXY[1] -> SetMarkerColor(kBlack);
        fGraphZXY[2] -> SetMarkerColor(kBlue);
        fGraphZXY[3] -> SetMarkerColor(kGreen+1);
    }
    fGraphZXY[0] -> Clear();
    fGraphZXY[1] -> Clear();
    fGraphZXY[2] -> Clear();

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight})
    {
        for (auto iSCA : {kStrip,kChain,kStripAndChain})
        {
            fTrackerXY[iCLR][iSCA] -> Clear();
            fTrackerZY[iCLR][iSCA] -> Clear();
            fTrackerXZ[iCLR][iSCA] -> Clear();
        }
    }

    for (auto iCLR : {kMMCenter,kMMLeft,kMMRight})
    {
        for (auto iSCA : {kStrip,kChain})
        {
            auto numHits = fHitArray[iCLR][iSCA] -> GetEntries();
            for (auto iHit=0; iHit<numHits; ++iHit)
            {
                auto hit = (LKHit*) fHitArray[iCLR][iSCA] -> At(iHit);
                auto w = hit -> W();
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

                fGraphZXY[iCLR] -> SetPoint(fGraphZXY[iCLR]->GetN(),z,x,y);
                fGraphZXY[iCLR] -> SetPointError(fGraphZXY[iCLR]->GetN()-1,dz,dx,dy);
                if (iCLR==0&&iSCA==1) {
                    fGraphZXY[3] -> SetPoint(fGraphZXY[3]->GetN(),z,x,y);
                    fGraphZXY[3] -> SetPointError(fGraphZXY[3]->GetN()-1,dz,dx,dy);
                }
            }
        }
    }

    auto TransformAndFit = [eventNumber](int iView, int iCLR, LKHTLineTracker* tk[3][3], TVirtualPad *padImage, TVirtualPad *padParam=nullptr)
    {
        ejungwoo::Binning *bnn = fBnnXY;
        if (iView==kTopView) bnn = fBnnXZ;
        else if (iView==kSideView) bnn = fBnnZY;
        const char* mainTitle0 = bnn -> fMainTitle.Data();

        padImage -> cd();
        if (padImage->GetListOfPrimitives()->GetEntries()==0) {
            bnn -> fHist2 -> SetTitle(Form("[event-%d(%d)] %s",fDrawEntry,eventNumber,mainTitle0));
            bnn -> fHist2 -> Draw("colz");
        }

        for (auto iSCA : {kStrip,kChain})
        {
            auto graph = tk[iCLR][iSCA] -> GetDataGraphImageSapce();
            graph -> SetMarkerStyle(20);
            if (iSCA==kStrip) graph -> SetMarkerColor(kBlack);
            if (iSCA==kChain) graph -> SetMarkerColor(kBlue);
            if (iSCA==kStripAndChain) graph -> SetMarkerColor(kViolet);
            if (iCLR==kMMCenter && iSCA==kStrip) graph -> SetMarkerColor(kRed);
            if (iCLR==kMMCenter && iSCA==kChain) graph -> SetMarkerColor(kGreen+1);
            graph -> SetLineColor(kGray);
            graph -> SetMarkerSize(0.8);
            graph -> Draw("same pz");

        }

        int iSCA = ((iView==kFrontView)?kChain:kStrip);
        auto tkLAll = tk[iCLR][kStripAndChain];
        auto tkLFit = tk[iCLR][iSCA];

        LKLinearTrack* track = nullptr;

        if (padParam!=nullptr)
        {
            LKHTLineTracker* tkTrf = tkLAll;
            LKHTLineTracker* tkFit = tkLFit;
            tkTrf -> Transform();
            const char* titleCLR = "C";
            if (iCLR==kMMLeft) titleCLR = "L";
            if (iCLR==kMMRight) titleCLR = "R";
            auto histParam = tkTrf -> GetHistParamSpace(Form("paramSpace%s_%d_%d",titleCLR,fDrawEntry,iView));
            histParam -> SetTitle(Form("%s (%s) %s",mainTitle0, ((iCLR==kMMLeft)?"L":"R"), histParam->GetTitle()));
            auto paramPoint = tkTrf -> FindNextMaximumParamPoint();
            auto graphRange = paramPoint -> GetRangeGraphInParamSpace(1);

            padParam -> cd();
            histParam -> Draw("colz");
            graphRange -> Draw("samel");

            if (iCLR!=kMMCenter && iView!=kTopView && tkFit->GetNumImagePoints()>3)
            {
                track = tkFit -> FitTrackWithParamPoint(paramPoint);
                auto graphBand = paramPoint -> GetBandInImageSpace(tkTrf->GetRangeImageSpace(0,0),tkTrf->GetRangeImageSpace(0,1),tkTrf->GetRangeImageSpace(1,0),tkTrf->GetRangeImageSpace(1,1));
                graphBand -> SetLineColor(kBlue);
                auto graphTrack = track -> TrajectoryOnPlane(LKVector3::kX,LKVector3::kY);
                graphBand -> SetLineColor(kBlue);
                graphTrack -> SetLineColor(kRed);
                if (iCLR==kMMRight) {
                    graphBand -> SetLineColor(kGreen+1);
                    graphTrack -> SetLineColor(kViolet);
                }

                padImage -> cd();
                graphBand -> Draw("samel");
                graphTrack -> Draw("samel");
            }
        }

        return track;
    };

    fCvsAll -> cd(1) -> Clear();
    fCvsAll -> cd(2) -> Clear();
    fCvsAll -> cd(3) -> Clear();
    fCvsAll -> cd(4) -> Clear();

    TransformAndFit(kTopView,   kMMCenter, fTrackerXZ, fCvsAll->cd(3));
    TransformAndFit(kTopView,   kMMLeft,   fTrackerXZ, fCvsAll->cd(3));
    TransformAndFit(kTopView,   kMMRight,  fTrackerXZ, fCvsAll->cd(3));
    TransformAndFit(kFrontView, kMMCenter, fTrackerXY, fCvsAll->cd(1), fCvsParam->cd(3));
    TransformAndFit(kSideView,  kMMCenter, fTrackerZY, fCvsAll->cd(2), fCvsParam->cd(6));

    auto trackFL = TransformAndFit(kFrontView, kMMLeft,  fTrackerXY, fCvsAll->cd(1), fCvsParam->cd(1));
    auto trackSL = TransformAndFit(kSideView,  kMMLeft,  fTrackerZY, fCvsAll->cd(2), fCvsParam->cd(2));
    auto trackFR = TransformAndFit(kFrontView, kMMRight, fTrackerXY, fCvsAll->cd(1), fCvsParam->cd(4));
    auto trackSR = TransformAndFit(kSideView,  kMMRight, fTrackerZY, fCvsAll->cd(2), fCvsParam->cd(5));

    LKGeoLine lineL;
    LKGeoLine lineR;
    bool existLTrack = false;
    bool existRTrack = false;

    if (trackFL!=nullptr && trackSL!=nullptr)
    {
        existLTrack = true;
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
        lineL = plane1.GetCrossSectionLine(plane2);
        auto vertex1 = lineL.GetPointAtX(0);
        auto vertex2 = lineL.GetPointAtX(fBnnX->X1());
        lineL.SetLine(vertex1,vertex2);
    }
    if (trackFR!=nullptr && trackSR!=nullptr)
    {
        existRTrack = true;
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
        lineR = plane1.GetCrossSectionLine(plane2);
        auto vertex1 = lineR.GetPointAtX(0);
        auto vertex2 = lineR.GetPointAtX(fBnnX->X2());
        lineR.SetLine(vertex1,vertex2);
    }

    fCvsAll -> cd(4);
    fHistZXY -> Draw();
    if (fGraphZXY[0]->GetN()>0) fGraphZXY[0] -> Draw("same p error");
    if (fGraphZXY[1]->GetN()>0) fGraphZXY[1] -> Draw("same p error");
    if (fGraphZXY[2]->GetN()>0) fGraphZXY[2] -> Draw("same p error");
    if (fGraphZXY[3]->GetN()>0) fGraphZXY[3] -> Draw("same p error");
    fCvsAll -> Modified();
    fCvsAll -> Update();

    if (existLTrack) {
        auto graphFitL = lineL.GetGraphZXY();
        graphFitL -> SetMarkerColor(kRed);
        graphFitL -> SetLineColor(kRed);
        graphFitL -> Draw("same p0 line");
    }
    if (existRTrack) {
        auto graphFitR = lineR.GetGraphZXY();
        graphFitR -> SetMarkerColor(kViolet);
        graphFitR -> SetLineColor(kViolet);
        graphFitR -> Draw("same p0 line");
    }
}
