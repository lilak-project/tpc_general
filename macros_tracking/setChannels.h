const int fNumTypes = 18;
int eMMCenter00    = 0;
int eMMCenter01    = 1;
int eMMCenter02    = 2;
int eMMCenter03    = 3;
int eMMCenter10    = 4;
int eMMCenter11    = 5;
int eMMCenter12    = 6;
int eMMCenter13    = 7;
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
    eMMCenter00,
    eMMCenter01,
    eMMCenter02,
    eMMCenter03,
    eMMCenter10,
    eMMCenter11,
    eMMCenter12,
    eMMCenter13,
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
    "eMMCenter00",
    "eMMCenter01",
    "eMMCenter02",
    "eMMCenter03",
    "eMMCenter10",
    "eMMCenter11",
    "eMMCenter12",
    "eMMCenter13",
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
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter00
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter01
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter02
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter03
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter10
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter11
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter12
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter13
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

void SetType0()
{
    fSelTypes.clear();
    fSelTypes.push_back(eMMCenter00);
}

void SetMMType()
{
    fSelTypes.clear();
    fSelTypes.push_back(eMMCenter00   );
    fSelTypes.push_back(eMMCenter01   );
    fSelTypes.push_back(eMMCenter02   );
    fSelTypes.push_back(eMMCenter03   );
}

void SetType(int i=-1)
{
    fSelTypes.clear();
    if (i<0 || i==eMMCenter00   ) fSelTypes.push_back(eMMCenter00   );
    if (i<0 || i==eMMCenter01   ) fSelTypes.push_back(eMMCenter01   );
    if (i<0 || i==eMMCenter02   ) fSelTypes.push_back(eMMCenter02   );
    if (i<0 || i==eMMCenter03   ) fSelTypes.push_back(eMMCenter03   );
    if (i<0 || i==eMMCenter10   ) fSelTypes.push_back(eMMCenter10   );
    if (i<0 || i==eMMCenter11   ) fSelTypes.push_back(eMMCenter11   );
    if (i<0 || i==eMMCenter12   ) fSelTypes.push_back(eMMCenter12   );
    if (i<0 || i==eMMCenter13   ) fSelTypes.push_back(eMMCenter13   );
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
        if (chan>11) mchannel -= 1;
        if (chan>22) mchannel -= 1;
        if (chan>45) mchannel -= 1;
        if (chan>57) mchannel -= 1;
        //     if (chan>57) ;
        //else if (chan>45) mchannel -= 1;
        //else if (chan>22) ;
        //else if (chan>11) mchannel -= 1;

        if ((int(mchannel+1)/2)%2==0)
        {
                 if (aget==0) return eMMCenter00;
            else if (aget==1) return eMMCenter01;
            else if (aget==2) return eMMCenter02;
            else if (aget==3) return eMMCenter03;
        }
        else
        {
                 if (aget==0) return eMMCenter10;
            else if (aget==1) return eMMCenter11;
            else if (aget==2) return eMMCenter12;
            else if (aget==3) return eMMCenter13;
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
