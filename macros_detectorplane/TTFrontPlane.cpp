#include "TTFrontPlane.h"

ClassImp(TTFrontPlane);

TTFrontPlane* TTFrontPlane::fInstance = nullptr;
TTFrontPlane* TTFrontPlane::GetMMCC() { return fInstance; }

TTFrontPlane::TTFrontPlane()
{
    fInstance = this;
    fName = "TTFrontPlane";
}

bool TTFrontPlane::Init()
{
    e_info << "Initializing TTFrontPlane" << std::endl;

    fHistChannelChain = new TH1D("hist_channel_buffer_chain","channel buffer;time-bucket;charge",360,0,360);
    for (auto histChannel : {fHistChannelChain}) {
        histChannel -> SetStats(0);
        histChannel -> GetXaxis() -> SetLabelSize(0.065);
        histChannel -> GetYaxis() -> SetLabelSize(0.065);
        histChannel -> GetXaxis() -> SetTitleSize(0.065);
        histChannel -> GetYaxis() -> SetTitleSize(0.065);
        histChannel -> GetXaxis() -> SetTitleOffset(1.20);
        histChannel -> GetYaxis() -> SetTitleOffset(0.68);
    }

    int cobo, asad, aget, chan, dumi;
    double x0, x1, x2, z0, z1, z2;
    TString mapFileNameChain;
    if (fPar!=nullptr) {
        mapFileNameChain = fPar -> GetParString("TTFrontPlane/map_position_center_chain");
    }
    if (mapFileNameChain.IsNull()) mapFileNameChain = "position_channels_center_and_chain.txt";

    fHistPlaneChain = new TH2Poly("hist_TexATv2_plane_center_chain","TexAT2 Micromegas Center and Chain;x (mm);z (beam-axis)",-125,125,180,420);
    fHistPlaneChain -> SetStats(0);
    ifstream fileCC(mapFileNameChain);
    while (fileCC >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) {
        if (fXMin > x1) fXMin < x1;
        if (fXMax < x1) fXMin < x1;
        if (fXMin > x2) fXMin < x2;
        if (fXMax < x2) fXMin < x2;
        if (fZMin > z1) fXMin < z1;
        if (fZMax < z1) fXMin < z1;
        if (fZMin > z2) fXMin < z2;
        if (fZMax < z2) fXMin < z2;
        auto bin = fHistPlaneChain -> AddBin(x1,z1,x2,z2);
        int caac = cobo*10000 + asad*1000 + aget*100 + chan;
        fMapCAACToBinChain.insert(std::pair<int, int>(caac,bin));
        fMapBinToCAACChain.insert(std::pair<int, int>(bin,caac));
        fMapBinToX1Chain.insert(std::pair<int, double>(bin,x1));
        fMapBinToX2Chain.insert(std::pair<int, double>(bin,x2));
        fMapBinToZ1Chain.insert(std::pair<int, double>(bin,z1));
        fMapBinToZ2Chain.insert(std::pair<int, double>(bin,z2));
    }

    SetDataFromBranch();

    fGraphChannelBoundaryChain = new TGraph();
    fGraphChannelBoundaryChain -> SetLineColor(kRed);
    fGraphChannelBoundaryChain -> SetLineWidth(2);

    return true;
}

void TTFrontPlane::Clear(Option_t *option)
{
    LKPadPlane::Clear(option);
}

void TTFrontPlane::Print(Option_t *option) const
{
    e_info << "TTFrontPlane" << std::endl;
}

bool TTFrontPlane::IsInBoundary(Double_t x, Double_t z)
{
    if (x>fXMin && x<fXMax && z>fZMin && z<fZMax)
        return true;
    return true;
}

Int_t TTFrontPlane::FindChannelID(Double_t x, Double_t y)
{
    //return fHistPlaneChain -> FindBin(x,y);
    return -1;
}

Int_t TTFrontPlane::FindChannelID(Int_t section, Int_t row, Int_t layer)
{
    // example find id
    // int id = 10000*section + 100*row + layer;
    // return id;
    return -1;
}

