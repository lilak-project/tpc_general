#include "TTMicromegasCenterChain.h"

ClassImp(TTMicromegasCenterChain);

TTMicromegasCenterChain* TTMicromegasCenterChain::fInstance = nullptr;
TTMicromegasCenterChain* TTMicromegasCenterChain::GetMMCC() { return fInstance; }

TTMicromegasCenterChain::TTMicromegasCenterChain()
{
    fInstance = this;
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
        auto bin = fHistPlane -> AddBin(x1,z1,x2,z2);
        int caac = cobo*10000 + asad*1000 + aget*100 + chan;
        fMapBinToCAAC.insert(std::pair<std::vector<Int_t>, Int_t>(bin,caac));
        fMapCAACToBin.insert(std::pair<std::vector<Int_t>, Int_t>(caac,bin));
    }

    SetDataFromBranch();

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
        pad1 -> AddExec("ex", "TTMicromegasCenterChain::MouseClickEvent()");
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
    fChannelArray = fRun -> GetBranchA("RawData");
    fHitCenterArray = fRun -> GetBranchA("HitCenter");
    fHitLChainArray = fRun -> GetBranchA("HitLChain");
    fHitRChainArray = fRun -> GetBranchA("HitRChain");
    return false;
}

void TTMicromegasCenterChain::FillDataToHist()
{
    for (auto hitArray : {fHitCenterArray,fHitLChainArray,fHitRChainArray})
    {
        auto numHits = hitArray -> GetEntries();
        for (auto iHit=0; iHit<numHits; ++iHit)
        {
            auto hit = (LKHit *) hitArray -> At(iHit);
            auto position = hit -> GetPosition();
            fHistPlane -> Fill(position.X(), position.Z());
        }
    }

    FillChannelBuffer();
}

void TTMicromegasCenterChain::FillChannelBuffer(Int_t bin)
{
    if (fChannelArray==nullptr)
        return;

    if (bin<0) {
        LKHit *hit = nullptr;
        for (auto hitArray : {fHitCenterArray,fHitLChainArray,fHitRChainArray}) {
            auto numHits = hitArray -> GetEntries();
            for (auto iHit=0; iHit<numHits; ++iHit)
                hit = (LKHit *) hitArray -> At(iHit);
        }
        if (hit==nullptr)
            return;
        auto caac = hit -> GetChannel();
        bin = fMapCAACToBin[caac];
    }

    //fHistArray

    MMChannel* channel = nullptr;
    auto caac = fMapBinToCAAC[bin];
    auto numChannels = fChannelArray -> GetEntries();
    for (auto iChannel=0; iChannel<numChannels; ++iChannel)
    {
        auto channel0 = (MMChannel* ) fChannelArray -> At(iChannel);
        if (caac==channel0->GetCAAC()) {
            channel = channel0;
            break;
        }
    }
    if (channel==nullptr)
        return;

    auto buffer = channel -> GetWavformY();
    fHistChannel -> Reset();
    for (auto tb=0; tb<360; ++tb)
        fHistChannel -> SetBinContent(tb+1,buffer[tb]);
    fCanvas -> cd(2);
    fHistChannel -> Draw();
    auto texat = (TexAT2*) fDetector;
    for (auto hitArray : {fHitCenterArray,fHitLChainArray,fHitRChainArray}) {
        auto numHits = hitArray -> GetEntries();
        for (auto iHit=0; iHit<numHits; ++iHit) {
            hit = (LKHit *) hitArray -> At(iHit);
            if (caac==hit -> GetChannelID()) {
                auto caac0 = caac;
                auto cobo = int(caac0/10000); caac0 - cobo*10000;
                auto asad = int(caac0/1000); caac0 - asad*1000;
                auto aget = int(caac0/100); caac0 - aget*100;
                auto chan = caac0;
                auto electronicsID = texat -> GetElectronicsID(cobo,asad,aget,chan);
                auto pulse = texat -> GetChannelAnalyzer(electronicsID) -> GetPulse();
                pulse -> GetPulseGraph(hit->GetTb(), hit -> GetCharge());
                pulse -> Draw("samel");
            }
        }
    }

    //fHistChannel -> Fill();
    fCanvas -> Modified();
    fCanvas -> Update();
}

void TTMicromegasCenterChain::DrawFrame(Option_t *option)
{
    ;
}

void TTMicromegasCenterChain::Draw(Option_t *option)
{
    //SetDataFromBranch();
    FillDataToHist();

    auto cvs = GetCanvas();

    cvs -> cd(1);
    fHistPlane -> Draw("colz")
    DrawFrame();

    cvs -> cd(2);
    fHistChannel -> Draw()
}

void TTMicromegasCenterChain::MouseClickEvent()
{
    TObject* select = ((TCanvas*)gPad) -> GetClickSelected();
    if (select == nullptr)
        return;

    bool isNotH2 = !(select -> InheritsFrom(TH2::Class()));
    bool isNotGraph = !(select -> InheritsFrom(TGraph::Class()));
    if (isNotH2 && isNotGraph)
        return;

    TH2D* hist = (TH2D*) select;

    Int_t xEvent = gPad -> GetEventX();
    Int_t yEvent = gPad -> GetEventY();

    Float_t xAbs = gPad -> AbsPixeltoX(xEvent);
    Float_t yAbs = gPad -> AbsPixeltoY(yEvent);
    Double_t xOnClick = gPad -> PadtoX(xAbs);
    Double_t yOnClick = gPad -> PadtoY(yAbs);

    Int_t bin = hist -> FindBin(xOnClick, yOnClick);
    gPad -> SetUniqueID(bin);
    gPad -> GetCanvas() -> SetClickSelected(NULL);

    TTMicromegasCenterChain::GetMMCC() -> FillChannelBuffer(bin);
}
