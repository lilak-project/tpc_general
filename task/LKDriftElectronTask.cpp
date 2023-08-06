#include "LKDriftElectronTask.h"

ClassImp(LKDriftElectronTask);

LKDriftElectronTask::LKDriftElectronTask()
{
    fName = "LKDriftElectronTask";
}

bool LKDriftElectronTask::Init()
{
    lk_info << "Initializing LKDriftElectronTask" << std::endl;

    fNumTPCs = fPar -> GetParInt("LKDriftElectronTask/numTPCs");
    for (auto iTPC=0; iTPC<numTPCs; ++iTPC)
    {
        fMCStepArray[iTPC] = nullptr;
        fPadArray[iTPC] = nullptr;
        fPadPlane[iTPC] = nullptr;
        fElectronDriftSim[iTPC] = nullptr;

        TString tpcName = fPar -> GetParString("LKDriftElectronTask/tpcNames",iTPC);
        TString g4DetName = fPar -> GetParString(tpcName+"/G4DetectorName");
        TString padPlaneName = fPar -> GetParString(tpcName+"/padPlaneName");
        TString outputBranchName = fPar -> GetParString(tpcName+"/outputBranchName");

        lk_info << "Initializing LKDriftElectronTask >> " << tpcName << " : " << g4DetName << " " << padPlaneName << " " << outputBranchName << std::endl;

        double driftVelocity = fPar -> GetParDouble(tpcName+"/driftVelocity");
        double coefLongDiff = fPar -> GetParDouble(tpcName+"/driftCoefLongDiff");
        double coefTranDiff = fPar -> GetParDouble(tpcName+"/driftCoefTranDiff");
        double ionizationEnergy = fPar -> GetParDouble(tpcName+"/ionizationEnergy");
        int numElectronsInCluster = fPar -> GetParInt(tpcName+"/numElectronsInCluster");

        fMCStepArray[iTPC] = (TClonesArray *) run -> GetBranch(Form("MCStep_%s",g4DetName.Data()));
        fPadPlane[iTPC] = (LKTpc *) run -> GetPadPlane();
        fElectronDriftSim[iTPC] = new LKElectronDriftSim();
        auto sim = fElectronDriftSim[iTPC];
        sim -> SetDriftVelocity(driftVelocity);
        sim -> SetCoefLongDiff(coefLongDiff);
        sim -> SetCoefTranDiff(coefTranDiff);
        sim -> SetIonizationEnergy(ionizationEnergy);
        sim -> SetNumElectronsInCluster(numElectronsInCluster);

        TString gasDiffusion = fPar -> GetParString(tpcName+"/gasDiffusion",0);
        if (gasDiffusion=="value") {
            double valueLD = fPar -> GetParDouble(tpcName+"/gasDiffusion",1);
            double valueTD = fPar -> GetParDouble(tpcName+"/gasDiffusion",2);
            sim -> SetGasDiffusion(valueLD,valueTD);
        }
        if (gasDiffusion.Index("hist")==0) {
            fPar -> GetParDouble(tpcName+"/gasDiffusion",1)
            TString diffValueFile = fPar -> GetParString(tpcName+"/gasDiffusion",1);
            TFile* file = new TFile(diffValueFile,"read");
            auto funcLD = (TF1*) file -> Get("longDiff");
            auto funcTD = (TF1*) file -> Get("tranDiff");
            sim -> SetGasDiffusionFunc(funcLD, funcTD);
        }
        if (gasDiffusion.Index("func")==0) {
            fPar -> GetParDouble(tpcName+"/gasDiffusion",1)
            TString diffValueFile = fPar -> GetParString(tpcName+"/gasDiffusion",1);
            TFile* file = new TFile(diffValueFile,"read");
            auto funcLD = (TF1*) file -> Get("longDiff");
            auto funcTD = (TF1*) file -> Get("tranDiff");
            sim -> SetGasDiffusionFunc(funcLD, funcTD);
        }

        TString carDiffusion = fPar -> GetParString(tpcName+"/CARDiffusion",0);
        if (carDiffusion=="value") {
            double gainValue = fPar -> GetParDouble(tpcName+"/CARDiffusion",1);
            sim -> SetCARDiffusion(gainValue);
        }
        if (carDiffusion.Index("hist")==0) {
            TString gainValueFile = fPar -> GetParString(tpcName+"/CARDiffusion",1);
            TFile* file = new TFile(gainValueFile,"read");
            auto gainFunc = (TH1D*) file -> Get("diffusion");
            sim -> SetCARDiffusionFunc(gainFunc);
        }
        if (carDiffusion.Index("func")==0) {
            TString gainValueFile = fPar -> GetParString(tpcName+"/CARDiffusion",1);
            TFile* file = new TFile(gainValueFile,"read");
            auto gainFunction = (TF1*) file -> Get("diffusion");
            sim -> SetCARDiffusionFunc(gainFunction);
        }

        TString carGain = fPar -> GetParString(tpcName+"/CARGain",0);
        if (carGain=="value") {
            double gainValue = fPar -> GetParDouble(tpcName+"/CARGain",1);
            sim -> SetCARGain(gainValue);
        }
        if (carGain.Index("hist")==0) {
            TString gainValueFile = fPar -> GetParString(tpcName+"/CARGain",1);
            TFile* file = new TFile(gainValueFile,"read");
            auto gainHist = (TH1D*) file -> Get("gain");
            sim -> SetCARGainHist(gainHist);
        }
        if (carGain.Index("func")==0) {
            TString gainValueFile = fPar -> GetParString(tpcName+"/CARGain",1);
            TFile* file = new TFile(gainValueFile,"read");
            auto gainFunction = (TF1*) file -> Get("gain");
            sim -> SetCARGainFunction(gainFunction);
        }

        fPadArray[iTPC] = new TClonesArray("LKPad");
        run -> RegisterBranch(outputBranchName, fPadArray[iTPC]);
    }

    return true;
}

