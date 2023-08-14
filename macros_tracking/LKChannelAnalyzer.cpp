#include "LKChannelAnalyzer.h"

ClassImp(LKChannelAnalyzer);

LKChannelAnalyzer::LKChannelAnalyzer()
{
}

bool LKChannelAnalyzer::Init()
{
    //e_info << "Initializing LKChannelAnalyzer" << std::endl;

    return true;
}

void LKChannelAnalyzer::SetPulse(const char* fileName)
{
    fPulse = new LKPulse(fileName);
    fNDFPulse = fPulse -> GetNDF();
}

void LKChannelAnalyzer::Clear(Option_t *option)
{
    TObject::Clear(option);
    fNumHits = 0;
    //fChannelHitArray.clear();
}

void LKChannelAnalyzer::Print(Option_t *option) const
{
}

void LKChannelAnalyzer::Analyze(double* data)
{
    memcpy(&fBuffer, data, sizeof(double)*fTbMax);

    // Found peak information
    int tbPointer = fTbStart; // start of tb for analysis = 0
    int tbStartOfPulse;

    // Fitted hit information
    double tbHit;
    double amplitude;
    double squareSum;
    int ndf = fNDFPulse;

    // Previous hit information
    double tbHitPrev = fTbStart;
    double amplitudePrev = 0;

    while (FindPeak(fBuffer, tbPointer, tbStartOfPulse))
    {
        if (tbStartOfPulse > fTbStartCut-1)
            break;

        bool isSaturated = false;
        if (FitPulse(fBuffer, tbStartOfPulse, tbPointer, tbHit, amplitude, squareSum, ndf, isSaturated) == false)
            continue;

        // Pulse is found!

        if (TestPulse(fBuffer, tbHitPrev, amplitudePrev, tbHit, amplitude)) 
        {
            //auto channelHit = LKChannelHit(tbHit,amplitude);
            //fChannelHitArray.push_back(channelHit);

            tbHitPrev = tbHit;
            amplitudePrev = amplitude;
            if (isSaturated)
                tbPointer = int(tbHit) + 15;
            else
                tbPointer = int(tbHit) + 9;
        }
    }
}

bool LKChannelAnalyzer::FindPeak(double *buffer, int &tbPointer, int &tbStartOfPulse)
{ return true; }
/*
{
    int countAscending      = 0;
    int countAscendingBelow = 0;

    for (; tbPointer<fTbEnd; tbPointer++)
    {
        double value = buffer[tbPointer]
        double diff = value - buffer[tbPointer - 1];

        // If buffer difference of step is above threshold
        if (diff > fThresholdOneTbStep) 
        {
            if (value > fThreshold) countAscending++;
            else countAscendingBelow++;
        }
        else 
        {
            // If acended step is below 5, 
            // or negative pulse is bigger than the found pulse, continue
            //if (countAscending < fNumAcendingCut || ((countAscendingBelow >= countAscending) && (-buffer[tbPointer - 1 - countAscending - countAscendingBelow] > buffer[tbPointer - 1])))
            if (countAscending < fNumAcendingCut)
            {
                countAscending = 0;
                countAscendingBelow = 0;
                continue;
            }

            tbPointer -= 1;
            if (value < fThreshold)
                continue;

            // Peak is found!
            tbStartOfPulse = tbPointer - countAscending;
            while (buffer[tbStartOfPulse] < value * 0.05)
                tbStartOfPulse++;

            return true;
        }
    }

    return false;
}
*/

