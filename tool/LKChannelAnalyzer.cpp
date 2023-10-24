#include "LKChannelAnalyzer.h"
#include "TLine.h"

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
    fPulseFileName = fileName;
    fPulse = new LKPulse(fPulseFileName);

    auto numPulsePoints = fPulse -> GetNDF();
    fFWHM           = fPulse -> GetFWHM();
    fFloorRatio     = fPulse -> GetFloorRatio();
    fWidth          = fPulse -> GetWidth();
    fWidthLeading   = fPulse -> GetLeadingWidth();
    fWidthTrailing  = fPulse -> GetTrailingWidth();
    fPulseRefTbMin  = fPulse -> GetPulseRefTbMin();
    fPulseRefTbMax  = fPulse -> GetPulseRefTbMax();
    fNDFPulse       = fWidthLeading + fWidthTrailing;
    //fNDFPulse       = fPulse -> GetNDF();

    fNDFFit = fWidthLeading + fFWHM/4;
    //fNumTbsCorrection = int(numPulsePoints);
    fTbStepIfFoundHit = fNDFFit;
    fTbStepIfSaturated = int(fWidth*1.2);
    fTbSeparationWidth = fNDFFit;
    if (fTbStartCut<0)
        fTbStartCut = fTbMax - fNDFFit;
    fNumTbAcendingCut = int(fWidthLeading*2/3);
}

void LKChannelAnalyzer::Clear(Option_t *option)
{
    fPedestal = 0;
    fDynamicRange = fDynamicRangeOriginal;
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
    e_info << "   fPulseRefTbMin    =" << fPulseRefTbMin       << endl;
    e_info << "   fPulseRefTbMax    =" << fPulseRefTbMax       << endl;
    e_info << "== Peak Finding" << endl;
    e_info << "   fThreshold        =" << fThreshold           << endl;
    e_info << "   fThresholdOneStep =" << fThresholdOneStep    << endl;
    e_info << "   fTbStepIfFoundHit =" << fTbStepIfFoundHit    << endl;
    e_info << "   fTbStepIfSaturated=" << fTbStepIfSaturated   << endl;
    e_info << "   fTbSeparationWidth=" << fTbSeparationWidth   << endl;
    //e_info << "   fNumTbsCorrection =" << fNumTbsCorrection    << endl;
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

void LKChannelAnalyzer::Draw(Option_t *option)
{
    if (fHistBuffer==nullptr)
        fHistBuffer = new TH1D("hist_chana_buffer",";tb",fTbMax,0,fTbMax);

    for (auto tb=0; tb<fTbMax; ++tb)
        fHistBuffer -> SetBinContent(tb+1,fBufferOrigin[tb]);

    fHistBuffer -> Draw();
    for (auto iHit=0; iHit<fNumHits; ++iHit) {
        auto tbHit = GetTbHit(iHit);
        auto amplitude = GetAmplitude(iHit);
        auto graph = fPulse -> GetPulseGraph(tbHit,amplitude,fPedestal);
        graph -> Draw("samelx");
    }

    int numTbPart = fTbMax/NUMBER_OF_PEDESTAL_TEST_REGIONS;
    auto ymax = fHistBuffer -> GetMaximum();
    for (auto iPart=0; iPart<NUMBER_OF_PEDESTAL_TEST_REGIONS; ++iPart) {
        auto x = numTbPart*iPart;
        auto line = new TLine(x,0,x,ymax);
        line -> SetLineColor(kBlue);
        line -> SetLineStyle(2);
        line -> Draw("samel");
    }
}

void LKChannelAnalyzer::Analyze(int* data)
{
    double buffer[512];
    for (auto tb=0; tb<512; ++tb)
        buffer[tb] = (double)data[tb];
    Analyze(buffer);
}

void LKChannelAnalyzer::Analyze(double* data)
{
    Clear();
    //fNumHits = 0;
    //fTbHitArray.clear();
    //fAmplitudeArray.clear();

    memcpy(&fBufferOrigin, data, sizeof(double)*fTbMax);
    FindAndSubtractPedestal(data);
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

    for (auto it : {0,1})
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
#ifdef DEBUG_CHANA_ANALYZE_NHIT
                if (fFitParameterArray.size()>=DEBUG_CHANA_ANALYZE_NHIT)
                    break;
#endif
                tbHitPrev = tbHit;
                amplitudePrev = amplitude;
                if (isSaturated)
                    tbPointer = int(tbHit) + fTbStepIfSaturated;
                else
                    tbPointer = int(tbHit) + fTbStepIfFoundHit;
            }
        }

    fNumHits = fFitParameterArray.size();
    //fNumHits = fTbHitArray.size();
}

