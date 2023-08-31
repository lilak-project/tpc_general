const int fNumTypes = 18;
int eMMCenterSideA0    = 0;
int eMMCenterSideA1    = 1;
int eMMCenterSideA2    = 2;
int eMMCenterSideA3    = 3;
int eMMCenterCenterA0    = 4;
int eMMCenterCenterA1    = 5;
int eMMCenterCenterA2    = 6;
int eMMCenterCenterA3    = 7;
int eMMLeftSide    = 8;
int eMMLeftCenter  = 9;
int eMMRightSide   = 10;
int eMMRightCenter = 11;
int efSiJunction   = 12;
int efSiOhmic      = 13;
int efCsI          = 14;
int eX6Ohmic       = 15;
int eX6Junction    = 16;
int eCsICT         = 17;

vector<int> fSelTypes = {
    eMMCenterSideA0,
    eMMCenterSideA1,
    eMMCenterSideA2,
    eMMCenterSideA3,
    eMMCenterCenterA0,
    eMMCenterCenterA1,
    eMMCenterCenterA2,
    eMMCenterCenterA3,
    eMMLeftSide,
    eMMLeftCenter,
    eMMRightSide,
    eMMRightCenter,
    efSiJunction,
    efSiOhmic,
    efCsI,
    eX6Ohmic,
    eX6Junction,
    eCsICT
};

const char *fTypeNames[fNumTypes] = {
    "MMCenterSideA0",
    "MMCenterSideA1",
    "MMCenterSideA2",
    "MMCenterSideA3",
    "MMCenterCenterA0",
    "MMCenterCenterA1",
    "MMCenterCenterA2",
    "MMCenterCenterA3",
    "MMLeftSide",
    "MMLeftCenter",
    "MMRightSide",
    "MMRightCenter",
    "fSiJunction",
    "fSiOhmic",
    "fCsI",
    "X6Ohmic",
    "X6Junction",
    "CsICT"
};

int fPulseCuts[fNumTypes][10] = {
 // threshold, h1,h2,    width1,width2, tb1,tb2, tbStart, invert?, fixpd?
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterSideA0
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterSideA1
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterSideA2
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterSideA3
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterCenterA0
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterCenterA1
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterCenterA2
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenterCenterA3
    {650,      650,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMLeftSide
    {650,      650,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMLeftCenter
    {750,      750,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMRightSide
    {750,      750,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMRightCenter
    {450,      450,4000, 25,  40,        10, 50, 2,       1,       -999},  // efSiJunction
    {600,      600,4000, 25,  40,        10, 50, 2,       0,       -999},  // efSiOhmic
    {600,      800,4000, 50,1000,        20,100, 2,       1,          0},  // efCsI
    {650,      800,4000, 20,  40,        10, 50, 0,       0,       -999},  // eX6Ohmic
    {600,      700,4000, 15,  40,        10, 50, 0,       1,       -999},  // eX6Junction
    {600,      700,4000, 15,  40,        10, 50, 0,       1,       -999}   // eCsICT
};

void SetType(int i=-1)
{
    fSelTypes.clear();
    if (i<0 || i==eMMCenterSideA0   ) fSelTypes.push_back(eMMCenterSideA0   );
    if (i<0 || i==eMMCenterSideA1   ) fSelTypes.push_back(eMMCenterSideA1   );
    if (i<0 || i==eMMCenterSideA2   ) fSelTypes.push_back(eMMCenterSideA2   );
    if (i<0 || i==eMMCenterSideA3   ) fSelTypes.push_back(eMMCenterSideA3   );
    if (i<0 || i==eMMCenterCenterA0 ) fSelTypes.push_back(eMMCenterCenterA0 );
    if (i<0 || i==eMMCenterCenterA1 ) fSelTypes.push_back(eMMCenterCenterA1 );
    if (i<0 || i==eMMCenterCenterA2 ) fSelTypes.push_back(eMMCenterCenterA2 );
    if (i<0 || i==eMMCenterCenterA3 ) fSelTypes.push_back(eMMCenterCenterA3 );
    if (i<0 || i==eMMLeftSide   ) fSelTypes.push_back(eMMLeftSide   );
    if (i<0 || i==eMMLeftCenter ) fSelTypes.push_back(eMMLeftCenter );
    if (i<0 || i==eMMRightSide  ) fSelTypes.push_back(eMMRightSide  );
    if (i<0 || i==eMMRightCenter) fSelTypes.push_back(eMMRightCenter);
    if (i<0 || i==efSiJunction  ) fSelTypes.push_back(efSiJunction  );
    if (i<0 || i==efSiOhmic     ) fSelTypes.push_back(efSiOhmic     );
    if (i<0 || i==efCsI         ) fSelTypes.push_back(efCsI         );
    if (i<0 || i==eX6Ohmic      ) fSelTypes.push_back(eX6Ohmic      );
    if (i<0 || i==eX6Junction   ) fSelTypes.push_back(eX6Junction   );
    if (i<0 || i==eCsICT        ) fSelTypes.push_back(eCsICT        );
}

void SetTypeAll()
{
}

int GetType(int cobo, int asad, int aget, int chan) 
{
    if (cobo==0 && (asad==0 || asad==1)) {
        int mchannel = chan;
             if(chan<11) mchannel = chan;
        else if(chan<22) mchannel = chan - 1;
        else if(chan<45) mchannel = chan - 2;
        else if(chan<56) mchannel = chan - 3;
        else             mchannel = chan - 4;

        if ((int(mchannel+1)/2)%2==0)
        {
                 if (aget==0) return eMMCenterSideA0;
            else if (aget==1) return eMMCenterSideA1;
            else if (aget==2) return eMMCenterSideA2;
            else if (aget==3) return eMMCenterSideA3;
        }
        else
        {
                 if (aget==0) return eMMCenterCenterA0;
            else if (aget==1) return eMMCenterCenterA1;
            else if (aget==2) return eMMCenterCenterA2;
            else if (aget==3) return eMMCenterCenterA3;
        }
    }
    if (cobo==0 && asad==2 && (aget==0 || aget==1)) return eMMLeftSide;
    if (cobo==0 && asad==2 && (aget==2 || aget==3)) return eMMLeftCenter;
    if (cobo==0 && asad==3 && (aget==0 || aget==1)) return eMMRightSide;
    if (cobo==0 && asad==3 && (aget==2 || aget==3)) return eMMRightCenter;
    if (cobo==1 && asad==0 && aget==0) return efSiJunction;
    if (cobo==1 && asad==0 && aget==1) return efSiOhmic;
    if (cobo==1 && asad==1 && aget==0
        && (chan==2
         || chan==7
         || chan==10
         || chan==16
         || chan==19
         || chan==25
         || chan==28
         || chan==33
         || chan==36
         || chan==41)
       ) 
        return efCsI;
    if (cobo==2 && aget==0) return eX6Ohmic;
    if (cobo==2 && (aget==1 || aget==2)) return eX6Junction;
    if (cobo==2 && aget==3) return eCsICT;

    return -1;
}