bool LKChannelAnalyzer::FitPulse(double *buffer, int tbStartOfPulse, int tbPeak,
        double &tbHit, 
        double &amplitude,
        double &chi2Fitted,
        int    &ndf,
        bool   &isSaturated)
{
    double valuePeak = buffer[tbPeak];
    isSaturated = false;

    if (ndf > fNDFPulse)
        ndf = fNDFPulse;

    // if peak value is larger than fDynamicRange, the pulse is isSaturated
    if (valuePeak > fDynamicRange)
    {
        ndf = tbPeak - tbStartOfPulse;
        if (ndf > fNDFPulse)
            ndf = fNDFPulse;
        isSaturated = true;
    }

    //double scaleTbStep = fScaleTbStep / (valuePeak * valuePeak); // Weight of time-bucket step
    //double stepChi2NDFCut = fStepChi2NDFCut * (valuePeak * valuePeak); // Effective cut for dChi2NDFdTb
    double scaleTbStep = .5;
    double stepChi2NDFCut = 1;
#ifdef DEBUG_FITPULSE
    lk_debug << "scaleTbStep = " << scaleTbStep << endl;
    lk_debug << "stepChi2NDFCut = " << stepChi2NDFCut << endl;
#endif

    double chi2Prev = DBL_MAX; // Least-squares of previous fit
    double chi2Curr = DBL_MAX; // Least-squares of current fit
    double tbStep = - 0.1; // Time-bucket step to next fit
    double tbPrev = tbStartOfPulse + 1; // Pulse starting time-bucket of previous fit
    double tbCurr = tbPrev + tbStep; // Pulse starting time-bucket of current fit

    LeastSquareFitAtGivenTb(buffer, tbPrev, ndf, amplitude, chi2Prev);
    LeastSquareFitAtGivenTb(buffer, tbCurr, ndf, amplitude, chi2Curr);
    double dChi2NDFdTb = (chi2Prev - chi2Curr) / (tbCurr - tbPrev) / ndf; // this value is in order of 1
#ifdef DEBUG_FITPULSE
    lk_debug << tbPrev << " " << ndf << " | " << chi2Prev << " " << chi2Curr << " " << dChi2NDFdTb << endl;
    if (dGraphStep==nullptr) { dGraphStep = new TGraph(); dGraphStep -> SetMarkerStyle(20); dGraphStep -> SetMarkerSize(0.5); }
    if (dGraphTime==nullptr) { dGraphTime = new TGraph(); dGraphTime -> SetMarkerStyle(20); dGraphTime -> SetMarkerSize(0.5); }
    if (dGraphChi2==nullptr) { dGraphChi2 = new TGraph(); dGraphChi2 -> SetMarkerStyle(20); dGraphChi2 -> SetMarkerSize(0.5); }
    if (dGraphTbC2==nullptr) { dGraphTbC2 = new TGraph(); dGraphTbC2 -> SetMarkerStyle(20); dGraphTbC2 -> SetMarkerSize(0.5); }
    if (dGraphBeta==nullptr) { dGraphBeta = new TGraph(); dGraphBeta -> SetMarkerStyle(20); dGraphBeta -> SetMarkerSize(0.5); }
    dGraphStep -> Set(0);
    dGraphTime -> Set(0);
    dGraphChi2 -> Set(0);
    dGraphTbC2 -> Set(0);
    dGraphBeta -> Set(0);
#endif

    int countIteration = 1;
    bool firstCheckFlag = false; // Checking flag to apply cut twice in a row
    while (true)
    {
        /*
        if (tbStep!=0 && chi2Curr!=chi2Prev) {
#ifdef DEBUG_FITPULSE
            lk_debug << "break : tbStep==0 || chi2Curr!=chi2Prev: " << tbStep << " !=0 && " << chi2Curr << " != " << chi2Prev <<  endl;
            break;
#endif
        }
        */
        tbPrev = tbCurr;
        chi2Prev = chi2Curr;
        tbStep = scaleTbStep * dChi2NDFdTb; // = fScaleTbStep(50) / (valuePeak * valuePeak) * (chi2Prev - chi2Curr) / (tbCurr - tbPrev) / ndf;
        if (tbStep> 1) tbStep =  1;
        if (tbStep<-1) tbStep = -1;
        tbCurr = tbPrev + tbStep;
        if (tbCurr<0 || tbCurr>fTbStartCut) {
#ifdef DEBUG_FITPULSE
            lk_debug << "break tbCur<0 || tbCur<fTbStartCut : " << tbCurr << " " << fTbStartCut << endl;
#endif
            return false;
        }

        LeastSquareFitAtGivenTb(buffer, tbCurr, ndf, amplitude, chi2Curr);
        dChi2NDFdTb = -(chi2Curr - chi2Prev) / (tbCurr - tbPrev) / ndf; // this value is in order of 1
#ifdef DEBUG_FITPULSE
        dGraphStep -> SetPoint(dGraphStep->GetN(), dGraphStep->GetN(), tbStep     );
        dGraphTime -> SetPoint(dGraphTime->GetN(), dGraphTime->GetN(), tbCurr     );
        dGraphChi2 -> SetPoint(dGraphChi2->GetN(), dGraphChi2->GetN(), chi2Curr   );
        dGraphTbC2 -> SetPoint(dGraphTbC2->GetN(), tbCurr,             chi2Curr   );
        dGraphBeta -> SetPoint(dGraphBeta->GetN(), dGraphBeta->GetN(), dChi2NDFdTb);
#endif

        countIteration++;
#ifdef DEBUG_FITPULSE
        //lk_debug << "IT-" << countIteration << " tb: " << tbPrev << "->" << tbCurr << "(" << tbStep << ")" << ",  chi2: " << chi2Prev << "->" << chi2Curr << ",  step: " << dChi2NDFdTb << "<?" << stepChi2NDFCut << endl;
#endif

        if (abs(dChi2NDFdTb) < stepChi2NDFCut)
        {
            /*
            if (firstCheckFlag==true) {
                // break if the fit is good enough two times in a row
#ifdef DEBUG_FITPULSE
                lk_debug << "break : good chi2 exit" << endl;
#endif
                break;
            }
            else
                firstCheckFlag = true;
                */
        }
        else
            firstCheckFlag = false;

        if (countIteration >= fIterMax) {
#ifdef DEBUG_FITPULSE
            lk_debug << "break : iteration cut exit: " << countIteration << " < " << fIterMax << endl;
#endif
            break;
        }
    }

    if (dChi2NDFdTb > 0) { // pre-fit is better
        tbHit = tbPrev;
        chi2Fitted = chi2Prev;
    }
    else { // current-fit is better
        tbHit = tbCurr;
        chi2Fitted = chi2Curr;
    }

    return true;
}

