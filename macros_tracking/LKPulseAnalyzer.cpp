#include "LKPulseAnalyzer.h"
#include "TFile.h"
#include "TLine.h"
#include "TText.h"
#include "TMath.h"

using namespace std;

ClassImp(LKPulseAnalyzer);

LKPulseAnalyzer::LKPulseAnalyzer(const char* name, const char *path)
{
    fName = name;
    fPath = path;
    Init();
}

bool LKPulseAnalyzer::Init()
{
    fFile = new TFile(Form("%s/summary_%s.root",fPath,fName),"recreate");
    fTree = new TTree("pulse","");
    fTree -> Branch("isCollected", &fIsCollected);
    fTree -> Branch("isSingle", &fIsSinglePulseChannel);
    fTree -> Branch("numPulse", &fCountPulse);
    fTree -> Branch("pedestal", &fPedestalPry);
    fTree -> Branch("tbAtMax", &fTbAtMaxValue);
    fTree -> Branch("height", &fMaxValue);
    fTree -> Branch("width", &fWidth); 
    fTree -> Branch("event", &fEventID);
    fTree -> Branch("cobo", &fCobo);
    fTree -> Branch("asad", &fAsad);
    fTree -> Branch("aget", &fAget);
    fTree -> Branch("channel", &fChannel);

    fHistArray = new TObjArray();
    fHistWidth = new TH1D(Form("histWidth_%s",fName),Form("[%s]  Width;width (tb);count",fName),200,0,200);
    fHistHeight = new TH1D(Form("histHeight_%s",fName),Form("[%s]  Height;height;count",fName),105,0,4200);
    fHistPulseTb = new TH1D(Form("histPulseTb_%s",fName),Form("[%s]  PulseTb;tb_{pulse};count",fName),128,0,512);
    fHistPedestal = new TH1D(Form("histPedestal_%s",fName),Form("[%s]  Pedestal;pedestal;count",fName),200,0,1000);
    fHistReusedData = new TH1D(Form("histReusedData_%s",fName),";tb;y",fTbMax,-100,fTbMax-100);
    fHistAccumulate = new TH2D(Form("histAccumulate_%s",fName),";tb;y",fTbMax,-100,fTbMax-100,150,-.25,1.25);
    fHistPedestalPry = new TH1D(Form("histPedestalPry_%s",fName),Form("[%s]  Pedestal preliminary;pedestal;count",fName),105,0,4200);
    fHistHeightWidth = new TH2D(Form("histHeightWidth_%s",fName),Form("[%s];height;width",fName),210,0,4200,100,0,100);
    fHistAccumulate -> SetStats(0);
    Clear();
    return true;
}

void LKPulseAnalyzer::Clear(Option_t *option)
{
    for (auto tb=0; tb<512; ++tb) fAverageData[tb] = 0;
    for (auto tb=0; tb<512; ++tb) fChannelData[tb] = 0;

    if (fHistAverage!=nullptr)
        fHistAverage -> Reset("ICES");
    fHistAccumulate -> Reset("ICES");
}

void LKPulseAnalyzer::Print(Option_t *option) const
{
}

void LKPulseAnalyzer::AddChannel(int *data, int event, int cobo, int asad, int aget, int channel)
{
    fEventID = event;
    fCobo = cobo;
    fAsad = asad;
    fAget = aget;
    fChannel = channel;
    auto caac = event*1000000 + cobo*100000 + asad*10000 + aget*1000 + channel;
    AddChannel(data, caac);
}

