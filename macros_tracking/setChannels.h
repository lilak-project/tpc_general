const int fNumTypes = 12;
int eMMCenter1     = 0;
int eMMCenter2     = 1;
int eMMLeftSide    = 2;
int eMMLeftCenter  = 3;
int eMMRightSide   = 4;
int eMMRightCenter = 5;
int efSiJunction   = 6;
int efSiOhmic      = 7;
int efCsI          = 8;
int eX6Ohmic       = 9;
int eX6Junction    = 10;
int eCsICT         = 11;

vector<int> fSelTypes = {eMMCenter1, eMMCenter2, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter, efSiJunction, efSiOhmic, efCsI, eX6Ohmic, eX6Junction, eCsICT};
const char *fTypeNames[fNumTypes] = {"MMCenter1", "MMCenter2", "MMLeftSide", "MMLeftCenter", "MMRightSide", "MMRightCenter", "fSiJunction", "fSiOhmic", "fCsI", "X6Ohmic", "X6Junction", "CsICT"};
int fPulseCuts[fNumTypes][10] = {
 // threshold, h1,h2,    width1,width2, tb1,tb2, tbStart, invert?, fixpd?
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter1
    {700,      700,4000, 20,  40,       100,300, 0,       0,       -999},  // eMMCenter2
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

void SetType1()
{
    fSelTypes.clear();
    fSelTypes.push_back(eMMCenter1);
}

void SetTypeMM(int i=-1)
{
    fSelTypes.clear();
    if (i<0 || i==0) fSelTypes.push_back(eMMCenter1);
    if (i<0 || i==1) fSelTypes.push_back(eMMCenter2);
    if (i<0 || i==2) fSelTypes.push_back(eMMLeftSide);
    if (i<0 || i==3) fSelTypes.push_back(eMMLeftCenter);
    if (i<0 || i==4) fSelTypes.push_back(eMMRightSide);
    if (i<0 || i==5) fSelTypes.push_back(eMMRightCenter);
}

void SetTypeAll()
{
    fSelTypes.clear();
    fSelTypes.push_back(eMMCenter1);
    fSelTypes.push_back(eMMCenter2);
    fSelTypes.push_back(eMMLeftSide);
    fSelTypes.push_back(eMMLeftCenter);
    fSelTypes.push_back(eMMRightSide);
    fSelTypes.push_back(eMMRightCenter);
    fSelTypes.push_back(efSiJunction);
    fSelTypes.push_back(efSiOhmic);
    fSelTypes.push_back(efCsI);
    fSelTypes.push_back(eX6Ohmic);
    fSelTypes.push_back(eX6Junction);
    fSelTypes.push_back(eCsICT);
}

int GetType(int cobo, int asad, int aget, int chan) 
{
    if (cobo==0 && (asad==0 || asad==1)) {
        int mchannel = chan;
        //if (chan>11) mchannel -= 1;
        //if (chan>22) mchannel -= 1;
        //if (chan>45) mchannel -= 1;
        //if (chan>57) mchannel -= 1;
             if (chan>57) ;
        else if (chan>45) mchannel -= 1;
        else if (chan>22) ;
        else if (chan>11) mchannel -= 1;

        if ((int(mchannel+1)/2)%2==0)
            return eMMCenter1;
        else
            return eMMCenter2;
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
