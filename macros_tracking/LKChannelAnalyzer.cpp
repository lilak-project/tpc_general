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

    auto numPulsePoints = fPulse -> GetNDF();
    fFWHM           = fPulse -> GetFWHM();
    fFloorRatio     = fPulse -> GetFloorRatio();
    fWidth          = fPulse -> GetWidth();
    fWidthLeading   = fPulse -> GetLeadingWidth();
    fWidthTrailing  = fPulse -> GetTrailingWidth();

    fNDFFit = fWidthLeading + fFWHM/4;
    fNumTbsCorrection = int(numPulsePoints);
    fTbStepIfFoundHit = fNDFFit;
    fTbStepIfSaturated = int(fWidth*1.2);
    fTbSeparationWidth = fNDFFit;
    if (fTbStartCut<0)
        fTbStartCut = fTbMax - fNDFFit;
    fNumTbAcendingCut = int(fWidthLeading*2/3);

    e_info << "fFWHM               = " << fFWHM              << endl;
    e_info << "fFloorRatio         = " << fFloorRatio        << endl;
    e_info << "fWidth              = " << fWidth             << endl;
    e_info << "fWidthLeading       = " << fWidthLeading      << endl;
    e_info << "fWidthTrailing      = " << fWidthTrailing     << endl;
    e_info << "fNDFFit             = " << fNDFFit            << endl;
    e_info << "fNumTbsCorrection   = " << fNumTbsCorrection  << endl;
    e_info << "fTbStepIfFoundHit   = " << fTbStepIfFoundHit  << endl;
    e_info << "fTbStepIfSaturated  = " << fTbStepIfSaturated << endl;
    e_info << "fTbSeparationWidth  = " << fTbSeparationWidth << endl;
}

void LKChannelAnalyzer::Clear(Option_t *option)
{
    TObject::Clear(option);
    fNumHits = 0;
    fTbHitArray.clear();
    fAmplitudeArray.clear();
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
    int ndf = fNDFFit;

    // Previous hit information
    double tbHitPrev = fTbStart;
    double amplitudePrev = 0;

    fTbHitArray.clear();
    fAmplitudeArray.clear();

    while (FindPeak(fBuffer, tbPointer, tbStartOfPulse))
    {
#ifdef DEBUG_CHANA_ANALYZE
        lk_debug << tbPointer << " " << tbStartOfPulse << endl;
#endif
        if (tbStartOfPulse > fTbStartCut-1)
            break;

        bool isSaturated = false;
        if (FitPulse(fBuffer, tbStartOfPulse, tbPointer, tbHit, amplitude, squareSum, ndf, isSaturated) == false)
            continue;

        // Pulse is found!

        if (TestPulse(fBuffer, tbHitPrev, amplitudePrev, tbHit, amplitude)) 
        {
            fTbHitArray.push_back(tbHit);
            fAmplitudeArray.push_back(amplitude);

            tbHitPrev = tbHit;
            amplitudePrev = amplitude;
            if (isSaturated)
                tbPointer = int(tbHit) + fTbStepIfSaturated;
            else
                tbPointer = int(tbHit) + fTbStepIfFoundHit;
        }
    }

    fNumHits = fTbHitArray.size();
}

bool LKChannelAnalyzer::FindPeak(double *buffer, int &tbPointer, int &tbStartOfPulse)
{
    int countAscending      = 0;
    //int countAscendingBelowThreshold = 0;

    for (; tbPointer<fTbMax; tbPointer++)
    {
        double value = buffer[tbPointer];
        double yDiff = value - buffer[tbPointer-1];

        // If buffer difference of step is above threshold
        if (yDiff > fOneStepThreshold)
        {
            if (value > fThreshold) countAscending++;
            //else countAscendingBelowThreshold++;
        }
        else 
        {
            // If acended step is below 5, 
            // or negative pulse is bigger than the found pulse, continue
            //if (countAscending < fNumTbAcendingCut || ((countAscendingBelowThreshold >= countAscending) && (-buffer[tbPointer - 1 - countAscending - countAscendingBelowThreshold] > buffer[tbPointer - 1])))
            if (countAscending < fNumTbAcendingCut)
            {
#ifdef DEBUG_FINDPEAK
                if (countAscending>2)
                    lk_debug << "skip candidate num ascending cut : " << countAscending << " < " << fNumTbAcendingCut << endl;
#endif
                countAscending = 0;
                //countAscendingBelowThreshold = 0;
                continue;
            }

            tbPointer -= 1;
            if (value < fThreshold) {
#ifdef DEBUG_FINDPEAK
                lk_debug << "skip candidate from threshold cut : " << value << " < " << fThreshold << endl;
#endif
                continue;
            }

            // Peak is found!
            tbStartOfPulse = tbPointer - countAscending;
            while (buffer[tbStartOfPulse] < value * fFloorRatio)
                tbStartOfPulse++;

#ifdef DEBUG_FINDPEAK
            lk_debug << "found peak : " << tbStartOfPulse << " = " << tbPointer << " - " << countAscending << " + alpha" << endl;
#endif

            return true;
        }
    }

    return false;
}

