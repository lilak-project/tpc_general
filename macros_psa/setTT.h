TH2Poly* GetHistStrip()
{
    int cobo, asad, aget, chan;
    double x0, x1, x2, z0, z1, z2;

    auto hist1 = new TH2Poly("hist1",";x;z",-125,125,180,420);
    ifstream file1("hist_channels_center_and_strip.txt");
    while (file1 >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2)
        hist1 -> AddBin(x1,z1,x2,z2);

    return hist1;
}

TH2Poly* GetHistChain()
{
    int cobo, asad, aget, chan;
    double x0, x1, x2, z0, z1, z2;

    auto hist1 = new TH2Poly("hist1",";x;z",-125,125,180,420);
    ifstream file2("hist_channels_center_and_chain.txt");
    while (file2 >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2)
        hist1 -> AddBin(x1,z1,x2,z2);

    return hist1;
}

void CAACtoRealDim(Int_t Cobo, Int_t Asad, Int_t Aget, Int_t Chan,
                   Double_t &posx, Double_t &posy, Double_t &posz,
                   Double_t &errx, Double_t &erry, Double_t &errz)
{
    //[mm]
    posx = 0;
    posy = 0;
    posz = 0;
    errx = 0;
    erry = 0;
    errz = 0;

    if(Chan==11 || Chan==22 || Chan==45 || Chan==56) return;
    if(Cobo==0)
    {
        if(Chan<11) Chan = Chan;
        else if(Chan>11 && Chan<22) Chan = Chan -1;
        else if(Chan>22 && Chan<45) Chan = Chan -2;
        else if(Chan>45 && Chan<56) Chan = Chan -3;
        else if(Chan>56) Chan = Chan -4;
    }
    auto fDetector = ((TexAT2*) LKRun::GetRun() -> GetDetector());
    auto type = fDetector -> GetType(Cobo,Asad,Aget,Chan);

    Double_t MMAllX = 240;
    Double_t MMAllZ = 224;
    Double_t minMMZ = 191.7;
    Double_t MMBeamAllLong = 3.43;
    Double_t MMBeamAllShort = 1.75;
    Double_t MMBeamActLong = 3.42;
    Double_t MMBeamActShort = 1.67;
    Double_t MMChainAllShort = 1.714;
    Double_t MMStripX = (MMAllX - MMBeamAllLong*6)/2;
    Double_t MMChainZ = MMAllZ;

    Double_t maxForwardSiZ = 476.9;

    Double_t maxSideX6X = 146.8;
    Double_t minBottomX6Y= -66.1;
    Double_t minX6Z = 147.9;
    Double_t X6AllLong = 93.1;
    Double_t X6AllShort = 45.2;
    Double_t X6ActLong = 75;
    Double_t X6ActShort = 40.3;

    if(type==TexAT2::eType::kLeftStrip ||
       type==TexAT2::eType::kRightStrip ||
       type==TexAT2::eType::kLeftChain ||
       type==TexAT2::eType::kRightChain ||
       type==TexAT2::eType::kLowCenter ||
       type==TexAT2::eType::kHighCenter) //MM
    {
        Int_t mmpx = fDetector -> Getmmpx(Asad,Aget,Chan);
        Int_t mmpy = fDetector -> Getmmpy(Asad,Aget,Chan);
        if(type==TexAT2::eType::kLowCenter || type==TexAT2::eType::kHighCenter) //beam
        {
            posx = (mmpx-66.5)*MMBeamAllLong;
            posz = minMMZ + (mmpy+0.5)*MMBeamAllShort;
            errx = MMBeamActLong/2;
            errz = MMBeamActShort/2;
        }
        else //side
        {
            if(mmpx<0) //strip
            {
                if(Asad==2) posx = -(MMStripX/2+MMBeamAllLong*3);
                else if(Asad==3) posx = (MMStripX/2+MMBeamAllLong*3);
                posz = minMMZ + (mmpy+0.5)*MMBeamAllShort;
                errx = MMStripX/2;
                errz = MMBeamActShort/2;
            }
            else if(mmpy<0) //chain
            {
                if(Asad==2) posx = -(MMBeamAllLong*3+(63.5-mmpx)*MMChainAllShort);
                if(Asad==3) posx = MMBeamAllLong*3+(mmpx-69.5)*MMChainAllShort;
                posz = minMMZ + MMChainZ/2;
                errx = MMBeamActShort/2;
                errz = MMChainZ/2;
            }
        }
        posy = 75/2; //should be checked
        erry = -1; //Detector on xz-plane
    }
    else if(type==TexAT2::eType::kForwardSi) //Forward Si
    {
        Int_t sipx = fDetector -> GetSipx(Asad,Aget,Chan);
        Int_t sipy = fDetector -> GetSipy(Asad,Aget,Chan);
        Int_t sistrip = fDetector -> GetSistrip(Asad,Aget,Chan);
        if(Aget==0) //Junction
        {
            if(sipx==2)
            {
                posx = 0;
                if(sipy==0) posy = -12.5;
                else if(sipy==1) posy = 51.8;
            }
            else
            {
                if(sipx==0) { posx = -124; posy = 35.8; }
                else if(sipx==1) { posx = -62.2; posy = 30.8; }
                else if(sipx==3) { posx = 62.2; posy = 30.8; }
                else if(sipx==4) { posx = 124; posy = 35.8; }
                if(sipy==0) posy *= -1;
            }
            //should be checked below here
            if(sistrip==1) { posx -= 12.5; posy += 12.5; }
            else if(sistrip==2) { posx += 12.5; posy += 12.5; }
            else if(sistrip==3) { posx -= 12.5; posy -= 12.5; }
            else if(sistrip==4) { posx += 12.5; posy -= 12.5; }
            posz = maxForwardSiZ;
            errx = 12.5;
            erry = 12.5;
            errz = -1; //Detector on xy-plane
        }
        else if(Aget==1) //Ohmic
        {
            if(sipx==2)
            {
                posx = 0;
                if(sipy==0) posy = -12.5;
                else if(sipy==1) posy = 51.8;
            }
            else
            {
                if(sipx==0) { posx = -124; posy = 35.8; }
                else if(sipx==1) { posx = -62.2; posy = 30.8; }
                else if(sipx==3) { posx = 62.2; posy = 30.8; }
                else if(sipx==4) { posx = 124; posy = 35.8; }
                if(sipy==0) posy *= -1;
            }
            posz = maxForwardSiZ;
            errx = 25;
            erry = 25;
            errz = -1; //Detector on xy-plane
        }
    }
    else if(type==TexAT2::eType::kCENSX6) //X6
    {
        Int_t det = fDetector -> GetX6det(Asad,Aget,Chan);
        Int_t strip = fDetector -> GetX6strip(Asad,Aget,Chan);
        if(Aget==1 || Aget==2) //Junction
        {
            Double_t posx = fDetector -> GetX6posx(det,strip-1);
            Double_t posy = fDetector -> GetX6posy(det,strip-1);
            Double_t posz = fDetector -> GetX6posz(det,strip-1);
            if(det<100) //Side
            {
                if(det<19) posz += 2.5;
                else if(det<29) posz -= 2.5;
                errx = -1; //Detector on yz-plane
                erry = 1;
                errz = 2.5;
            }
            else //Bottom
            {
                if(det==101 || det==204)
                {
                    posx += 2.5;
                    errx = 2.5;
                    erry = -1; //Detector on xz-plane
                    errz = 1;
                }
                else
                {
                    if(det<109) posz -= 2.5;
                    else if(det<209) posz += 2.5;
                    errx = 1;
                    erry = -1; //Detector on xz-plane
                    errz = 2.5;
                }
            }
        }
        else if(Aget==0) //Ohmic
        {
            if(det<100)
            {
                Int_t map;
                if(det<15) map = det-8;
                else if(det<19) map = det-15;
                else map = 27-det;

                if(det<19) posx = -maxSideX6X;
                else if(det<29) posx = maxSideX6X;
                posy = (strip-2.5)*X6ActLong/4;
                posz = minX6Z + (map+0.5)*X6AllShort;
                errx = -1; //Detector on yz-plane
                erry = X6ActLong/8;
                errz = X6ActShort/2;
            }
            else
            {
                if(det==101 || det==204)
                {
                    if(det==101) posx = -X6AllShort/2;
                    else posx = X6AllShort/2;
                    posy = minBottomX6Y;
                    posz = minX6Z + 7*X6AllShort - X6AllLong/2 + (strip-2.5)*X6ActLong/4;
                    errx = X6ActShort/2;
                    erry = -1; //Detector on xz-plane
                    errz = X6ActLong/8;
                }
                else
                {
                    Int_t map;
                    if(det<105) map = det-98;
                    else if(det<109) map = det-105;
                    else if(det<204) map = 207-det;
                    else map = 208-det;

                    if(det<109) posx = - (X6AllShort + X6AllLong/2 + (strip-2.5)*X6ActLong/4);
                    else posx = X6AllShort + X6AllLong/2 + (strip-2.5)*X6ActLong/4;
                    posy = minBottomX6Y;
                    posz = minX6Z + (map+0.5)*X6AllShort;
                    errx = X6ActLong/8;
                    erry = -1; //Detector on xz-plane
                    errz = X6ActShort/2;
                }
            }
        }
    }
    else //Forward CsI, CENS CsI
    {
        posx = 0;
        posy = 0;
        posz = 0;
        errx = 0;
        erry = 0;
        errz = 0;
    }
}
