#ifndef LKDRIFTELECTRONSIM_HH
#define LKDRIFTELECTRONSIM_HH

#include "TObject.h"
#include "LKLogger.h"

class LKDriftElectronSim : public TObject
{
    public:
        LKDriftElectronSim();
        virtual ~LKDriftElectronSim() { ; }

        bool Init();
        void Print(Option_t *option="") const;

        TString GetTpcName() const  { return fTpcName; }
        TString GetG4DetName() const  { return fG4DetName; }
        TString GetPadPlaneName() const  { return fPadPlaneName; }
        double GetDriftVelocity() const  { return fDriftVelocity; }
        double GetCoefLongDiff() const  { return fCoefLongDiff; }
        double GetCoefTranDiff() const  { return fCoefTranDiff; }
        double GetIonizationEnergy() const  { return fIonizationEnergy; }
        int GetNumElectronsInCluster() const  { return fNumElectronsInCluster; }

        void SetTpcName(TString tpcName) { fTpcName = tpcName; }
        void SetG4DetName(TString g4DetName) { fG4DetName = g4DetName; }
        void SetPadPlaneName(TString padPlaneName) { fPadPlaneName = padPlaneName; }
        void SetDriftVelocity(double driftVelocity) { fDriftVelocity = driftVelocity; }
        void SetIonizationEnergy(double ionizationEnergy) { fIonizationEnergy = ionizationEnergy; }
        void SetNumElectronsInCluster(int numElectronsInCluster) { fNumElectronsInCluster = numElectronsInCluster; }

        void SetGasDiffusion(double valueLongDiff, double valueTranDiff)
        {
            fGasDiffusionType = 0;
            fCoefLongDiff = valueLongDiff;
            fCoefTRanDiff = valueTranDiff;
        }
        void SetGasDiffusionFunc(TH1D *histLongDiff, TH1D*histTranDiff)
        {
            fGasDiffusionType = 1;
            fGasLongDiffHist = histLongDiff;
            fGasTranDiffHist = histTranDiff;
        }
        void SetGasDiffusionFunc(TF1 *funcLongDiff, TF1*funcTranDiff)
        {
            fGasDiffusionType = 2;
            fGasLongDiffFunc = funcLongDiff;
            fGasTranDiffFunc = funcTranDiff;
        }

        void SetCARDiffusion(double value)   { fCARDiffusionType = 0; fCARDiffusionSigma = value; }
        void SetCARDiffusionHist(TH1D *hist) { fCARDiffusionType = 1; fCARDiffusionHist = hist; }
        void SetCARDiffusionFunc(TF1 *func)  { fCARDiffusionType = 2; fCARDiffusionFunc = func; }

        void SetCARGain(double value)   { fCARGainType = 0; fCARGain = value; }
        void SetCARGainHist(TH1D *hist) { fCARGainType = 1; fCARGainHist = hist; }
        void SetCARGainFunc(TF1 *func)  { fCARGainType = 2; fCARGainFunc = func; }


        /// Set number of electrons from currenct energy deposit
        void SetNumElectrons(double edep);

        int GetNextElectronBunch();

        /// return (dx,dy,dl) where dx, dy and dl are diffusion in x, y and length-axis
        TVector3 CalculateGasDiffusion(double driftLength); 

        /// return (dx,dy,ac) where dx, dy and ac are diffusion in x, y and amplified charge
        TVector3 CalculateCARDiffusion(double x, double y, double initialCharge);

    private:
        TString      fTpcName;
        TString      fG4DetName;
        TString      fPadPlaneName;
        double       fDriftVelocity = 0;
        double       fCoefLongDiff = 0;
        double       fCoefTranDiff = 0;
        double       fIonizationEnergy = 0;
        int          fNumElectronsInCluster = 0;

        int          fNumElectronsFromEdep = 0;

        int          fGasDiffusionType = 0; ///< 0: value, 2: function
        double       fGasDiffusion = 0;
        TH1D*        fGasLongDiffHist = nullptr;
        TH1D*        fGasTranDiffHist = nullptr;
        TF1*         fGasLongDiffFunc = nullptr;
        TF1*         fGasTranDiffFunc = nullptr;

        int          fCARDiffusionType = 0; ///< 0: value, 1: hist, 2: function
        double       fCARDiffusionSigma = 0;
        TH1D*        fCARDiffusionHist = nullptr;
        TF1*         fCARDiffusionFunc = nullptr;

        int          fCARGainType = 0; ///< 0: value, 1: hist, 2: function
        double       fCARGain = 0;
        TH1D*        fCARGainHist = nullptr;
        TF1*         fCARGainFunc = nullptr;

    ClassDef(LKDriftElectronSim,1);
};

#endif