bool LKChannelAnalyzer::TestPulse(double *buffer, double tbHitPrev, double amplitudePrev, double tbHit, double amplitude)
{ return true; }
/*
{
    int numTbsCorrection = fNumTbsCorrection;

    if (numTbsCorrection + int(tbHit) >= fTbMax)
        numTbsCorrection = fTbMax - int(tbHit);

    if (amplitude < fThreshold) 
    {
        return false;
    }

    if (amplitude < fPulseGenerator -> Pulse(tbHit + 9, amplitudePrev, tbHitPrev) / 2.5) 
    {
        for (int iTbPulse = -1; iTbPulse < numTbsCorrection; iTbPulse++) {
            int tb = int(tbHit) + iTbPulse;
            buffer[tb] -= fPulseGenerator -> Pulse(tb, amplitude, tbHit);
        }

        return false;
    }

    for (int iTbPulse = -1; iTbPulse < numTbsCorrection; iTbPulse++) {
        int tb = int(tbHit) + iTbPulse;
        buffer[tb] -= fPulseGenerator -> Pulse(tb, amplitude, tbHit);
    }


    return true;
}
*/

void LKChannelAnalyzer::LeastSquareFitAtGivenTb(double *buffer, double tbStartOfPulse, int ndf,
        double &amplitude,
        double &chi2)
{
    double refy = 0;
    double ref2 = 0;
    int tb0 = int(tbStartOfPulse); 
    double tbOffset = tbStartOfPulse - tb0; 
    for (int iTbPulse = 1; iTbPulse < ndf; iTbPulse++)
    {
        int tbData = tb0 + iTbPulse;
        double valueData = buffer[tbData];
        double tbRef = iTbPulse - tbOffset + 0.5;
        double valueRef = fPulse -> Eval(tbRef);
        double errorRef = fPulse -> Error(tbRef);
        double weigth = 1./(errorRef*errorRef);
        refy += weigth * valueRef * valueData;
        ref2 += weigth * valueRef * valueRef;
    }
    if (ref2==0) {
        chi2 = 1.e10;
        return;
    }
    amplitude = refy / ref2;
    chi2 = 0;

    //for (int iTbPulse = 0; iTbPulse < ndf; iTbPulse++)
    for (int iTbPulse = 1; iTbPulse < ndf; iTbPulse++)
    {
        int tbData = tb0 + iTbPulse;
        double valueData = buffer[tbData];
        double tbRef = iTbPulse - tbOffset + 0.5;
        double valueRefA = amplitude * fPulse -> Eval(tbRef);
        double errorRefA = amplitude * fPulse -> Error(tbRef);
        double residual = (valueData-valueRefA)*(valueData-valueRefA)/errorRefA/errorRefA;
        chi2 += residual;
    }
}