double LKChannelAnalyzer::FindAndSubtractPedestal(double *buffer)
{
    int numTbPart = fTbMax/NUMBER_OF_PEDESTAL_TEST_REGIONS;
    int numTbPartLast = fTbMax - numTbPart*(NUMBER_OF_PEDESTAL_TEST_REGIONS_M1);
    double pedestalPart[NUMBER_OF_PEDESTAL_TEST_REGIONS] = {0.};
    double stddevPart[NUMBER_OF_PEDESTAL_TEST_REGIONS] = {0.};

    int tbGlobal = 0;
    for (auto iPart=0; iPart<NUMBER_OF_PEDESTAL_TEST_REGIONS_M1; ++iPart) {
        for (int iTb=0; iTb<numTbPart; iTb++) {
            pedestalPart[iPart] += buffer[tbGlobal];
            stddevPart[iPart] += buffer[tbGlobal]*buffer[tbGlobal];
            tbGlobal++;
        }
        pedestalPart[iPart] = pedestalPart[iPart] / numTbPart;
        stddevPart[iPart] = stddevPart[iPart] / numTbPart;
        stddevPart[iPart] = sqrt(stddevPart[iPart] - pedestalPart[iPart]*pedestalPart[iPart]);
    }
    for (int iTb=0; iTb<numTbPartLast; iTb++) {
        pedestalPart[NUMBER_OF_PEDESTAL_TEST_REGIONS_M1] = pedestalPart[NUMBER_OF_PEDESTAL_TEST_REGIONS_M1] + buffer[tbGlobal];
        tbGlobal++;
    }
    pedestalPart[NUMBER_OF_PEDESTAL_TEST_REGIONS_M1] = pedestalPart[NUMBER_OF_PEDESTAL_TEST_REGIONS_M1] / numTbPartLast;
#ifdef DEBUG_CHANA_FINDPED
    for (auto iPart=0; iPart<NUMBER_OF_PEDESTAL_TEST_REGIONS; ++iPart)
        lk_debug << iPart << " " << pedestalPart[iPart] << " " << stddevPart[iPart] << " " << stddevPart[iPart]/pedestalPart[iPart] << endl;
#endif

    int countBelowCut = 0;
    for (auto iPart=0; iPart<NUMBER_OF_PEDESTAL_TEST_REGIONS; ++iPart) {
        if (stddevPart[iPart]/pedestalPart[iPart]<0.1)
            countBelowCut++;
    }

    double pedestalDiffMin = DBL_MAX;
    double pedestalMeanRef = 0;
    int idx1 = 0;
    int idx2 = 0;
    if (countBelowCut>=2) {
        for (auto iPart=0; iPart<NUMBER_OF_PEDESTAL_TEST_REGIONS; ++iPart) {
            if (stddevPart[iPart]/pedestalPart[iPart]>=0.1) continue;
            for (auto jPart=0; jPart<NUMBER_OF_PEDESTAL_TEST_REGIONS; ++jPart) {
                if (iPart>=jPart) continue;
                if (stddevPart[jPart]/pedestalPart[jPart]>=0.1) continue;
                double diff = abs(pedestalPart[iPart] - pedestalPart[jPart]);
                if (diff<pedestalDiffMin) {
                    pedestalDiffMin = diff;
                    pedestalMeanRef = 0.5 * (pedestalPart[iPart] + pedestalPart[jPart]);
                    idx1 = iPart;
                    idx2 = jPart;
                }
            }
        }
    }
    else {
        for (auto iPart=0; iPart<NUMBER_OF_PEDESTAL_TEST_REGIONS; ++iPart) {
            for (auto jPart=0; jPart<NUMBER_OF_PEDESTAL_TEST_REGIONS; ++jPart) {
                if (iPart>=jPart) continue;
                double diff = abs(pedestalPart[iPart] - pedestalPart[jPart]);
                if (diff<pedestalDiffMin) {
                    pedestalDiffMin = diff;
                    pedestalMeanRef = 0.5 * (pedestalPart[iPart] + pedestalPart[jPart]);
                    idx1 = iPart;
                    idx2 = jPart;
                }
            }
        }
    }

    //double pedestalErrorRef  = 0.1 * pedestalMeanRef;
    //double pedestalErrorRefPart = 0.2 * pedestalMeanRef;
    double pedestalErrorRefPart = 0.1 * pedestalMeanRef;
    pedestalErrorRefPart = sqrt(pedestalErrorRefPart*pedestalErrorRefPart + pedestalDiffMin*pedestalDiffMin);

#ifdef DEBUG_CHANA_FINDPED
    lk_debug << "diff min : " << pedestalDiffMin << endl;
    lk_debug << "ref-diff : " << pedestalMeanRef << endl;
    //lk_debug << "ref-error: " << pedestalErrorRef << endl;
    lk_debug << "ref-error-part: " << pedestalErrorRefPart << endl;
#endif

    double pedestalFinal = 0;
    int countNumPedestalTb = 0;

    tbGlobal = 0;
    for (auto iPart=0; iPart<NUMBER_OF_PEDESTAL_TEST_REGIONS_M1; ++iPart)
    {
        double diffPart = abs(pedestalMeanRef - pedestalPart[iPart]);
        if (diffPart<pedestalErrorRefPart)
        {
            countNumPedestalTb += numTbPart;
            for (int iTb=0; iTb<numTbPart; iTb++)
            {
                pedestalFinal += buffer[tbGlobal];
                tbGlobal++;
            }
#ifdef DEBUG_CHANA_FINDPED
            lk_debug << iPart << " diff=" << diffPart << " " << pedestalFinal/countNumPedestalTb << " " << countNumPedestalTb << endl;
#endif
        }
        else
            tbGlobal += numTbPart;
    }
    double diffPart = abs(pedestalMeanRef - pedestalPart[NUMBER_OF_PEDESTAL_TEST_REGIONS_M1]);
    if (diffPart<pedestalErrorRefPart)
    {
        countNumPedestalTb += numTbPartLast;
        for (int iTb=0; iTb<numTbPartLast; iTb++) {
            pedestalFinal += buffer[tbGlobal];
            tbGlobal++;
        }
#ifdef DEBUG_CHANA_FINDPED
        lk_debug << NUMBER_OF_PEDESTAL_TEST_REGIONS_M1 << " diff=" << diffPart << " " << pedestalFinal/countNumPedestalTb << " " << countNumPedestalTb << endl;
#endif
    }

    pedestalFinal = pedestalFinal / countNumPedestalTb;

#ifdef DEBUG_CHANA_FINDPED
    lk_debug << pedestalFinal << endl;
#endif
    for (auto iTb=0; iTb<fTbMax; ++iTb)
        buffer[iTb] = buffer[iTb] - pedestalFinal;

    fDynamicRange = fDynamicRange - pedestalFinal;

    fPedestal = pedestalFinal;

    return pedestalFinal;
}

