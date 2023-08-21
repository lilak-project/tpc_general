#include "LKChannelAnalyzer.h"

ClassImp(LKChannelAnalyzer);

LKChannelAnalyzer::LKChannelAnalyzer()
{
}

bool LKChannelAnalyzer::Init()
{
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
}

void LKChannelAnalyzer::Clear(Option_t *option)
{
    TObject::Clear(option);
    fNumHits = 0;
    //fTbHitArray.clear();
    //fAmplitudeArray.clear();
    fFitParameterArray.clear();
}

void LKChannelAnalyzer::Print(Option_t *option) const
{
    e_info << "== General" << endl;
    e_info << "   fTbMax            =" << fTbMax               << endl;
    e_info << "   fTbStart          =" << fTbStart             << endl;
    e_info << "   fTbStartCut       =" << fTbStartCut          << endl;
    e_info << "   fNumTbAcendingCut =" << fNumTbAcendingCut    << endl;
    e_info << "   fDynamicRange     =" << fDynamicRange        << endl;
    e_info << "== Pulse information" << endl;
    e_info << "   fFWHM             =" << fFWHM                << endl;
    e_info << "   fFloorRatio       =" << fFloorRatio          << endl;
    e_info << "   fWidth            =" << fWidth               << endl;
    e_info << "   fWidthLeading     =" << fWidthLeading        << endl;
    e_info << "   fWidthTrailing    =" << fWidthTrailing       << endl;
    e_info << "== Peak Finding" << endl;
    e_info << "   fThreshold        =" << fThreshold           << endl;
    e_info << "   fThresholdOneStep =" << fThresholdOneStep    << endl;
    e_info << "   fTbStepIfFoundHit =" << fTbStepIfFoundHit    << endl;
    e_info << "   fTbStepIfSaturated=" << fTbStepIfSaturated   << endl;
    e_info << "   fTbSeparationWidth=" << fTbSeparationWidth   << endl;
    e_info << "   fNumTbsCorrection =" << fNumTbsCorrection    << endl;
    e_info << "== Pulse Fitting" << endl;
    e_info << "   fNDFFit           =" << fNDFFit              << endl;
    e_info << "   fIterMax          =" << fIterMax             << endl;
    e_info << "   fTbStepCut        =" << fTbStepCut           << endl;
    e_info << "   fScaleTbStep      =" << fScaleTbStep         << endl;
    e_info << "== Number of found hits: " << fNumHits << endl;
    if (fNumHits>0)
        for (auto iHit=0; iHit<fNumHits; ++iHit) {
            auto tbHit = GetTbHit(iHit);
            auto amplitude = GetAmplitude(iHit);
            e_info << "   Hit-" << iHit << ": tb=" << tbHit << " amplitude=" << amplitude << endl;
        }
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
    double chi2NDF;
    int ndf = fNDFFit;

    // Previous hit information
    double tbHitPrev = fTbStart;
    double amplitudePrev = 0;

    fNumHits = 0;
    //fTbHitArray.clear();
    //fAmplitudeArray.clear();

    while (FindPeak(fBuffer, tbPointer, tbStartOfPulse))
    {
#ifdef DEBUG_CHANA_ANALYZE
        lk_debug << tbPointer << " " << tbStartOfPulse << endl;
#endif
        if (tbStartOfPulse > fTbStartCut-1)
            break;

        bool isSaturated = false;
        if (FitPulse(fBuffer, tbStartOfPulse, tbPointer, tbHit, amplitude, chi2NDF, ndf, isSaturated) == false)
            continue;

        // Pulse is found!

        if (TestPulse(fBuffer, tbHitPrev, amplitudePrev, tbHit, amplitude)) 
        {
            //fTbHitArray.push_back(tbHit);
            //fAmplitudeArray.push_back(amplitude);
            fFitParameterArray.push_back(LKPulseFitParameter(tbHit,amplitude,chi2NDF,ndf));

            tbHitPrev = tbHit;
            amplitudePrev = amplitude;
            if (isSaturated)
                tbPointer = int(tbHit) + fTbStepIfSaturated;
            else
                tbPointer = int(tbHit) + fTbStepIfFoundHit;
        }
    }

    //fNumHits = fTbHitArray.size();
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
        if (yDiff > fThresholdOneStep)
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
#ifdef DEBUG_CHANA_FINDPEAK
                if (countAscending>2)
                    lk_debug << "skip candidate num ascending cut : " << countAscending << " < " << fNumTbAcendingCut << endl;
#endif
                countAscending = 0;
                //countAscendingBelowThreshold = 0;
                continue;
            }

            tbPointer -= 1;
            if (value < fThreshold) {
#ifdef DEBUG_CHANA_FINDPEAK
                lk_debug << "skip candidate from threshold cut : " << value << " < " << fThreshold << endl;
#endif
                continue;
            }

            // Peak is found!
            tbStartOfPulse = tbPointer - countAscending;
            while (buffer[tbStartOfPulse] < value * fFloorRatio)
                tbStartOfPulse++;

#ifdef DEBUG_CHANA_FINDPEAK
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
#ifdef DEBUG_CHANA_FITPULSE
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

    FitAmplitude(buffer, tbPrev, ndf, amplitude, chi2NDFPrev);
    FitAmplitude(buffer, tbCurr, ndf, amplitude, chi2NDFCurr);
    double slope = -(chi2NDFCurr-chi2NDFPrev) / (tbCurr-tbPrev);

    int countIteration = 0;
    bool firstCheckFlag = false; // Checking flag to apply cut twice in a row
