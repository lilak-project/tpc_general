#include "TTMicromegasCenterChain.h"

ClassImp(TTMicromegasCenterChain);

TTMicromegasCenterChain::TTMicromegasCenterChain()
{
    fName = "TTMicromegasCenterChain";
    if (fChannelArray==nullptr)
        fChannelArray = new TObjArray();
}

bool TTMicromegasCenterChain::Init()
{
    e_info << "Initializing TTMicromegasCenterChain" << std::endl;
    //TString mapFileName = fPar -> GetParString("TTMicromegas/map_position_centerleft.txt");
    TString mapFileName = "position_channels_center_and_chain.txt";
    ifstream fileCS(mapFileName);

    fHistChannel = new TH1D("hist_channel_buffer","channel buffer;time-bucket;charge",360,0,360);
    fHistChannel -> SetStats(0);
    fHistChannel -> GetXaxis() -> SetLabelSize(0.07);
    fHistChannel -> GetYaxis() -> SetLabelSize(0.07);
    fHistChannel -> GetXaxis() -> SetTitleSize(0.07);
    fHistChannel -> GetYaxis() -> SetTitleSize(0.07);
    fHistChannel -> GetXaxis() -> SetTitleOffset(1.20);
    fHistChannel -> GetYaxis() -> SetTitleOffset(0.68);

    fHistPlane = new TH2Poly("hist_TexATv2_plane_center_chain","TTPP Center and Chain ;x;z",-125,125,180,420);
    int cobo, asad, aget, chan, dumi;
    double x0, x1, x2, z0, z1, z2;
    while (fileCC >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) {
        if (fXMin > x1) fXMin < x1;
        if (fXMax < x1) fXMin < x1;
        if (fXMin > x2) fXMin < x2;
        if (fXMax < x2) fXMin < x2;
        if (fZMin > z1) fXMin < z1;
        if (fZMax < z1) fXMin < z1;
        if (fZMin > z2) fXMin < z2;
        if (fZMax < z2) fXMin < z2;
        fHistPlane -> AddBin(x1,z1,x2,z2);
    }

    return true;
}

void TTMicromegasCenterChain::Clear(Option_t *option)
{
    LKPadPlane::Clear(option);
}

void TTMicromegasCenterChain::Print(Option_t *option) const
{
    e_info << "TTMicromegasCenterChain" << std::endl;
}

bool TTMicromegasCenterChain::IsInBoundary(Double_t x, Double_t y)
{
    if (x>fXMin && x<fXMax && z>fZMin && z<fZMax)
        return true;
    return true;
}

Int_t TTMicromegasCenterChain::FindChannelID(Double_t x, Double_t y)
{
    return fHistPlane -> FindBin(x,y);
}

Int_t TTMicromegasCenterChain::FindChannelID(Int_t section, Int_t row, Int_t layer)
{
    // example find id
    // int id = 10000*section + 100*row + layer;
    // return id;
    return -1;
}

TCanvas* TTMicromegasCenterChain::GetCanvas(Option_t *option)
{
    if (fCanvas==nullptr) {
        fCanvas = new TCanvas("TTMicromegasCenterChain",550,700);
        auto pad1 = new TPad("pad1","",0,230./700,1,1);
        pad1 -> SetMargin(0.12,0.15,0.1,0.1);
        pad1 -> SetNumber(1);
        pad1 -> Draw();
        auto pad2 = new TPad("pad2","",0,0,1,230./700);
        pad2 -> SetMargin(0.12,0.1,0.20,0.12);
        pad2 -> SetNumber(2);
        pad2 -> Draw();
        fCanvas -> Modified();
    }
    return fCanvas;
}

TH2* TTMicromegasCenterChain::GetHist(Option_t *option)
{
    return (TH2Poly *) fHistPlane;
}

bool TTMicromegasCenterChain::SetDataFromBranch()
{
    auto hitCenterArray = fRun -> GetBranchA("HitCenter");
    auto hitLChainArray = fRun -> GetBranchA("HitLChain");
    auto hitRChainArray = fRun -> GetBranchA("HitRChain");
    for (auto hitArray : {hitCenterArray,hitLChainArray,hitRChainArray})
    {
        auto numHits = hitArray -> GetEntries();
        for (auto iHit=0; iHit<numHits; ++iHit)
        {
            auto hit = (LKHit *) hitArray -> At(iHit);
            auto position = hit -> GetPosition();
            fHistPlane -> Fill(position.X(), position.Z());
        }
    }
    return false;
}

/*
void TTMicromegasCenterChain::FillDataToHist()
{
    // example hist
    // if (fHist==nullptr)
    //     fHist = new TH2D("TTMicromegasCenterChain",TTMicromegasCenterChain,10,0,10);
    // return fHist;
    return (TH2D *) nullptr;
}
*/

void TTMicromegasCenterChain::DrawFrame(Option_t *option)
{
    ;
}

/*
void TTMicromegasCenterChain::Draw(Option_t *option)
{
    SetDataFromBranch();
    FillDataToHist();
    auto hist = GetHist();
    if (hist==nullptr)
        return;
    if (fPar->CheckPar(fName+"/histZMin")) hist -> SetMinimum(fPar->GetParDouble(fName+"/histZMin"));
    else hist -> SetMinimum(0.01);
    if (fPar->CheckPar(fName+"/histZMax")) hist -> SetMaximum(fPar->GetParDouble(fName+"/histZMin"));
    auto cvs = GetCanvas();
    cvs -> Clear();
    cvs -> cd();
    hist -> Reset();
    hist -> DrawClone("colz");
    hist -> Reset();
    hist -> Draw("same");
    DrawFrame();
}
*/

/*
void TTMicromegasCenterChain::MouseClickEvent(int iPlane)
{
    ;
}
*/