void LKPulseAnalyzer::AddChannel(int *data, int channelID)
{
    fPreValue = 0;
    fCurValue = 0;
    fCountPulse = 0;
    fCountWidePulse = 0;
    fCountPedestalPry = 0;
    fCountTbWhileAbove = 0;
    fValueIsAboveThreshold = false;

    fIsSinglePulseChannel = false;
    fIsCollected = false;
    fFirstPulseTb = -1;
    fTbAtMaxValue = 0;
    fPedestalPry = 0;
    fMaxValue = 0;
    fWidth = 0;

    fChannelID = channelID;
    if (fInvertChannel)
        for (auto tb=fTbStart; tb<fTbMax; ++tb)
            fChannelData[tb] = fChannelMax - data[tb];
    else
        for (auto tb=fTbStart; tb<fTbMax; ++tb)
            fChannelData[tb] = data[tb];

    for (auto tb=fTbStart; tb<fTbMax; ++tb)
    {
        fCurValue = fChannelData[tb];
        if (fCurValue>fMaxValue) {
            fMaxValue = fCurValue;
            fTbAtMaxValue = tb;
        }

        if (fPreValue>=fThreshold && fCurValue>=fThreshold) {
            fCountTbWhileAbove++;
        }
        // >> going up
        else if (fPreValue<fThreshold && fCurValue>=fThreshold) {
            fValueIsAboveThreshold = true;
            fCountTbWhileAbove = 0;
            fFirstPulseTb = tb;
        }
        // << going down
        else if (fPreValue>=fThreshold && fCurValue<fThreshold) {
            fValueIsAboveThreshold = false;
            fHistWidth -> Fill(fCountTbWhileAbove);
            if (fCountTbWhileAbove>=fPulseWidthAtThresholdMin && fCountTbWhileAbove<fPulseWidthAtThresholdMax)
                fCountPulse++;
            else if (fCountTbWhileAbove>fPulseWidthAtThresholdMax)
                fCountWidePulse++;
            else
                fFirstPulseTb = -1;
        }
        else {
            fPedestalPry += fCurValue;
            fCountPedestalPry++;
        }

        fPreValue = fCurValue;
    }

    fPedestalPry = fPedestalPry / fCountPedestalPry;
    if (fFixPedestal>-999) fPedestalPry = fFixPedestal;
    fWidth = fCountTbWhileAbove;

    if (fValueIsAboveThreshold && fCountTbWhileAbove>fPulseWidthAtThresholdMax) {
        fHistWidth -> Fill(fCountTbWhileAbove);
        fCountWidePulse++;
    }

    fHistHeight -> Fill(fMaxValue);
    fHistPulseTb -> Fill(fFirstPulseTb);

    if (fCountPulse==1 && fCountWidePulse==0 && fMaxValue>fPulseHeightMin && fMaxValue<fPulseHeightMax)
    {
        fIsSinglePulseChannel = true;

        auto tb1 = fTbStart;
        auto tb2 = fTbAtMaxValue-40;
        auto tb3 = fTbAtMaxValue+40;
        auto tb4 = fTbMax;

        fPedestal = 0.;
        int countPedestal = 0;
        for (auto tb=tb1; tb<tb2; ++tb) {
            fPedestal += fChannelData[tb];
            countPedestal++;
        }
        for (auto tb=tb3; tb<tb4; ++tb) {
            fPedestal += fChannelData[tb];
            countPedestal++;
        }

        fPedestal = fPedestal/countPedestal;
        if (fFixPedestal>-999) fPedestal = fFixPedestal;
        fHistPedestal -> Fill(fPedestal);
        fPedestalPry = fPedestal;

        if (fFirstPulseTb>fPulseTbMin && fFirstPulseTb<fPulseTbMax)
        {
            fIsCollected = true;
            fCountGoodChannels++;

            double scale = 1./(fMaxValue-fPedestal)*1;
            fHistReusedData -> Reset("ICES");
            for (auto tb=fTbStart; tb<fTbMax; ++tb)
            {
                int tb_aligned = tb - fTbAtMaxValue + 100;
                if (tb_aligned<0 || tb_aligned>fTbMax)
                    continue;

                double value = (fChannelData[tb] - fPedestal) * scale;
                fAverageData[tb_aligned] += value;

                int tb_aligned2 = tb - fTbAtMaxValue;
                fHistAccumulate -> Fill(tb_aligned2, value);

                fHistReusedData -> SetBinContent(tb_aligned,value);
            }

            fWidth = FullWidthRatioMaximum(fHistReusedData,0.05,10);
            fHistHeightWidth -> Fill(fMaxValue-fPedestal,fWidth);
        }
    }

    fHistPedestalPry -> Fill(fPedestalPry);

    fTree -> Fill();
}

void LKPulseAnalyzer::DumpChannel(Option_t *option)
{
    const char* fileName = Form("%s/buffer_%s_%d.dat",fPath,fName,fChannelID);
    ofstream file_out(fileName);
    cout << fileName << endl;
    if (TString(option)=="raw")
        for (auto tb=0; tb<fTbMax; ++tb)
            file_out << fChannelData[tb] << endl;
    else {
        for (auto tb=0; tb<fTbMax; ++tb)
            file_out << fChannelData[tb] - fPedestal << endl;
    }
}