void LKDriftElectronTask::Exec(Option_t *option)
{
    for (auto iTPC=0; iTPC<numTPCs; ++iTPC)
    {
        auto mcArray = fMCStepArray[iTPC];
        auto padArray = fPadArray[iTPC];
        auto padPlane = fPadPlane[iTPC];
        auto sim = fElectronDriftSim[iTPC];

        padArray -> Clear("C");
        padPlane -> Clear();

        /////////////////////////////////////////////////////////////////
        // Fill electrons into pad plane
        /////////////////////////////////////////////////////////////////
        Long64_t numMCSteps = mcArray -> GetEntries();
        for (Long64_t iStep = 0; iStep < numMCSteps; ++iStep)
        {
            LKMCStep* step = (LKMCStep*) mcArray -> At(iStep);
            Int_t trackID = step -> GetTrackID();
            Double_t edep = step -> GetEdep();
            TVector3 posMC(step -> GetX(), step -> GetY(), step -> GetZ());

            auto xyl = padPlane -> DriftElectron(posMC);
            auto xylDiff = sim -> CalculateGasDiffusion(xyl.Z());
            auto xylGas = xyl + xylDiff;

            sim -> SetNumElectrons(edep);
            while((int numElectrons = sim -> GetNextElectronBunch()))
            {
                if (numElectrons==0)
                    break;

                auto xPlane = xylGas.X();
                auto yPlane = xylGas.Y();

                auto tb = sim -> GetTimeBucket(xylGas.Z());
                auto xyc = sim -> CalculateCARDiffusion(xPlane, yPlane, numElectrons);

                xPlane = xPlane + xyc.X();
                yPlane = yPlane + xyc.Y();
                auto charge = xyc.Z();
                padPlane -> FillBufferIn(xPlane, yPlane, tb, charge, trackID);
            }
        }
        /////////////////////////////////////////////////////////////////


        /////////////////////////////////////////////////////////////////
        // Write active pad to output file
        /////////////////////////////////////////////////////////////////
        Int_t idx = 0;
        LKPad *pad;
        TIter itChannel(padPlane -> GetChannelArray());
        while ((pad = (LKPad *) itChannel.Next())) {
            if (pad -> IsActive() == false)
                continue;
            auto padSave = (LKPad*) padArray -> ConstructedAt(idx);
            padSave -> SetPad(pad);
            padSave -> CopyPadData(pad);
            idx++;
        }
        /////////////////////////////////////////////////////////////////

        lk_info << "Number of fired pads in plane-" << iPlane << ": " << padArray -> GetEntries() << endl;
    }
}

bool LKDriftElectronTask::EndOfRun()
{
    return true;
}
