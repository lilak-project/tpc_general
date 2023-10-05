#include "ejungwooA.h"
#include "LKHoughTransformTracker.cpp"

LKRun* fRun;
TTree* fTree = nullptr;
TClonesArray* fHitArray;
TClonesArray* fLeftChainArray;
TClonesArray* fLeftStripArray;
TClonesArray* fRightChainArray;
TClonesArray* fRightStripArray;
TClonesArray* fUnfoldedArray;
double fYDiffCut = 3;
bool fDrawStripChain = true;
bool fFillIfMXZ = true;

ejungwoo::Binning* fBnnYDiff;
ejungwoo::Binning* fBnnX;
ejungwoo::Binning* fBnnY;
ejungwoo::Binning* fBnnZ;
ejungwoo::Binning* fBnnXZ;
ejungwoo::Binning* fBnnZY;
ejungwoo::Binning* fBnnXY;
TCanvas* fCvsAll;

int fTreeEntry = -1;
//int fDrawEntry = 114-1;
//int fDrawEntry = 220-1;
//int fDrawEntry = 536-1;
//int fDrawEntry = 805-1;
//int fDrawEntry = 1069-1;
int fDrawEntry = 1075-1;

void DrawTree(int entry=-1);
void DrawEvent(int entry=-1);
void UnfoldHits();
void SetYDiffCut(double value) { fYDiffCut = value; }

LKHoughTransformTracker* fTracker;

void drawTexat()
{
    fTracker = new LKHoughTransformTracker();
    fTracker -> SetTransformCenter(-150, 0);
    fTracker -> SetImageSpaceRange(150,-150,150,150,150,350);
    fTracker -> SetParamSpaceBins(120,120);
    fTracker -> SetCorrelateBoxBand();
    fTracker -> SetWFLinear();

    fRun = new LKRun("texat",0,"drawhit");
    //fRun -> AddInputFile("/home/ejungwoo/lilak/texat_reco/macros_tracking/data/texat_0000.psa.root");
    fRun -> AddInputFile("~/data/texat/reco/texat_0801.psa.root");
    fRun -> Init();
    fHitArray = fRun -> GetBranchA("Hit");
    fTree = fRun -> GetInputTree();

    fLeftChainArray  = new TClonesArray("LKHit",50);
    fLeftStripArray  = new TClonesArray("LKHit",50);
    fRightChainArray = new TClonesArray("LKHit",50);
    fRightStripArray = new TClonesArray("LKHit",50);
    fUnfoldedArray   = new TClonesArray("LKHit",50);

    fCvsAll = ejungwoo::Canvas("cvs_event_view",0,0,2,2);

    fBnnYDiff = new ejungwoo::Binning("ydiff","ydiff (tb)","",100,0,50);
    //fBnnX = new ejungwoo::Binning("xaxis","X", "Hit.fX", 143,-3,140);
    //fBnnY = new ejungwoo::Binning("yaxis","Y (time-axis) (350-tb)", "350-Hit.fY", 110,0,330);
    fBnnX = new ejungwoo::Binning("xaxis","X", "Hit.fX", 143,-150,150);
    fBnnY = new ejungwoo::Binning("yaxis","Y (time-axis) (350-tb)", "350-Hit.fY", 110,0,330);
    fBnnZ = new ejungwoo::Binning("zaxis","Z (beam axis)", "Hit.fZ", 141,150,500);

    fBnnXZ = fBnnX->Add(fBnnZ); fBnnXZ -> SetNameMainTitle("top","Top view");
    fBnnZY = fBnnZ->Add(fBnnY); fBnnZY -> SetNameMainTitle("side","Side view");
    fBnnXY = fBnnX->Add(fBnnY); fBnnXY -> SetNameMainTitle("front","Front (upstream) view");

    for (auto bnn : {fBnnXY,fBnnXZ,fBnnZY})
        auto hist = bnn -> NewHist2();
    fBnnYDiff -> NewHist();

    //DrawTree(-2);
    DrawEvent();
}

