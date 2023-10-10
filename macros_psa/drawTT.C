#include "setTT.h"
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
    ofstream fileCS("hist_channels_center_and_strip.txt");
    ofstream fileCC("hist_channels_center_and_chain.txt");
    ofstream file0("hist_channels_LeftStrip.txt");
    ofstream file1("hist_channels_RightStrip.txt");
    ofstream file2("hist_channels_LeftChain.txt");
    ofstream file3("hist_channels_RightChain.txt");
    ofstream file4("hist_channels_LowCenter.txt");
    ofstream file5("hist_channels_HighCenter.txt");
    int cobo=0, asad, aget, chan, dumi;
    while (fileCAAC >> asad >> aget >> chan >> dumi >> dumi)
    {
        auto detType = fDetector -> GetType(cobo,asad,aget,chan);
        auto dchan = chan;
        if(cobo==0)
        {
            if(chan<11) dchan = chan;
            else if(chan<21) dchan = chan +1;
            else if(chan<43) dchan = chan +2;
            else if(chan<53) dchan = chan +3;
            else if(chan<64) dchan = chan +4;
        }

        CAACtoRealDim(cobo,asad,aget,dchan, xPos,yPos,zPos,xErr,yErr,zErr);

        double x0 = xPos;
        double x1 = xPos - xErr;
        double x2 = xPos + xErr;
        double z0 = zPos;
        double z1 = zPos - zErr;
        double z2 = zPos + zErr;

        if (1) {
            if (detType==TexAT2::eType::kLeftStrip
                ||  detType==TexAT2::eType::kRightStrip
                ||  detType==TexAT2::eType::kLowCenter
                ||  detType==TexAT2::eType::kHighCenter)
            {
                auto bin = fHistXZ[0] -> AddBin(x1,z1,x2,z2);
                fileCS << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
                fHistXZ[0] -> SetBinContent(bin,bin);
            }
            if (detType==TexAT2::eType::kLeftChain
                ||  detType==TexAT2::eType::kRightChain
                ||  detType==TexAT2::eType::kLowCenter
                ||  detType==TexAT2::eType::kHighCenter)
            {
                auto bin = fHistXZ[2] -> AddBin(x1,z1,x2,z2);
                fileCC << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
                fHistXZ[2] -> SetBinContent(bin,bin);
            }

            if (detType==TexAT2::eType::kLeftStrip)  file0 << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
            if (detType==TexAT2::eType::kRightStrip) file1 << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
            if (detType==TexAT2::eType::kLeftChain)  file2 << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
            if (detType==TexAT2::eType::kRightChain) file3 << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
            if (detType==TexAT2::eType::kLowCenter)  file4 << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
            if (detType==TexAT2::eType::kHighCenter) file5 << setw(2) << cobo << setw(2) << asad << setw(2) << aget << setw(3) << chan << setw(9) << x0 << setw(9) << x1 << setw(9) << x2 << setw(9) << z0 << setw(9) << z1 << setw(9) << z2 << endl;
        }
        else {
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
            hist -> AddBin(x1,z1,x2,z2);

            if (detType==TexAT2::eType::kLeftStrip)  { hist -> Fill(xPos,zPos,1); }
            else if (detType==TexAT2::eType::kRightStrip) { hist -> Fill(xPos,zPos,2); }
            else if (detType==TexAT2::eType::kLeftChain)  { hist -> Fill(xPos,zPos,3); }
            else if (detType==TexAT2::eType::kRightChain) { hist -> Fill(xPos,zPos,4); }
            else if (detType==TexAT2::eType::kLowCenter)  { hist -> Fill(xPos,zPos,5); }
            else if (detType==TexAT2::eType::kHighCenter) { hist -> Fill(xPos,zPos,6); }
        }
    }

    gStyle -> SetPalette(kBird);

    fCanvas -> cd(1);
    fHistXZ[0] -> SetTitle("eType == Strip and Center");
    fHistXZ[0] -> Draw("same colz");
    //fHistXZ[1] -> Draw("same col");
    //fHistXZ[4] -> Draw("same col");
    //fHistXZ[5] -> Draw("same col");

    fCanvas -> cd(2);
    fHistXZ[2] -> SetTitle("eType == Chain");
    fHistXZ[2] -> Draw("same colz");
    //fHistXZ[3] -> Draw("same col");
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

        fHistXZ[i] = new TH2Poly(Form("hist_%s",name),";x;z",-125,125,180,420);
        fHistXZ[i] -> SetStats(0);
    }
}
