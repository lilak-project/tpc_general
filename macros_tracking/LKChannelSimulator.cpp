#include "LKChannelSimulator.h"

ClassImp(LKChannelSimulator);

LKChannelSimulator::LKChannelSimulator()
{
}

void LKChannelSimulator::SetPulse(const char* fileName)
{
    fPulse = new LKPulse(fileName);

    fBackGroundLevel = fPulse -> GetBackGroundLevel();
    fPedestalFluctuationLevel = fPulse -> GetFluctuationLevel();
}

void LKChannelSimulator::SetPedestal(int* buffer)
{
    double pedestalFluctuationLevel = fPedestalFluctuationScale * fPedestalFluctuationLevel;
    for (auto tb=0; tb<fTbMax; ++tb)
        buffer[tb] = gRandom -> Gaus(0, pedestalFluctuationLevel);

    //LKChannelSimulator::Smoothing(buffer,fTbMax,fSmoothingLength);
    Smoothing(buffer,fTbMax,fSmoothingLength,fNumSmoothing);
}

void LKChannelSimulator::SetFluctuatingPedestal(int* buffer)
{
    int pmFluctuation = 1;
    double pedestalFluctuationLevel = fPedestalFluctuationScale * fPedestalFluctuationLevel;
    int valuePointer = gRandom -> Gaus(0, pedestalFluctuationLevel);
    valuePointer = fBackGroundLevel + pmFluctuation*(valuePointer);
    pmFluctuation = -pmFluctuation;

    int tbPointer = 0;
    buffer[tbPointer++] = valuePointer;

    while (tbPointer<fTbMax)
    {
        int tbFlucLength = gRandom -> Gaus(fPedestalFluctuationLength,fPedestalFluctuationLength*0.2);
        tbFlucLength = abs(tbFlucLength);
        if (tbFlucLength==0) tbFlucLength = 1;

        int valueTarget = gRandom -> Gaus(0, pedestalFluctuationLevel);
        valueTarget = fBackGroundLevel + pmFluctuation*(valueTarget);
        pmFluctuation = -pmFluctuation;

        int dValueTotal = valueTarget - valuePointer;
        int dValuePerLength = dValueTotal/tbFlucLength;
        if (tbPointer+tbFlucLength>fTbMax)
            tbFlucLength = fTbMax - tbPointer;

        for (int iTb=0; iTb<tbFlucLength-1; ++iTb)
        {
            int dValue = gRandom -> Gaus(dValuePerLength,0.5*dValuePerLength);
            valuePointer = valuePointer + dValue;
            buffer[tbPointer++] = valuePointer;
        }

        int dValueLast = valueTarget - valuePointer;
        valuePointer = valuePointer + dValueLast;
        buffer[tbPointer++] = valuePointer;

        //lk_debug << tbPointer << " " << tbFlucLength << " " << valueTarget << endl;
    }

    Smoothing(buffer,fTbMax,fSmoothingLength,fNumSmoothing);
}

void LKChannelSimulator::AddHit(int* buffer, double tb0, double amplitude)
{
    for (auto tb=0; tb<fTbMax; ++tb)
    {
        double value = fPulse -> Eval(tb, tb0, amplitude);
        if (value>1 && fPulseErrorScale>0)
        {
            double error = gRandom -> Gaus(0, fPulse->Error0(tb,tb0,fPulseErrorScale*amplitude));
            value = value + error;
        }
        buffer[tb] = buffer[tb] + value;
    }
}

void LKChannelSimulator::Smoothing(int* buffer, int n, int smoothingLevel, int numSmoothing)
{
    for (int it=0; it<numSmoothing; ++it)
        for (int i=0; i<n; i++)
        {
            double sum = 0.;
            int count = 0;

            for (int j = i-smoothingLevel; j<=i+smoothingLevel; j++) {
                if (j>=0 && j<n) {
                    sum += buffer[j];
                    count++;
                }
            }
            buffer[i] = sum / count;
        }
}