void DrawTree(int entry)
{
    if (entry==-1) fTreeEntry = fTreeEntry + 1;
    else fTreeEntry = entry;

    TCut cut(Form("Hit.fW&&Entry$==%d",fTreeEntry));
    if (entry==-2)
        cut = TCut();

    auto iCvs = 0;
    for (auto bnn : {fBnnXY,fBnnXZ,fBnnZY})
    {
        TString title = (TString(cut.GetTitle()).IsNull()==false) ? bnn->fMainTitle+" {"+cut.GetTitle()+"}" : bnn->fMainTitle;
        auto hist = bnn -> fHist2;
        fTree -> Project(bnn->fName,bnn->GetSelection(2),cut);
        fCvsAll -> cd(++iCvs);
        hist -> SetTitle(title);
        hist -> Draw("colz");
    }
}

void DrawEvent(int entry)
{
    if (entry==-1) fDrawEntry = fDrawEntry + 1;
    else fDrawEntry = entry;

    auto histXY = fBnnXY -> fHist2;
    auto histXZ = fBnnXZ -> fHist2;
    auto histZY = fBnnZY -> fHist2;
    histXY -> Reset("ICES");
    histXZ -> Reset("ICES");
    histZY -> Reset("ICES");

    fLeftStripArray -> Clear("C");
    fRightStripArray -> Clear("C");
    fLeftChainArray -> Clear("C");
    fRightChainArray -> Clear("C");

    fRun -> GetEntry(fDrawEntry);
    auto numHits = fHitArray -> GetEntries();
    if (numHits==0) {
        DrawEvent();
        return;
    }
    for (auto iHit=0; iHit<numHits; ++iHit) {
        auto hit = (LKHit*) fHitArray -> At(iHit);
        auto x = hit -> X();
        auto y = 350-hit -> Y();
        auto z = hit -> Z();
        auto w = hit -> W();
        //w = 1;
        bool fillX = true;
        //auto dx = hit->GetDX()/2.;
        //auto dy = hit->GetDY()/2.;
        auto dx = hit->GetDX()*2.;
        auto dy = hit->GetDY()*2.;
        fTracker -> AddImagePoint(x,dx,y,dy,1);
        //fTracker -> AddImagePointBox(x,y,x,y,1);
        if (x<0) {
            fillX = false;
            if (fFillIfMXZ)
                fillX = true;
        }
        bool fillZ = true;
        if (z<0) {
            fillZ = false;
            if (fFillIfMXZ)
                fillZ = true;
        }
        if (fillX) histXY -> Fill(x,y,w);
        if (fillX&&fillZ) histXZ -> Fill(x,z,w);
        if (fillZ) histZY -> Fill(z,y,w);
        if (x==-3) {
            auto hit0 = (LKHit*) fLeftStripArray -> ConstructedAt(fLeftStripArray->GetEntries());
            hit0 -> CopyFrom(hit);
            if (fDrawStripChain)
                for (auto xAll=0.5; xAll<64; xAll+=1) {
                    histXY -> Fill(xAll,y,w);
                    histXZ -> Fill(xAll,z,w);
                }
        }
        if (x==-2) {
            auto hit0 = (LKHit*) fRightStripArray -> ConstructedAt(fRightStripArray->GetEntries());
            hit0 -> CopyFrom(hit);
            if (fDrawStripChain)
                for (auto xAll=70.5; xAll<134; xAll+=1) {
                    histXY -> Fill(xAll,y,w);
                    histXZ -> Fill(xAll,z,w);
                }
        }
        if (z==-1) {
            if (x<65) {
                auto hit0 = (LKHit*) fLeftChainArray -> ConstructedAt(fLeftChainArray->GetEntries());
                hit0 -> CopyFrom(hit);
                if (fDrawStripChain)
                    for (auto zAll=0.5; zAll<127; zAll+=1) {
                        histXZ -> Fill(x,zAll);
                        histZY -> Fill(zAll,y);
                    }
            }
            else {
                auto hit0 = (LKHit*) fRightChainArray -> ConstructedAt(fRightChainArray->GetEntries());
                hit0 -> CopyFrom(hit);
                if (fDrawStripChain)
                    for (auto zAll=0.5; zAll<127; zAll+=1) {
                        histXZ -> Fill(x,zAll);
                        histZY -> Fill(zAll,y);
                    }
            }
        }
    }

    //UnfoldHits();

    auto numHitsUnfolded = fUnfoldedArray -> GetEntries();
    for (auto iHit=0; iHit<numHitsUnfolded; ++iHit) {
        auto hit = (LKHit*) fUnfoldedArray -> At(iHit);
        auto x = hit -> X();
        auto y = hit -> Y();
        auto z = hit -> Z();
        auto w = hit -> W();
        //w = 1;
        histXY -> Fill(x,y,w);
        histXZ -> Fill(x,z,w);
        histZY -> Fill(z,y,w);
    }

    auto iCvs = 0;
    for (auto bnn : {fBnnXY,fBnnXZ,fBnnZY})
    {
        TString title = TString("[event-") + fDrawEntry + "]  " + bnn->fMainTitle;
        auto hist = bnn -> fHist2;
        hist -> SetTitle(title);
        fCvsAll -> cd(++iCvs);
        hist -> Draw("colz");
        //auto line1 = new TLine(64,fBnnY->fMin[0],64,fBnnY->fMax[0]);
        //auto line2 = new TLine(70,fBnnY->fMin[0],70,fBnnY->fMax[0]);
        auto line1 = new TLine(-10,fBnnY->fMin[0],-10,fBnnY->fMax[0]);
        auto line2 = new TLine(10,fBnnY->fMin[0],10,fBnnY->fMax[0]);
        auto line3 = new TLine(fBnnX->fMin[0],150,fBnnX->fMax[0],150);
        auto line4 = new TLine(fBnnX->fMin[0],200,fBnnX->fMax[0],200);
        for (auto line : {line1, line2, line3, line4}) {
            line -> SetLineColor(kOrange-3);
            line -> SetLineStyle(2);
            line -> SetLineWidth(1);
        }
        if (bnn==fBnnXZ) {
            line1 -> Draw("samel");
            line2 -> Draw("samel");
        }
        if (bnn==fBnnXY) {
            line1 -> Draw("samel");
            line2 -> Draw("samel");
            line3 -> Draw("samel");
            line4 -> Draw("samel");
        }
        if (bnn==fBnnZY) {
            line3 -> Draw("samel");
            line4 -> Draw("samel");
        }
    }

    fCvsAll -> cd(++iCvs);
    fBnnYDiff -> fHist -> Draw();

    //for (auto i : {0,1,2}) {
    //    fTracker -> Transform();
    //    auto paramPointAtMax = fTracker -> FindNextMaximumParamPoint();
    //    auto paramPointRange = fTracker -> ReinitializeFromLastParamPoint();
    //}

    fTracker -> Transform();
    auto paramPointAtMax = fTracker -> FindNextMaximumParamPoint();
    //{
    //    auto track = fTracker -> FitTrackWithParamPoint(paramPointAtMax,0.01);
    //    auto graphTrack = track -> TrajectoryOnPlane(LKVector3::kX,LKVector3::kY);
    //    fCvsAll -> cd(1);
    //    graphTrack -> Draw("samel");
    //}
    auto graph1 = paramPointAtMax -> GetRBandInImageSpace(-150,150,0,350);
    fCvsAll -> cd(1);
    graph1 -> Draw("samel");
    {
        paramPointAtMax -> GetRadialLineInImageSpace(0,5) -> Draw("samel");
        paramPointAtMax -> GetRadialLineInImageSpace(1,5) -> Draw("samel");
        paramPointAtMax -> GetRadialLineInImageSpace(2,5) -> Draw("samel");
        paramPointAtMax -> GetRadialLineInImageSpace(3,5) -> Draw("samel");
        paramPointAtMax -> GetRadialLineInImageSpace(4,5) -> Draw("samel");
    }

    fCvsAll -> cd(iCvs);
    auto histParam = fTracker -> GetHistParamSpace("paramSpace");
    histParam -> Draw("colz");
    auto graph2 = paramPointAtMax -> GetRangeGraphInParamSpace(1);
    graph2 -> Draw("samel");
}