TCanvas* TTFrontPlane::GetCanvas(Option_t *option)
{
    if (fCanvas==nullptr) {
        fCanvas = new TCanvas("TTFrontPlane","",550,700);
        auto pad1 = new TPad("pad1","",0,230./700,0.5,1);
        pad1 -> SetMargin(0.12,0.15,0.1,0.1);
        pad1 -> SetNumber(1);
        pad1 -> AddExec("ex", "TTFrontPlane::MouseClickEventChain()");
        pad1 -> Draw();
        auto pad2 = new TPad("pad2","",0,0,0.5,230./700);
        pad2 -> SetMargin(0.12,0.05,0.20,0.12);
        pad2 -> SetNumber(2);
        pad2 -> Draw();
        fCanvas -> Modified();
    }
    return fCanvas;
}

TH2* TTFrontPlane::GetHist(Option_t *option)
{
    return (TH2Poly *) fHistPlaneChain;
}

bool TTFrontPlane::SetDataFromBranch()
{
    if (fRun==nullptr)
        return false;

    fBufferArray = fRun -> GetBranchA("RawData");
    fHitCenterArray = fRun -> GetBranchA("HitCenter");
    fHitLChainArray = fRun -> GetBranchA("HitLChain");
    fHitRChainArray = fRun -> GetBranchA("HitRChain");
    return true;
}

void TTFrontPlane::FillDataToHist()
{
    if (fHitCenterArray==nullptr)
        return;

    for (auto hitArray : {fHitCenterArray,fHitLChainArray,fHitRChainArray})
    {
        auto numHits = hitArray -> GetEntries();
        for (auto iHit=0; iHit<numHits; ++iHit)
        {
            auto hit = (LKHit *) hitArray -> At(iHit);
            auto position = hit -> GetPosition();
            fHistPlaneChain -> Fill(position.X(), position.Z());
        }
    }

    SelectAndDrawChannelChain();
}


void TTFrontPlane::SelectAndDrawChannel(bool isChain, Int_t bin)
{
    if (isChain) lk_info << "SelectAndDrawChannel (Chain) " << bin << endl;

    cvsPlane = fCanvas -> cd(1);
    cvsChannel = fCanvas -> cd(2);

    bool existHitArray = false;
    bool existBufferArray = (fBufferArray!=nullptr);
    existHitArray = (fHitCenterArray!=nullptr&&fHitLChainArray!=nullptr&&fHitRChainArray!=nullptr);

    vector<TClonesArray*> hitArrayList = {fHitCenterArray};
    hitArrayList.push_back(fHitLChainArray);
    hitArrayList.push_back(fHitRChainArray);
    if (bin<0) {
        if (existHitArray) {
            LKHit *hit = nullptr;
            for (auto hitArray : hitArrayList) {
                auto numHits = hitArray -> GetEntries();
                for (auto iHit=0; iHit<numHits; ++iHit)
                    hit = (LKHit *) hitArray -> At(iHit);
            }
            if (hit==nullptr)
                return;
            auto caac = hit -> GetChannelID();
            bin = fMapCAACToBinChain[caac];
        }
    }

    if (bin<0)
        return;

    cvsPlane -> cd();
    double x1,x2,z1,z2;
    TGraph* graphBoundary;
    x1 = fMapBinToX1Chain[bin];
    x2 = fMapBinToX2Chain[bin];
    z1 = fMapBinToZ1Chain[bin];
    z2 = fMapBinToZ2Chain[bin];
    graphBoundary = fGraphChannelBoundaryChain;
    double x0 = (x1 + x2)/2.;
    double z0 = (z1 + z2)/2.;
    graphBoundary -> Set(0);
    graphBoundary -> SetPoint(0,x1,z1);
    graphBoundary -> SetPoint(1,x2,z1);
    graphBoundary -> SetPoint(2,x2,z2);
    graphBoundary -> SetPoint(3,x1,z2);
    graphBoundary -> SetPoint(4,x1,z1);
    graphBoundary -> Draw("samel");

    int caac = 0;
    caac = fMapBinToCAACChain[bin];
    
    TH1D* histChannel = nullptr;
    histChannel = fHistChannelChain;
    histChannel -> SetTitle(Form("CAAC=%d, position=(%.2f, %.2f)",caac,x0,z0));
    
    cvsChannel -> Modified();
    cvsChannel -> Update();

    if (!existBufferArray)
        return;

    MMChannel* channel = nullptr;
    auto numChannels = fBufferArray -> GetEntries();
    for (auto iChannel=0; iChannel<numChannels; ++iChannel)
    {
        auto channel0 = (MMChannel* ) fBufferArray -> At(iChannel);
        if (caac==channel0->GetCAAC()) {
            channel = channel0;
            break;
        }
    }
    if (channel==nullptr)
        return;

    auto buffer = channel -> GetWaveformY();
    histChannel -> Reset();
    for (auto tb=0; tb<360; ++tb)
        histChannel -> SetBinContent(tb+1,buffer[tb]);
    cvsChannel -> cd();
    histChannel -> Draw();
    auto texat = (TexAT2*) fDetector;
    //if (texat==nullptr) ...;
    if (existHitArray)
    {
        for (auto hitArray : hitArrayList) {
            auto numHits = hitArray -> GetEntries();
            for (auto iHit=0; iHit<numHits; ++iHit) {
                auto hit = (LKHit *) hitArray -> At(iHit);
                if (caac==hit -> GetChannelID()) {
                    auto caac0 = caac;
                    auto cobo = int(caac0/10000); caac0 - cobo*10000;
                    auto asad = int(caac0/1000); caac0 - asad*1000;
                    auto aget = int(caac0/100); caac0 - aget*100;
                    auto chan = caac0;
                    if (texat!=nullptr) {
                        auto electronicsID = texat -> GetElectronicsID(cobo,asad,aget,chan);
                        auto pulse = texat -> GetChannelAnalyzer(electronicsID) -> GetPulse();
                        pulse -> GetPulseGraph(hit->GetY(), hit -> GetCharge());
                        pulse -> Draw("samel"); }
                }
            }
        }
    }

    fCanvas -> Modified();
    fCanvas -> Update();
}

