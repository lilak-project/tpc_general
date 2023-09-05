#include "setPosition.h"
#include "ejungwooA.h"

TexAT2* fDetector;
TCanvas* fCanvas = nullptr;
TH2Poly* fHistXZ[10];

void SetDrawing();

void drawTT()
{
    auto fRun = new LKRun();
    fRun -> AddDetector(new TexAT2);
    fRun -> AddPar("config.mac");
    fRun -> SetTag("read");
    fRun -> Init();
    fDetector = (TexAT2*) fRun -> GetDetector();

    SetDrawing();
    Double_t xPos;
    Double_t yPos;
    Double_t zPos;
    Double_t xErr;
    Double_t yErr;
    Double_t zErr;

    TString fileName = fRun -> GetPar() -> GetParString("TexAT2/mapmmFileName");
    ifstream fileCAAC(fileName);
    int cobo=0, asad, aget, chan, dumi;
    while (fileCAAC >> asad >> aget >> chan >> dumi >> dumi)
    {
        auto detType = fDetector -> GetType(cobo,asad,aget,chan);
        if(cobo==0)
        {
            if(chan<11) chan = chan;
            else if(chan<21) chan = chan +1;
            else if(chan<43) chan = chan +2;
            else if(chan<53) chan = chan +3;
            else if(chan<64) chan = chan +4;
        }

        TH2Poly *hist = nullptr;

        if (detType==TexAT2::eType::kNon) continue;
        else if (detType==TexAT2::eType::kLeftStrip)  hist = fHistXZ[0];
        else if (detType==TexAT2::eType::kRightStrip) hist = fHistXZ[1];
        else if (detType==TexAT2::eType::kLeftChain)  hist = fHistXZ[2];
        else if (detType==TexAT2::eType::kRightChain) hist = fHistXZ[3];
        else if (detType==TexAT2::eType::kLowCenter)  hist = fHistXZ[4];
        else if (detType==TexAT2::eType::kHighCenter) hist = fHistXZ[5];
        else
            continue;

        CAACtoRealDim(cobo,asad,aget,chan, xPos,yPos,zPos,xErr,yErr,zErr);

        double x1 = xPos - xErr;
        double x2 = xPos + xErr;
        double z1 = zPos - zErr;
        double z2 = zPos + zErr;

        auto numBins = hist -> GetNumberOfBins();
        hist -> AddBin(x1,z1,x2,z2);

        if (detType==TexAT2::eType::kLeftStrip)  { hist -> Fill(xPos,zPos); }
        else if (detType==TexAT2::eType::kRightStrip) { hist -> Fill(xPos,zPos); }
        else if (detType==TexAT2::eType::kLeftChain)  { hist -> Fill(xPos,zPos); }
        else if (detType==TexAT2::eType::kRightChain) { hist -> Fill(xPos,zPos); }
        else if (detType==TexAT2::eType::kLowCenter)  { hist -> Fill(xPos,zPos); }
        else if (detType==TexAT2::eType::kHighCenter) { hist -> Fill(xPos,zPos); }
    }

    gStyle -> SetPalette(kBird);

    fCanvas -> cd(1);
    fHistXZ[0] -> SetTitle("eType == Strip and Center");
    fHistXZ[0] -> Draw("same colz");
    fHistXZ[1] -> Draw("same colz");
    fHistXZ[4] -> Draw("same colz");
    fHistXZ[5] -> Draw("same colz");

    fCanvas -> cd(2);
    fHistXZ[2] -> SetTitle("eType == Chain");
    fHistXZ[2] -> Draw("same colz");
    fHistXZ[3] -> Draw("same colz");
}

void SetDrawing()
{
    TString names[] = {
        "LeftStrip", "RightStrip",
        "LeftChain", "RightChain",
        "LowCenter", "HighCenter",
        "ForwardSi",
        "ForwardCsI",
        "CENSX6",
        "CENSCsI"
    };

    fCanvas = ejungwoo::Canvas("cvsXZ",100,70,2,1);

    for (auto i=0; i<10; ++i)
    {
        auto name = names[i].Data();

        fHistXZ[i] = new TH2Poly(Form("hist_%s",name),";x;z",-120,120,180,420);
        fHistXZ[i] -> SetStats(0);
    }
}