#ifdef DEBUG_CHANA_FITPULSE
    if (dGraph_it_tb==nullptr)      { dGraph_it_tb = new TGraph();      dGraph_it_tb      -> SetTitle(";iteration;tb"); }
    if (dGraph_it_tbStep==nullptr)  { dGraph_it_tbStep = new TGraph();  dGraph_it_tbStep  -> SetTitle(";iteration;tb step"); }
    if (dGraph_it_chi2==nullptr)    { dGraph_it_chi2 = new TGraph();    dGraph_it_chi2    -> SetTitle(";iteration;#Chi^{2}/NDF"); }
    if (dGraph_tb_chi2==nullptr)    { dGraph_tb_chi2 = new TGraph();    dGraph_tb_chi2    -> SetTitle(";tb;#Chi^{2}/NDF"); }
    if (dGraph_it_slope==nullptr)    { dGraph_it_slope = new TGraph();    dGraph_it_slope    -> SetTitle(";iteration;#slope = d(#Chi^{2}/NDF)/dtb"); }
    if (dGraph_tb_slope==nullptr)    { dGraph_tb_slope = new TGraph();    dGraph_tb_slope    -> SetTitle(";tb;#slope = d(#Chi^{2}/NDF)/dtb"); }
    if (dGraph_it_slopeInv==nullptr) { dGraph_it_slopeInv = new TGraph(); dGraph_it_slopeInv -> SetTitle(";iteration;1/#slope = dtb/d(#Chi^{2}/NDF)"); }
    if (dGraph_tb_slopeInv==nullptr) { dGraph_tb_slopeInv = new TGraph(); dGraph_tb_slopeInv -> SetTitle(";tb;1/#slope = dtb/d(#Chi^{2}/NDF)"); }
    for (auto graph : {
            dGraph_it_tb,
            dGraph_it_tbStep,
            dGraph_it_chi2,
            dGraph_tb_chi2,
            dGraph_it_slope,
            dGraph_tb_slope,
            dGraph_it_slopeInv,
            dGraph_tb_slopeInv}
        )
    {
        graph -> Set(0);
        graph -> SetMarkerStyle(24);
        graph -> SetLineColor(kBlue);
    }
    dGraph_it_tb -> SetPoint(countIteration, countIteration, tbCurr);
    dGraph_it_tbStep -> SetPoint(countIteration, countIteration, tbStep);
    dGraph_it_chi2 -> SetPoint(countIteration, countIteration, chi2NDFCurr);
    dGraph_it_slope -> SetPoint(countIteration, countIteration, double(slope));
    dGraph_it_slopeInv -> SetPoint(countIteration, countIteration, double(1./slope));
    dGraph_tb_chi2 -> SetPoint(countIteration, tbCurr, chi2NDFCurr);
    dGraph_tb_slope -> SetPoint(countIteration, tbCurr, double(slope));
    dGraph_tb_slopeInv -> SetPoint(countIteration, tbCurr, double(1./slope));
#endif

    while (true)
    {
        countIteration++;
        tbPrev = tbCurr;
        chi2NDFPrev = chi2NDFCurr;
        tbStep = fScaleTbStep * slope;
        if (tbStep>1) tbStep = 1;
        else if (tbStep<-1) tbStep = -1;
        tbCurr = tbPrev + tbStep;

        if (tbCurr<0 || tbCurr>fTbStartCut) {
#ifdef DEBUG_CHANA_FITPULSE
            lk_debug << "break(" << countIteration << ") tbCur<0 || tbCur<fTbStartCut : " << tbCurr << " " << fTbStartCut << endl;
#endif
            return false;
        }

        FitAmplitude(buffer, tbCurr, ndf, amplitude, chi2NDFCurr);
        slope = -(chi2NDFCurr-chi2NDFPrev) / (tbCurr-tbPrev);
#ifdef DEBUG_CHANA_FITPULSE
        dGraph_it_tb -> SetPoint(countIteration, countIteration, tbCurr);
        dGraph_it_tbStep -> SetPoint(countIteration, countIteration, tbStep);
        dGraph_it_chi2 -> SetPoint(countIteration, countIteration, chi2NDFCurr);
        dGraph_it_slope -> SetPoint(countIteration, countIteration, double(slope));
        dGraph_it_slopeInv -> SetPoint(countIteration, countIteration, double(1./slope));
        dGraph_tb_chi2 -> SetPoint(countIteration, tbCurr, chi2NDFCurr);
        dGraph_tb_slope -> SetPoint(countIteration, tbCurr, double(slope));
        dGraph_tb_slopeInv -> SetPoint(countIteration, tbCurr, double(1./slope));
        lk_debug << "IT-" << countIteration << " tb: " << tbPrev << "->" << tbCurr << "(" << tbStep << ")" << ",  chi2: " << chi2NDFPrev << "->" << chi2NDFCurr << ",  tb-step: " << tbStep << endl;
#endif

        if (abs(tbStep)<fTbStepCut) {
#ifdef DEBUG_CHANA_FITPULSE
            lk_debug << "break(" << countIteration << ") tbStep < " << fTbStepCut << " : " << tbStep << endl;
#endif
            break;
        }

        if (countIteration >= fIterMax) {
#ifdef DEBUG_CHANA_FITPULSE
            lk_debug << "break(" << countIteration << ") : iteration cut exit: " << countIteration << " < " << fIterMax << endl;
#endif
            break;
        }
    }

    if (slope > 0) { // pre-fit is better
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

void LKChannelAnalyzer::FitAmplitude(double *buffer, double tbStartOfPulse, int ndf,
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

