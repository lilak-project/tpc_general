#include "LKChannelAnalyzer.h"

ClassImp(LKChannelAnalyzer);

LKChannelAnalyzer::LKChannelAnalyzer(const char* name)
{
    fName = name;
    Init();
}

bool LKChannelAnalyzer::Init()
{
    Clear();
    fHistWidth = new TH1D(Form("histWidth_%s",fName),Form("[%s]  Width;width (tb);count",fName),200,0,200);
    fHistHeight = new TH1D(Form("histHeight_%s",fName),Form("[%s]  Height;height;count",fName),205,0,4100);
    fHistPulseTb = new TH1D(Form("histPulseTb_%s",fName),Form("[%s]  PulseTb;tb_{pulse};count",fName),128,0,512);
    fHistPedestal = new TH1D(Form("histPedestal_%s",fName),Form("[%s]  Pedestal;pedestal;count",fName),200,0,1000);
    return true;
}

void LKChannelAnalyzer::Clear(Option_t *option)
{
    for (auto i=0; i<512; ++i) fAverageData[i] = 0;
    for (auto i=0; i<512; ++i) fChannelData[i] = 0;
}

void LKChannelAnalyzer::Print(Option_t *option) const
{
}

void LKChannelAnalyzer::AddChannel(int channelID, int *data)
{
    fPreValue = 0;
    fCurValue = 0;
    fMaxValue = 0;
    fCountPulse = 0;
    fTbAtMaxValue = 0;
    fFirstPulseTb = -1;
    fCountWidePulse = 0;
    fCountTbWhileAbove = 0;
    fIsGoodChannel = false;
    fValueIsAboveThreshold = false;

    fChannelID = channelID;
    if (fInvertChannel)
        for (auto i=0; i<fTbMax; ++i)
            fChannelData[i] = fChannelMax - data[i];
    else
        for (auto i=0; i<fTbMax; ++i)
            fChannelData[i] = data[i];

    for (auto tb = 0; tb<fTbMax; ++tb)
    {
        fCurValue = fChannelData[tb];
        if (fCurValue>fMaxValue) {
            fMaxValue = fCurValue;
            fTbAtMaxValue = tb;
        }

        if (fPreValue>=fThreshold && fCurValue>=fThreshold) {
            fCountTbWhileAbove++;
        }
        else if (fPreValue<fThreshold && fCurValue>=fThreshold) {
            fValueIsAboveThreshold = true;
            fCountTbWhileAbove = 0;
            fFirstPulseTb = tb;
        }
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

        fPreValue = fCurValue;
    }

    if (fValueIsAboveThreshold && fCountTbWhileAbove>fPulseWidthAtThresholdMax)
        fCountWidePulse++;

    fHistHeight -> Fill(fMaxValue);
    fHistPulseTb -> Fill(fFirstPulseTb);

    if (fCountPulse==1 && fCountWidePulse==0
            && fMaxValue>fPulseHeightMin && fMaxValue<fPulseHeightMax
            && fFirstPulseTb>fPulseTbMin && fFirstPulseTb<fPulseTbMax
       )
    {
        fCountGoodChannels++;
        fIsGoodChannel = true;

        auto tb1 = 0;
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
        fHistPedestal -> Fill(fPedestal);

        double scale = 1./(fMaxValue-fPedestal)*100;
        for (auto tb = 0; tb<fTbMax; ++tb)
        {
            int tb_aligned = tb - fTbAtMaxValue + 100;
            if (tb_aligned<0 || tb_aligned>fTbMax)
                continue;

            double value = fChannelData[tb] - fPedestal;

            fAverageData[tb_aligned] += value*scale;
        }
    }

    return fCountPulse;
}


