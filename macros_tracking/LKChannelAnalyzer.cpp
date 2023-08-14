#include "LKChannelAnalyzer.h"

ClassImp(LKChannelAnalyzer);

LKChannelAnalyzer::LKChannelAnalyzer()
{
}

bool LKChannelAnalyzer::Init()
{
    e_info << "Initializing LKChannelAnalyzer" << std::endl;

    fPulse = new LKPulse("data/pulseReference_MMCenter.root");

    return true;
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
    double tbHitPre = fTbStart;
    double amplitudePre = 0;

    while (FindPeak(fBuffer, tbPointer, tbStartOfPulse))
    {
        if (tbStartOfPulse > fTbStartCut-1)
            break;

        bool isSaturated = false;
        if (FitPulse(fBuffer, tbStartOfPulse, tbPointer, tbHit, amplitude, squareSum, ndf, isSaturated) == false)
            continue;

        // Pulse is found!

        if (TestPulse(fBuffer, tbHitPre, amplitudePre, tbHit, amplitude)) 
        {
            //auto channelHit = LKChannelHit(tbHit,amplitude);
            //fChannelHitArray.push_back(channelHit);

            tbHitPre = tbHit;
            amplitudePre = amplitude;
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
        double &squareSum,
        int    &ndf,
        bool   &isSaturated)
{ return true; }
/*
{
    double valuePeak = buffer[tbPeak];
    isSaturated = false;

    // if peak value is larger than fDynamicRange, the pulse is isSaturated
    if (valuePeak > fDynamicRange)
    {
        ndf = tbPeak - tbStartOfPulse;
        //if (ndf > fNDFPulse) ndf = fNDFPulse;
        isSaturated = true;
    }

    double alpha   = fAlpha   / (valuePeak * valuePeak); // Weight of time-bucket step
    double betaCut = fBetaCut * (valuePeak * valuePeak); // Effective cut for beta

    double lsPre; // Least-squares of previous fit
    double lsCur; // Least-squares of current fit

    double beta = 0;    // -(lsCur-lsPre)/(tbCur-tbPre)/ndf.
    double dTb = - 0.1; // Time-bucket step to next fit

    double tbPre = tbStartOfPulse + 1; // Pulse starting time-bucket of previous fit
    double tbCur = tbPre + dTb; // Pulse starting time-bucket of current fit

    LeastSquareFitAtGivenTb(buffer, tbPre, ndf, lsPre, amplitude);
    LeastSquareFitAtGivenTb(buffer, tbCur, ndf, lsCur, amplitude);
    beta = -(lsCur - lsPre) / (tbCur - tbPre) / ndf;

    int numIteration = 1;
    bool doubleCheckFlag = false; // Checking flag to apply cut twice in a row

    while (dTb!=0 && lsCur!=lsPre)
    {
        lsPre = lsCur;
        tbPre = tbCur;
        dTb = alpha * beta;
        if (dTb> 1) dTb =  1;
        if (dTb<-1) dTb = -1;
        tbCur = tbPre + dTb;
        if (tbCur<0 || tbCur>fTbStartCut)
            return false;

        LeastSquareFitAtGivenTb(buffer, tbCur, ndf, lsCur, amplitude);
        beta = -(lsCur - lsPre) / (tbCur - tbPre) / ndf;

        numIteration++;
        if (abs(beta) < betaCut) {
            // break at second true flag of doubleCheckFlag
            // >> break if the fit is good enough at two times check
            if (doubleCheckFlag == true)
                break;
            else // first true flag of doubleCheckFlag
                doubleCheckFlag = true;
        }
        else
            doubleCheckFlag = false;

        if (numIteration >= fIterMax)
            break;
    }

    if (beta > 0) { // pre-fit is better
        tbHit = tbPre;
        squareSum = lsPre;
    }
    else { // current-fit is better
        tbHit = tbCur;
        squareSum = lsCur;
    }

    return true;
}
*/

bool LKChannelAnalyzer::TestPulse(double *buffer, double tbHitPre, double amplitudePre, double tbHit, double amplitude)
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

    if (amplitude < fPulseGenerator -> Pulse(tbHit + 9, amplitudePre, tbHitPre) / 2.5) 
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
    for (int iTbPulse = 0; iTbPulse < ndf; iTbPulse++) {
        int tb = tbStartOfPulse + iTbPulse;
        double y = buffer[tb];
        double ref = fPulse -> Eval(iTbPulse);
        refy += ref * y;
        ref2 += ref * ref;
    }
    if (ref2==0) {
        chi2 = 1.e10;
        return;
    }
    amplitude = refy / ref2;
    chi2 = 0;
    for (int iTbPulse = 0; iTbPulse < ndf; iTbPulse++) {
        int tb = tbStartOfPulse + iTbPulse;
        double val = buffer[tb];
        double ref = fPulse -> Eval(iTbPulse) * amplitude;
        chi2 += (val-ref) * (val-ref);
        //cout << val << " " << ref << " " << (val-ref) * (val-ref) << " " << chi2 << endl;
    }
    //cout << chi2 << endl;
}