void LKPulseAnalyzer::WriteTree()
{
    fFile -> cd();
    fTree -> Write();
    cout << fFile -> GetName() << endl;
}

bool LKPulseAnalyzer::DrawChannel()
{
    auto hist = new TH1D(Form("channel_%s_%d",fName,fCountHistChannel),Form("%s %d;tb;y",fName,fChannelID),fTbMax,0,fTbMax);
    for (auto tb=0; tb<fTbMax; ++tb)
        hist -> SetBinContent(tb+1,fChannelData[tb]);

    bool cvsIsNew = false;
    if (fCountChannelPad==0) {
        cvsIsNew = true;
        fCvsGroup = new TCanvas(Form("cvsGroup_%s_%d",fName,fCountCvsGroup),"",fWGroup,fHGroup);
        fCvsGroup -> Divide(fXGroup,fYGroup);
        fCountCvsGroup++;
    }

    fCvsGroup -> cd(fCountChannelPad+1);

    if (fIsCollected) {
        double dy = fMaxValue - fPedestal;
        double y1 = fPedestal - dy*0.20;
        double y2 = fMaxValue + dy*0.20;
        if (y1<=0) y1 = 0;
        if (y2>fChannelMax) y2 = fChannelMax;
        hist -> SetMinimum(y1);
        hist -> SetMaximum(y2);
    }
    else {
        hist -> SetMaximum(fChannelMax);
        hist -> SetMinimum(0);
    }
    hist -> Draw();

    if (fIsCollected) {
        auto lineC = new TLine(fTbAtMaxValue,fPedestal,fTbAtMaxValue,fMaxValue);
        lineC -> SetLineColor(kBlue);
        lineC -> SetLineStyle(2);
        lineC -> Draw();

        auto lineT = new TLine(0,fThreshold,fTbMax,fThreshold);
        lineT -> SetLineColor(kGreen);
        lineT -> SetLineStyle(2);
        lineT -> Draw();

        auto line1 = new TLine(0,fPulseHeightMin,fTbMax,fPulseHeightMin);
        line1 -> SetLineColor(kRed);
        line1 -> SetLineStyle(2);
        line1 -> Draw();

        auto line2 = new TLine(0,fPulseHeightMax,fTbMax,fPulseHeightMax);
        line2 -> SetLineColor(kRed);
        line2 -> SetLineStyle(2);
        line2 -> Draw();

        auto lineP = new TLine(0,fPedestal,fTbMax,fPedestal);
        lineP -> SetLineColor(kViolet);
        lineP -> SetLineStyle(2);
        lineP -> Draw();
    }

    fCountChannelPad++;
    if (fCountChannelPad==fXGroup*fYGroup)
        fCountChannelPad = 0;

    fCountHistChannel++;

    return cvsIsNew;
}

TCanvas* LKPulseAnalyzer::DrawAverage(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsAverage = (TCanvas*) pad;
    else if (fCvsAverage==nullptr)
        fCvsAverage = new TCanvas(Form("cvsAverage_%s",fName),"",fWAverage,fHAverage);

    if (fHistAverage==nullptr)
        fHistAverage = new TH1D(Form("histAverage_%s",fName),";tb;y",fTbMax,-100,fTbMax-100);
    fHistAverage -> Reset("ICES");

    SetCvs(fCvsAverage);
    SetHist(fHistAverage);

    for (auto tb=0; tb<fTbMax; ++tb)
        fHistAverage -> SetBinContent(tb+1,fAverageData[tb]/fCountGoodChannels);
    fHistAverage -> SetTitle(Form("[%s] %d channels",fName,fCountGoodChannels));
    fHistAverage -> SetStats(0);

    fCvsAverage -> cd();
    fHistAverage -> Draw();

    auto lineC = new TLine(0,0,0,1);
    auto lineP = new TLine(-100,1,fTbMax-100,1);
    auto lineT = new TLine(-100,0,fTbMax-100,0);
    for (auto line : {lineC,lineP,lineT}) {
        line -> SetLineColor(kRed);
        line -> SetLineStyle(2);
        line -> Draw();
    }

    //for (auto ratio : {0.10, 0.25, 0.50, 0.75})
    for (auto ratio : {0.05, 0.25, 0.50, 0.75})
    {
        double tb1,tb2,error;
        auto width = FullWidthRatioMaximum(fHistAverage,ratio,10,tb1,tb2,error);
        auto ratio100 = ratio*100;
        auto tt = new TText(60,ratio100,Form("%.2f (at y=%d)",width,int(ratio100)));
        tt -> SetTextFont(132);
        tt -> SetTextSize(0.07);
        tt -> SetTextAlign(12);
        tt -> Draw();
        auto line = new TLine(tb1,ratio100,tb2,ratio100);
        line -> SetLineColor(kBlue);
        line -> SetLineWidth(4);
        line -> Draw();

        if (ratio==0.05) {
            fTbAtRefFloor1 = tb1;
            fTbAtRefFloor2 = tb2;
            fRefWidth = tb2 - tb1;
        }
    }

    return fCvsAverage;
}