bool LKChannelAnalyzer::FindPeak(double *buffer, int &tbPointer, int &tbStartOfPulse)
{
    int countAscending = 0;
    //int countAscendingBelowThreshold = 0;

    if (tbPointer==0)
        tbPointer = 1;

    double valuePrev = buffer[tbPointer-1];

    for (; tbPointer<fTbMax; tbPointer++)
    {
        double value = buffer[tbPointer];
        double yDiff = value - valuePrev;
#ifdef DEBUG_CHANA_FINDPEAK
        lk_debug << "tbPointer=" << tbPointer << ", value=" << value  << endl;
#endif

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
                tbPointer += 1;
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

        valuePrev = value;
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

    //if (ndf > fNDFFit)
        //ndf = fNDFFit;

    // if peak value is larger than fDynamicRange, the pulse is isSaturated
    if (valuePeak >= fDynamicRange) {
        //ndf = tbPeak - tbStartOfPulse;
        //if (ndf > fNDFFit)
        //  ndf = fNDFFit;
        isSaturated = true;
    }

    LKTbIterationParameters par;
    par.SetScaleTbStep(fScaleTbStep);

    double stepChi2NDFCut = 1;

    double chi2NDFPrev = DBL_MAX; // Least-squares of previous fit
    double chi2NDFCurr = DBL_MAX; // Least-squares of current fit
    double tbStep = 0.1; // Time-bucket step to next fit
    double tbCurr = tbStartOfPulse; // Pulse starting time-bucket of current fit
    double tbPrev = tbCurr - tbStep; // Pulse starting time-bucket of previous fit

    ndf = fNDFFit;
    FitAmplitude(buffer, tbPrev, ndf, amplitude, chi2NDFPrev);
    ndf = fNDFFit;
    FitAmplitude(buffer, tbCurr, ndf, amplitude, chi2NDFCurr);

    par.Add(chi2NDFPrev,tbPrev);
    par.Add(chi2NDFCurr,tbCurr);
    double slope = par.Slope();

#ifdef DEBUG_CHANA_FITPULSE
    //lk_debug << "slope is " << slope << " " << chi2NDFCurr << " " << chi2NDFPrev << " " <<  tbCurr << " " << tbPrev << endl;
    double chi2NDF0 = chi2NDFPrev;
#endif

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
    double slope2 = abs(slope);
    dGraph_it_tb -> SetPoint(countIteration, countIteration, tbCurr);
    dGraph_it_tbStep -> SetPoint(countIteration, countIteration, tbStep);
    dGraph_it_chi2 -> SetPoint(countIteration, countIteration, chi2NDFCurr);
    dGraph_it_slope -> SetPoint(countIteration, countIteration, slope2);
    dGraph_it_slopeInv -> SetPoint(countIteration, countIteration, double(1./slope2));
    dGraph_tb_chi2 -> SetPoint(countIteration, tbCurr, chi2NDFCurr);
    dGraph_tb_slope -> SetPoint(countIteration, tbCurr, slope2);
    dGraph_tb_slopeInv -> SetPoint(countIteration, tbCurr, double(1./slope2));
#endif

    bool foundNewMinimum = true;
    while (true)
    {
        countIteration++;
        if (foundNewMinimum) {
            tbPrev = tbCurr;
            chi2NDFPrev = chi2NDFCurr;
        }
        //tbStep = fScaleTbStep * slope;
        //if (tbStep>1) tbStep = 1;
        //else if (tbStep<-1) tbStep = -1;
        //tbCurr = tbPrev + tbStep;

        tbStep = par.TbStep();
        tbCurr = par.NextTb(tbPrev);

        if (tbCurr<0 || tbCurr>fTbStartCut) {
#ifdef DEBUG_CHANA_FITPULSE
            lk_debug << "break(" << countIteration << ") tbCur<0 || tbCur<fTbStartCut : " << tbCurr << " " << fTbStartCut << endl;
#endif
            return false;
        }

        ndf = fNDFFit;
        FitAmplitude(buffer, tbCurr, ndf, amplitude, chi2NDFCurr);
        //slope = -(chi2NDFCurr-chi2NDFPrev) / (tbCurr-tbPrev);
        foundNewMinimum = par.Add(chi2NDFCurr,tbCurr);
        slope = par.Slope();
#ifdef DEBUG_CHANA_FITPULSE
        double slope2 = abs(slope);
        dGraph_it_tb -> SetPoint(countIteration, countIteration, tbCurr);
        dGraph_it_tbStep -> SetPoint(countIteration, countIteration, tbStep);
        dGraph_it_chi2 -> SetPoint(countIteration, countIteration, chi2NDFCurr);
        dGraph_it_slope -> SetPoint(countIteration, countIteration, double(slope2));
        dGraph_it_slopeInv -> SetPoint(countIteration, countIteration, double(1./slope2));
        dGraph_tb_chi2 -> SetPoint(countIteration, tbCurr, chi2NDFCurr);
        dGraph_tb_slope -> SetPoint(countIteration, tbCurr, double(slope2));
        dGraph_tb_slopeInv -> SetPoint(countIteration, tbCurr, double(1./slope2));
        lk_debug << "IT-" << countIteration << " tb: " << tbPrev << "->" << tbCurr << "(" << tbStep << ")" << ",  c2/n: " << chi2NDFPrev << "->" << chi2NDFCurr << ",  tb-step: " << tbStep << " ndf=" << ndf << endl;
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


    tbHit = par.fT1;
    chi2Fitted = par.fC1;

    //if (slope > 0) { // pre-fit is better
    //    tbHit = tbPrev;
    //    chi2Fitted = chi2NDFPrev;
    //}
    //else { // current-fit is better
    //    tbHit = tbCurr;
    //    chi2Fitted = chi2NDFCurr;
    //}

    return true;
}

void LKChannelAnalyzer::FitAmplitude(double *buffer, double tbStartOfPulse,
        int &ndf,
        double &amplitude,
        double &chi2NDF)
{
    if (isnan(tbStartOfPulse)) {
#ifdef DEBUG_CHANA_FITAMPLITUDE
        lk_debug << "return tb is nan" << endl;
#endif
        chi2NDF = 1.e10;
        return;
    }
#ifdef DEBUG_CHANA_FITAMPLITUDE
    lk_debug << "pulse: "; for (auto i=0; i<20; ++i) e_cout << i << "/" << fPulse -> Eval(i) << ", "; e_cout << endl;
#endif
    double refy = 0;
    double ref2 = 0;
    int tb0 = int(tbStartOfPulse); 
#ifdef DEBUG_CHANA_FITAMPLITUDE
    lk_debug << tb0 << " = abs(" << tbStartOfPulse << ")" << endl;
#endif
    double tbOffset = tbStartOfPulse - tb0; 

    int ndfFit = 0;
    int iTbPulse = 0;
    for (; (ndfFit<ndf && iTbPulse<fNDFPulse); iTbPulse++)
    {
        int tbData = tb0 + iTbPulse;
#ifdef DEBUG_CHANA_FITAMPLITUDE
        lk_debug << tbData << " = " << tb0 << " " << iTbPulse << endl;
#endif
        //if (tbData<0 || tbData>=350)
        //    lk_debug << tbData << endl;
        if (tbData>=fTbMax)
            break;
        double valueData = buffer[tbData];
        if (valueData>=fDynamicRange)
            continue;
        ndfFit++;
        double tbRef = iTbPulse - tbOffset + 0.5;
        double valueRef = fPulse -> Eval(tbRef);
        double errorRef = fPulse -> Error(tbRef);
        double weigth = 1./(errorRef*errorRef);
        refy += weigth * valueRef * valueData;
        ref2 += weigth * valueRef * valueRef;
#ifdef DEBUG_CHANA_FITAMPLITUDE
        lk_debug << iTbPulse << "; " << refy << " / " << ref2 << endl;
#endif
    }
    ndfFit = ndfFit-2;
#ifdef DEBUG_CHANA_FITAMPLITUDE
    //lk_debug << "ndf is " << ndf << " ndfFit is " << ndfFit << endl;
#endif

    if (ref2==0) {
        chi2NDF = 1.e10;
        return;
    }
    amplitude = refy / ref2;
#ifdef DEBUG_CHANA_FITAMPLITUDE
    lk_debug << "amplitude = " << amplitude << " = " << refy << " / " << ref2 << endl;
#endif
    chi2NDF = 0;

    ndfFit = 0;
    iTbPulse = 0;
    for (; (ndfFit<ndf && iTbPulse<fNDFPulse); iTbPulse++)
    {
        int tbData = tb0 + iTbPulse;
        //if (tbData<0 || tbData>=350)
        //    lk_debug << "e " << tbData << endl;
        if (tbData>=fTbMax)
            break;
        double valueData = buffer[tbData];
        if (valueData>=fDynamicRange)
            continue;
        ndfFit++;
        double tbRef = iTbPulse - tbOffset + 0.5;
        double valueRefA = amplitude * fPulse -> Eval(tbRef);
        double errorRefA = amplitude * fPulse -> Error(tbRef);
        double residual = (valueData-valueRefA)*(valueData-valueRefA)/errorRefA/errorRefA;
#ifdef DEBUG_CHANA_FITAMPLITUDE
        lk_debug << residual << " = (" << valueData << " - " << valueRefA << ")^2 / " << errorRefA << "^2 at " << tbRef << endl;
#endif
        chi2NDF += residual;
    }
    ndfFit = ndfFit-2;

    //lk_debug << "ndfFit: " << ndfFit << endl;

    chi2NDF = chi2NDF/ndfFit;
    ndf = ndfFit;
}

bool LKChannelAnalyzer::TestPulse(double *buffer, double tbHitPrev, double amplitudePrev, double tbHit, double amplitude)
{
    //int numTbsCorrection = fNumTbsCorrection;
    //if (numTbsCorrection + int(tbHit) >= fTbMax)
    //    numTbsCorrection = fTbMax - int(tbHit);

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

    /*
    for (int iTbPulse = -1; iTbPulse < numTbsCorrection; iTbPulse++) {
        int tb = int(tbHit) + iTbPulse;
        buffer[tb] -= fPulse -> Eval(tb, tbHit, amplitude);
    }
    */

    int tbCorrectionRange1 = tbHit + fPulseRefTbMin;
    int tbCorrectionRange2 = tbHit + fPulseRefTbMax;
    if (tbCorrectionRange1 < 0)       tbCorrectionRange1 = 0;
    if (tbCorrectionRange2 >= fTbMax) tbCorrectionRange2 = fTbMax-1;
    //lk_debug << fPulse << endl;
    for (int tb = tbCorrectionRange1; tb < tbCorrectionRange2; tb++) {
        buffer[tb] -= fPulse -> Eval(tb+0.5, tbHit, amplitude);
    }

    return true;
}