void UnfoldHits()
{
    fBnnYDiff -> fHist -> Reset("ICES");
    fUnfoldedArray -> Clear("C");

    auto numHitsStrip = fLeftStripArray -> GetEntries();
    auto numHitsChain = fLeftChainArray -> GetEntries();
    e_info << "Left: " << numHitsStrip << " strip-hits and " << numHitsChain << " chain-hits" << endl;
    for (auto iHitStrip=0; iHitStrip<numHitsStrip; ++iHitStrip) {
        auto hitStrip = (LKHit*) fLeftStripArray -> At(iHitStrip);
        for (auto iHitChain=0; iHitChain<numHitsChain; ++iHitChain)
        {
            auto hitChain = (LKHit*) fLeftChainArray -> At(iHitChain);

            auto yChain = hitChain -> Y();
            auto yStrip = hitStrip -> Y();
            double yDiff = abs(yChain-yStrip);
            fBnnYDiff -> fHist -> Fill(yDiff);
            if (yDiff<fYDiffCut)
            {
                auto x = hitChain -> X();
                auto y = 0.5 * (yChain + yStrip);
                auto z = hitStrip -> Z();
                auto w = 0.5 * (hitChain->W() + hitStrip->W());
                //lk_debug << "uhit-" << iHitChain << "/" << iHitStrip << ", dy=" << abs(yChain-yStrip) << " (" << x << ", " << y << ", " << z << " | " << w << ")" << endl;
                auto hit = (LKHit*) fUnfoldedArray -> ConstructedAt(fUnfoldedArray->GetEntries());
                hit -> SetPosition(x,y,z);
                hit -> SetCharge(w);
            }
        }
    }

    numHitsStrip = fRightStripArray -> GetEntries();
    numHitsChain = fRightChainArray -> GetEntries();
    e_info << "Right: " << numHitsStrip << " strip-hits and " << numHitsChain << " chain-hits" << endl;
    for (auto iHitStrip=0; iHitStrip<numHitsStrip; ++iHitStrip) {
        auto hitStrip = (LKHit*) fRightStripArray -> At(iHitStrip);
        for (auto iHitChain=0; iHitChain<numHitsChain; ++iHitChain)
        {
            auto hitChain = (LKHit*) fRightChainArray -> At(iHitChain);

            auto yChain = hitChain -> Y();
            auto yStrip = hitStrip -> Y();
            double yDiff = abs(yChain-yStrip);
            fBnnYDiff -> fHist -> Fill(yDiff);
            if (yDiff<fYDiffCut)
            {
                auto x = hitChain -> X();
                auto y = 0.5 * (yChain + yStrip);
                auto z = hitStrip -> Z();
                auto w = 0.5 * (hitChain->W() + hitStrip->W());
                //e_info << "uhit-" << iHitChain << "/" << iHitStrip << ", dy=" << abs(yChain-yStrip) << " (" << x << ", " << y << ", " << z << " | " << w << ")" << endl;
                auto hit = (LKHit*) fUnfoldedArray -> ConstructedAt(fUnfoldedArray->GetEntries());
                hit -> SetPosition(x,y,z);
                hit -> SetCharge(w);
            }
        }
    }
    e_info << "unfolded " << fUnfoldedArray -> GetEntries() << " hits" << endl;
}