TCanvas* LKPulseAnalyzer::DrawAccumulate(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsAccumulate = (TCanvas*) pad;
    else if (fCvsAccumulate==nullptr)
        fCvsAccumulate = new TCanvas(Form("cvsAccumulate_%s",fName),"",fWAverage,fHAverage);

    SetCvs(fCvsAccumulate);
    SetHist(fHistAccumulate);

    fCvsAccumulate -> cd();
    fCvsAccumulate -> SetLogz();
    fHistAccumulate -> SetMinimum(1);
    fHistAccumulate -> Draw("colz");
    fHistAccumulate -> SetTitle(Form("[%s] %d channels",fName,fCountGoodChannels));
    if (fHistAverage!=nullptr) {
        auto histClone = (TH1D *) fHistAverage -> Clone(Form("histAverageClone_%s",fName));
        histClone -> SetLineColor(kRed);
        histClone -> Draw("samel");
    }

    return fCvsAccumulate;
}

void LKPulseAnalyzer::DrawAccumulatePY()
{
    if (fHistAverage!=nullptr && fHistAccumulate!=nullptr)
    {
        int numBinsY = 80;
        double rMax = .40;

        for (auto tBin=1; tBin<=fTbMax; ++tBin)
        {
            auto yAverage = fHistAverage -> GetBinContent(tBin);
            auto y1 = yAverage - rMax;
            auto y2 = yAverage + rMax;
            auto dy = (y2-y1) / numBinsY;

            auto hist = fHistAccumulate -> ProjectionY(Form("%s_px_ybin%d",fHistAccumulate->GetName(),tBin), tBin, tBin);
            hist -> SetTitle(Form("[%s] xbin-%d",fName,tBin));
            auto mean = hist -> GetMean();
            auto sd = hist -> GetStdDev();
            hist -> GetXaxis() -> SetRangeUser(mean-5*sd, mean+5*sd);
            fHistArray -> Add(hist);
        }
    }
}

TCanvas* LKPulseAnalyzer::DrawResidual(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsResidual = (TCanvas*) pad;
    else if (fCvsResidual==nullptr)
        fCvsResidual = new TCanvas(Form("cvsResidual_%s",fName),"",fWAverage,fHAverage);

    fCvsResidual -> SetLogz();

    if (fHistAverage!=nullptr && fHistAccumulate!=nullptr)
    {
        int numBinsY = 40;
        double rMax = .20;
        fHistResidual = new TH2D(Form("histResidual_%s",fName),Form("[%s];r;y residual;",fName),
                fTbMax,-100,fTbMax-100, numBinsY,-rMax,rMax);

        for (auto tBin=1; tBin<=fTbMax; ++tBin)
        {
            auto yAverage = fHistAverage -> GetBinContent(tBin);
            auto y1 = yAverage - rMax;
            auto y2 = yAverage + rMax;
            auto dy = (y2-y1) / numBinsY;
            for (auto y=y1; y<=y2; y+=dy)
            {
                auto ybin = fHistAccumulate -> GetYaxis() -> FindBin(y);
                auto zValue = fHistAccumulate -> GetBinContent(tBin,ybin);
                double residual = yAverage - y;
                fHistResidual -> Fill(tBin-100-0.5,residual,zValue);
            }
        }

        SetCvs(fCvsResidual);
        SetHist(fHistResidual);

        fHistResidual -> Draw("colz");
    }

    return fCvsResidual;
}