bool LKChannelAnalyzer::FitPulse(double *buffer, int tbStartOfPulse, int tbPeak,
        double &tbHit, 
        double &amplitude,
        double &chi2Fitted,
        int    &ndf,
        bool   &isSaturated)
{
#ifdef DEBUG_FITPULSE
    lk_debug << "tb0=" << tbStartOfPulse << ",  tbPeak=" << tbPeak << ",  ndf=" << ndf << endl;
#endif
    double valuePeak = buffer[tbPeak];
    isSaturated = false;

    if (ndf > fNDFFit)
        ndf = fNDFFit;

    // if peak value is larger than fDynamicRange, the pulse is isSaturated
    if (valuePeak > fDynamicRange) {
        ndf = tbPeak - tbStartOfPulse;
        if (ndf > fNDFFit)
            ndf = fNDFFit;
        isSaturated = true;
    }

    double stepChi2NDFCut = 1;

    double chi2NDFPrev = DBL_MAX; // Least-squares of previous fit
    double chi2NDFCurr = DBL_MAX; // Least-squares of current fit
    double tbStep = 0.1; // Time-bucket step to next fit
    double tbCurr = tbStartOfPulse; // Pulse starting time-bucket of current fit
    double tbPrev = tbCurr - tbStep; // Pulse starting time-bucket of previous fit

    LeastSquareFitAtGivenTb(buffer, tbPrev, ndf, amplitude, chi2NDFPrev);
    LeastSquareFitAtGivenTb(buffer, tbCurr, ndf, amplitude, chi2NDFCurr);
    double beta = -(chi2NDFCurr-chi2NDFPrev) / (tbCurr-tbPrev); // this value is in order of 1

    int countIteration = 0;
    bool firstCheckFlag = false; // Checking flag to apply cut twice in a row
#ifdef DEBUG_FITPULSE
    if (dGraphTb==nullptr)        { dGraphTb = new TGraph();         dGraphTb        -> SetTitle(";iteration;tb"); }
    if (dGraphTbStep==nullptr)    { dGraphTbStep = new TGraph();     dGraphTbStep    -> SetTitle(";iteration;tb step"); }
    if (dGraphChi2==nullptr)      { dGraphChi2 = new TGraph();       dGraphChi2      -> SetTitle(";iteration;#Chi^{2}/NDF"); }
    if (dGraphTbChi2==nullptr)    { dGraphTbChi2 = new TGraph();     dGraphTbChi2    -> SetTitle(";tb;#Chi^{2}/NDF"); }
    if (dGraphBeta==nullptr)      { dGraphBeta = new TGraph();       dGraphBeta      -> SetTitle(";iteration;#beta = d(#Chi^{2}/NDF)/dtb"); }
    if (dGraphTbBeta==nullptr)    { dGraphTbBeta = new TGraph();     dGraphTbBeta    -> SetTitle(";tb;#beta = d(#Chi^{2}/NDF)/dtb"); }
    if (dGraphBetaInv==nullptr)   { dGraphBetaInv = new TGraph();    dGraphBetaInv   -> SetTitle(";iteration;1/#beta = dtb/d(#Chi^{2}/NDF)"); }
    if (dGraphTbBetaInv==nullptr) { dGraphTbBetaInv = new TGraph();  dGraphTbBetaInv -> SetTitle(";tb;1/#beta = dtb/d(#Chi^{2}/NDF)"); }
    for (auto graph :  {dGraphTb, dGraphTbStep, dGraphChi2, dGraphTbChi2, dGraphBeta, dGraphTbBeta, dGraphBetaInv, dGraphTbBetaInv}) {
        graph -> SetMarkerStyle(24);
        //graph -> SetMarkerSize(0.8);
        graph -> SetLineColor(kBlue);
    }
    dGraphTb -> Set(0);
    dGraphTbStep -> Set(0);
    dGraphChi2 -> Set(0);
    dGraphTbChi2 -> Set(0);
    dGraphBeta -> Set(0);
    dGraphTbBeta -> Set(0);
    dGraphBetaInv -> Set(0);
    dGraphTbBetaInv -> Set(0);
    dGraphTb -> SetPoint(countIteration, countIteration, tbCurr);
    dGraphTbStep -> SetPoint(countIteration, countIteration, tbStep);
    dGraphChi2 -> SetPoint(countIteration, countIteration, chi2NDFCurr);
    dGraphBeta -> SetPoint(countIteration, countIteration, double(beta));
    dGraphBetaInv -> SetPoint(countIteration, countIteration, double(1./beta));
    dGraphTbChi2 -> SetPoint(countIteration, tbCurr, chi2NDFCurr);
    dGraphTbBeta -> SetPoint(countIteration, tbCurr, double(beta));
    dGraphTbBetaInv -> SetPoint(countIteration, tbCurr, double(1./beta));
#endif

    while (true)
    {
        countIteration++;
        tbPrev = tbCurr;
        chi2NDFPrev = chi2NDFCurr;
        tbStep = fScaleTbStep * beta;
        if (tbStep>1) tbStep = 1;
        else if (tbStep<-1) tbStep = -1;
        tbCurr = tbPrev + tbStep;

        if (tbCurr<0 || tbCurr>fTbStartCut) {
#ifdef DEBUG_FITPULSE
            lk_debug << "break(" << countIteration << ") tbCur<0 || tbCur<fTbStartCut : " << tbCurr << " " << fTbStartCut << endl;
#endif
            return false;
        }

        LeastSquareFitAtGivenTb(buffer, tbCurr, ndf, amplitude, chi2NDFCurr);
        beta = -(chi2NDFCurr-chi2NDFPrev) / (tbCurr-tbPrev); // this value is in order of 1
#ifdef DEBUG_FITPULSE
        dGraphTb -> SetPoint(countIteration, countIteration, tbCurr);
        dGraphTbStep -> SetPoint(countIteration, countIteration, tbStep);
        dGraphChi2 -> SetPoint(countIteration, countIteration, chi2NDFCurr);
        dGraphBeta -> SetPoint(countIteration, countIteration, double(beta));
        dGraphBetaInv -> SetPoint(countIteration, countIteration, double(1./beta));
        dGraphTbChi2 -> SetPoint(countIteration, tbCurr, chi2NDFCurr);
        dGraphTbBeta -> SetPoint(countIteration, tbCurr, double(beta));
        dGraphTbBetaInv -> SetPoint(countIteration, tbCurr, double(1./beta));
        lk_debug << "IT-" << countIteration << " tb: " << tbPrev << "->" << tbCurr << "(" << tbStep << ")" << ",  chi2: " << chi2NDFPrev << "->" << chi2NDFCurr << ",  tb-step: " << tbStep << endl;
#endif

        if (abs(tbStep)<fTbStepCut) {
#ifdef DEBUG_FITPULSE
            lk_debug << "break(" << countIteration << ") tbStep < " << fTbStepCut << " : " << tbStep << endl;
#endif
            break;
        }

        if (countIteration >= fIterMax) {
#ifdef DEBUG_FITPULSE
            lk_debug << "break(" << countIteration << ") : iteration cut exit: " << countIteration << " < " << fIterMax << endl;
#endif
            break;
        }
    }

    if (beta > 0) { // pre-fit is better
        tbHit = tbPrev;
        chi2Fitted = chi2NDFPrev;
    }
    else { // current-fit is better
        tbHit = tbCurr;
        chi2Fitted = chi2NDFCurr;
    }

    return true;
}