bool LKChannelAnalyzer::DrawChannel()
{
    auto hist = new TH1D(Form("channel_%s_%d",fName,fCountHistChannel),Form("%s %d;tb;y",fName,fChannelID),fTbMax,0,fTbMax);
    for (auto tb = 0; tb<fTbMax; ++tb)
        hist -> SetBinContent(tb+1,fChannelData[tb]);

    bool cvsIsNew = false;
    if (fCountHistLocal==0) {
        cvsIsNew = true;
        fCvsGroup = new TCanvas(Form("cvsGroup_%s_%d",fName,fCountCvsGroup),"",fWGroup,fHGroup);
        fCvsGroup -> Divide(fXGroup,fYGroup);
        fCountCvsGroup++;
    }

    fCvsGroup -> cd(fCountHistLocal+1);

    if (fIsGoodChannel) {
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

    if (fIsGoodChannel) {
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

    fCountHistLocal++;
    if (fCountHistLocal==fXGroup*fYGroup)
        fCountHistLocal = 0;

    fCountHistChannel++;

    return cvsIsNew;
}

TCanvas* LKChannelAnalyzer::DrawAverage(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsAverage = (TCanvas*) pad;
    else if (fCvsAverage==nullptr)
        fCvsAverage = new TCanvas(Form("cvsAverage_%s",fName),"",fWAverage,fHAverage);

    if (fHistAverage==nullptr)
        fHistAverage = new TH1D(Form("histAverage_%s",fName),";tb;y",fTbMax,-100,fTbMax-100);
    fHistAverage -> Reset("ICES");

    for (auto tb = 0; tb<fTbMax; ++tb)
        fHistAverage -> SetBinContent(tb+1,fAverageData[tb]/fCountGoodChannels);
    fHistAverage -> SetTitle(Form("[%s]  %d channels, cut(%d,%d,%d)",fName,fCountGoodChannels,fThreshold,fPulseHeightMin,fPulseHeightMax));
    fHistAverage -> SetStats(0);

    fCvsAverage -> cd();
    fHistAverage -> Draw();

    auto lineC = new TLine(0,0,0,100);
    auto lineP = new TLine(-100,100,fTbMax-100,100);
    auto lineT = new TLine(-100,0,fTbMax-100,0);
    for (auto line : {lineC,lineP,lineT}) {
        line -> SetLineColor(kRed);
        line -> SetLineStyle(2);
        line -> Draw();
    }

    for (auto ratio : {0.10, 0.25, 0.50, 0.75})
    {
        double x0,x1,error;
        auto width = FullWidthRatioMaximum(fHistAverage,ratio,10,x0,x1,error);
        auto ratio100 = ratio*100;
        auto tt = new TText(60,ratio100,Form("%.2f (at y=%d)",width,int(ratio100)));
        tt -> SetTextFont(132);
        tt -> SetTextSize(0.07);
        tt -> SetTextAlign(12);
        tt -> Draw();
        auto line = new TLine(x0,ratio100,x1,ratio100);
        line -> SetLineColor(kBlue);
        line -> SetLineWidth(4);
        line -> Draw();
    }

    return fCvsAverage;
}

TCanvas* LKChannelAnalyzer::DrawWidth(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsWidth = (TCanvas*) pad;
    else if (fCvsWidth==nullptr)
        fCvsWidth = new TCanvas(Form("cvsWidth_%s",fName),"",fWAverage,fHAverage);
    fHistWidth -> Draw();

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

TCanvas* LKChannelAnalyzer::DrawHeight(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsHeight = (TCanvas*) pad;
    else if (fCvsHeight==nullptr)
        fCvsHeight = new TCanvas(Form("cvsHeight_%s",fName),"",fWAverage,fHAverage);
    fHistHeight -> Draw();

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

TCanvas* LKChannelAnalyzer::DrawPulseTb(TVirtualPad *pad)
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

TCanvas* LKChannelAnalyzer::DrawPedestal(TVirtualPad *pad)
{
    if (pad!=nullptr)
        fCvsPedestal = (TCanvas*) pad;
    else if (fCvsPedestal==nullptr)
        fCvsPedestal = new TCanvas(Form("cvsPedestal_%s",fName),"",fWAverage,fHAverage);
    fHistPedestal -> Draw();

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

double LKChannelAnalyzer::FullWidthRatioMaximum(TH1D *hist, double ratioFromMax, double numSplitBin, double &x0, double &x1, double &error)
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

    x0 = xLow;
    x1 = xHigh;
    error = TMath::Sqrt(yError0*yError0 + yError1*yError1);
    double width = xHigh - xLow;

    return width;
}