TCanvas* LKPulseAnalyzer::DrawReference(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsReference = (TCanvas*) pad;
    else if (fCvsReference==nullptr)
        fCvsReference = new TCanvas(Form("cvsReference_%s",fName),"",fWAverage,fHAverage);

    if (fHistReference==nullptr)
        fHistReference = new TH2D(Form("histReference_%s",fName),";tb;y",100,0,50,140,-0.2,1.2);
    fHistReference -> SetTitle(Form("[%s] %d channels",fName,fCountGoodChannels));
    fHistReference -> Reset("ICES");
    fHistReference -> SetStats(0);

    SetCvs(fCvsReference);
    SetHist(fHistReference);

    if (fGraphReference==nullptr)
        fGraphReference = new TGraph();
    fGraphReference -> Clear();
    fGraphReference -> Set(0);
    fGraphReference -> SetMarkerStyle(20);
    fGraphReference -> SetMarkerSize(0.4);

    auto bin1 = fHistAverage -> FindBin(fTbAtRefFloor1);
    auto bin2 = fHistAverage -> FindBin(fTbAtRefFloor2);// + 20;
    for (auto bin=bin1; bin<=bin2; ++bin) {
        auto value = fHistAverage -> GetBinContent(bin);
        fGraphReference -> SetPoint(fGraphReference->GetN(),fGraphReference->GetN(),value);
    }

    fHistReference -> Draw();
    fGraphReference -> Draw("samepl");

    auto line0 = new TLine(0,0,50,0);
    line0 -> SetLineColor(kRed);
    line0 -> SetLineStyle(2);
    line0 -> Draw();

    return fCvsReference;
}

void LKPulseAnalyzer::WriteReferecePulse(int tbOffsetFromHead, int tbOffsetFromtail)
{
    if (fHistAverage==nullptr)
        return;

    if (fGraphReference==nullptr)
        fGraphReference = new TGraph();
    fGraphReference -> Clear();
    fGraphReference -> Set(0);

    auto bin1 = fHistAverage -> FindBin(fTbAtRefFloor1)-tbOffsetFromHead;
    auto bin2 = fHistAverage -> FindBin(fTbAtRefFloor2)+tbOffsetFromtail;
    for (auto bin=bin1; bin<=bin2; ++bin) {
        auto value = fHistAverage -> GetBinContent(bin);
        fGraphReference -> SetPoint(fGraphReference->GetN(),fGraphReference->GetN(),value);
    }

    auto file = new TFile(Form("%s/pulseReference_%s.root",fPath,fName),"recreate");
    fGraphReference -> Write("pulse");
}

TCanvas* LKPulseAnalyzer::DrawWidth(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsWidth = (TCanvas*) pad;
    else if (fCvsWidth==nullptr)
        fCvsWidth = new TCanvas(Form("cvsWidth_%s",fName),"",fWAverage,fHAverage);
    fHistWidth -> Draw();

    SetCvs(fCvsWidth);
    SetHist(fHistWidth);

    auto yMin = fHistWidth -> GetMinimum();
    auto yMax = fHistWidth -> GetMaximum();
    auto line1 = new TLine(fPulseWidthAtThresholdMin,yMin,fPulseWidthAtThresholdMin,yMax);
    auto line2 = new TLine(fPulseWidthAtThresholdMax,yMin,fPulseWidthAtThresholdMax,yMax);
    for (auto line : {line1,line2}) {
        line -> SetLineColor(kViolet);
        line -> SetLineStyle(2);
        line -> Draw();
    }

    return fCvsWidth;
}

TCanvas* LKPulseAnalyzer::DrawHeightWidth(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsHeightWidth = (TCanvas*) pad;
    else if (fCvsHeightWidth==nullptr)
        fCvsHeightWidth = new TCanvas(Form("cvsHeightWidth_%s",fName),"",fWAverage,fHAverage);
    fHistHeightWidth -> Draw("colz");

    SetCvs(fCvsHeightWidth);
    SetHist(fHistHeightWidth);

    return fCvsHeightWidth;
}

TCanvas* LKPulseAnalyzer::DrawHeight(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsHeight = (TCanvas*) pad;
    else if (fCvsHeight==nullptr)
        fCvsHeight = new TCanvas(Form("cvsHeight_%s",fName),"",fWAverage,fHAverage);
    fHistHeight -> Draw();

    SetCvs(fCvsHeight);
    SetHist(fHistHeight);

    fHistPedestalPry -> SetLineColor(kRed);
    fHistPedestalPry -> Draw("same");

    auto yMin = fHistHeight -> GetMinimum();
    auto yMax = fHistHeight -> GetMaximum();
    auto lineT = new TLine(fThreshold,yMin,fThreshold,yMax); lineT -> SetLineColor(kGreen);
    auto line1 = new TLine(fPulseHeightMin,yMin,fPulseHeightMin,yMax); line1 -> SetLineColor(kRed);
    auto line2 = new TLine(fPulseHeightMax,yMin,fPulseHeightMax,yMax); line2 -> SetLineColor(kRed);
    for (auto line : {lineT,line1,line2}) {
        line -> SetLineStyle(2);
        line -> Draw();
    }

    return fCvsHeight;
}