bool LKChannelAnalyzer::TestPulse(double *buffer, double tbHitPrev, double amplitudePrev, double tbHit, double amplitude)
{
    int numTbsCorrection = fNumTbsCorrection;
    if (numTbsCorrection + int(tbHit) >= fTbMax)
        numTbsCorrection = fTbMax - int(tbHit);

    if (amplitude < fThreshold) 
        return false;

    /*
    double amplitudeCut = 0.5 * fPulse -> Eval(tbHit+fTbSeparationWidth, tbHitPrev, amplitudePrev);
    if (amplitude < amplitudeCut)
    {
        for (int iTbPulse = -1; iTbPulse < numTbsCorrection; iTbPulse++) {
            int tb = int(tbHit) + iTbPulse;
            buffer[tb] -= fPulse -> Eval(tb, tbHit, amplitude);
        }
        return false;
    }
    */

    for (int iTbPulse = -1; iTbPulse < numTbsCorrection; iTbPulse++) {
        int tb = int(tbHit) + iTbPulse;
        buffer[tb] -= fPulse -> Eval(tb, tbHit, amplitude);
    }

    return true;
}

void LKChannelAnalyzer::LeastSquareFitAtGivenTb(double *buffer, double tbStartOfPulse, int ndf,
        double &amplitude,
        double &chi2NDF)
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
        chi2NDF = 1.e10;
        return;
    }
    amplitude = refy / ref2;
    chi2NDF = 0;

    //for (int iTbPulse = 0; iTbPulse < ndf; iTbPulse++)
    for (int iTbPulse = 1; iTbPulse < ndf; iTbPulse++)
    {
        int tbData = tb0 + iTbPulse;
        double valueData = buffer[tbData];
        double tbRef = iTbPulse - tbOffset + 0.5;
        double valueRefA = amplitude * fPulse -> Eval(tbRef);
        double errorRefA = amplitude * fPulse -> Error(tbRef);
        double residual = (valueData-valueRefA)*(valueData-valueRefA)/errorRefA/errorRefA;
        chi2NDF += residual;
    }
    chi2NDF = chi2NDF/ndf;
}