void TTFrontPlane::DrawFrame(Option_t *option)
{
    ;
}

void TTFrontPlane::Draw(Option_t *option)
{
    //SetDataFromBranch();
    FillDataToHist();

    auto cvs = GetCanvas();

    cvs -> cd(1);
    if (fHistPlaneChain->GetEntries()==0)
        fHistPlaneChain -> Draw();
    else
        fHistPlaneChain -> Draw("colz");

    cvs -> cd(2);
    fHistChannelChain -> Draw();
    cvs -> cd(2) -> Modified();
    cvs -> cd(2) -> Update();
    auto ttt1 = (TPaveText*) (cvs->cd(2)->GetListOfPrimitives()) -> FindObject("title");
    ttt1 -> SetTextSize(0.065);
    ttt1 -> SetTextAlign(12);
    cvs -> cd(2) -> Modified();
    cvs -> cd(2) -> Update();

}

void TTFrontPlane::MouseClickEventChain()
{
    if (gPad==nullptr)
        return;

    TObject* select = gPad -> GetCanvas() -> GetClickSelected();
    if (select == nullptr)
        return;

    bool isNotH2 = !(select -> InheritsFrom(TH1::Class()));
    //bool isNotGraph = !(select -> InheritsFrom(TGraph::Class()));
    //if (isNotH2 && isNotGraph)
    if (isNotH2)
        return;

    int xEvent = gPad -> GetEventX();
    int yEvent = gPad -> GetEventY();
    int xAbs = gPad -> AbsPixeltoX(xEvent);
    int yAbs = gPad -> AbsPixeltoY(yEvent);
    double xOnClick = gPad -> PadtoX(xAbs);
    double yOnClick = gPad -> PadtoY(yAbs);

    TH2* hist = dynamic_cast<TH2*> (select);
    int binCurr = hist -> FindBin(xOnClick, yOnClick);
    int binLast = gPad -> GetUniqueID();
    if (binCurr==binLast)
        return;

    gPad -> SetUniqueID(binCurr);
    gPad -> GetCanvas() -> SetClickSelected(nullptr);

    if (binCurr<=0)
        return;

    TTFrontPlane::GetMMCC() -> SelectAndDrawChannelChain(binCurr);
}