TCanvas* LKPulseAnalyzer::DrawPulseTb(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsPulseTb = (TCanvas*) pad;
    else if (fCvsPulseTb==nullptr)
        fCvsPulseTb = new TCanvas(Form("cvsPulseTb_%s",fName),"",fWAverage,fHAverage);
    fHistPulseTb -> Draw();

    auto yMin = fHistPulseTb -> GetMinimum();
    auto yMax = fHistPulseTb -> GetMaximum();
    auto line1 = new TLine(fPulseTbMin,yMin,fPulseTbMin,yMax);
    auto line2 = new TLine(fPulseTbMax,yMin,fPulseTbMax,yMax);
    for (auto line : {line1,line2}) {
        line -> SetLineColor(kViolet);
        line -> SetLineStyle(2);
        line -> Draw();
    }

    return fCvsPulseTb;
}

TCanvas* LKPulseAnalyzer::DrawPedestal(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsPedestal = (TCanvas*) pad;
    else if (fCvsPedestal==nullptr)
        fCvsPedestal = new TCanvas(Form("cvsPedestal_%s",fName),"",fWAverage,fHAverage);
    fHistPedestal -> Draw();

    SetCvs(fCvsPedestal);
    SetHist(fHistPedestal);

    auto yMin = fHistPedestal -> GetMinimum();
    auto yMax = fHistPedestal -> GetMaximum();
    auto lineT = new TLine(fThreshold,yMin,fThreshold,yMax); lineT -> SetLineColor(kGreen);
    auto line1 = new TLine(fPulseHeightMin,yMin,fPulseHeightMin,yMax); line1 -> SetLineColor(kRed);
    auto line2 = new TLine(fPulseHeightMax,yMin,fPulseHeightMax,yMax); line2 -> SetLineColor(kRed);
    for (auto line : {lineT,line1,line2}) {
        line -> SetLineStyle(2);
        line -> Draw();
    }

    return fCvsPedestal;
}

double LKPulseAnalyzer::FullWidthRatioMaximum(TH1D *hist, double ratioFromMax, double numSplitBin, double &tb1, double &tb2, double &error)
{
    int numBins = hist -> GetNbinsX();
    int binMax = hist -> GetMaximumBin();
    double xAtMax = hist -> GetBinCenter(binMax);
    double yMax = hist -> GetBinContent(binMax);
    double yAtRatio = ratioFromMax * yMax;
    double xMin = hist -> GetXaxis() -> GetBinLowEdge(1);
    double xMax = hist -> GetXaxis() -> GetBinUpEdge(numBins);
    double dx = (xMax-xMin)/numBins/numSplitBin;
    double yError1 = DBL_MAX;
    double yError0 = DBL_MAX;

    double x;
    double xLow;
    double xHigh;
    double yValue;
    double yError;

    for (x=xAtMax; x>=xMin; x-=dx)
    {
        yValue = hist -> Interpolate(x);
        yError = TMath::Abs(yValue - yAtRatio);
        if (yError > yError0) { xLow = x + dx; break; }
        yError0 = yError;
    }
    if (x==xMin)
        return -1;

    for (x=xAtMax; x<=xMax; x+=dx)
    {
        yValue = hist -> Interpolate(x);
        yError = TMath::Abs(yValue - yAtRatio);
        if (yError > yError1) { xHigh = x - dx; break; }
        yError1 = yError;
    }
    if (x==xMax)
        return -1;

    tb1 = xLow;
    tb2 = xHigh;
    error = TMath::Sqrt(yError0*yError0 + yError1*yError1);
    double width = xHigh - xLow;

    return width;
}

void LKPulseAnalyzer::SetCvs(TCanvas *cvs)
{ 
    cvs -> SetLeftMargin(0.1);
    cvs -> SetRightMargin(0.15);
}

void LKPulseAnalyzer::SetHist(TH1 *hist)
{
    hist -> GetXaxis() -> SetTitleOffset(1.25);
    hist -> GetYaxis() -> SetTitleOffset(1.50);
}
