const int fNumTypes = 11;
int eMMCenter      = 0;
int eMMLeftSide    = 1;
int eMMLeftCenter  = 2;
int eMMRightSide   = 3;
int eMMRightCenter = 4;
int efSiJunction   = 5;
int efSiOhmic      = 6;
int efCsI          = 7;
int eX6Ohmic       = 8;
int eX6Junction    = 9;
int eCsICT         = 10;

vector<int> fAllTypes = {eMMCenter, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter, efSiJunction, efSiOhmic, efCsI, eX6Ohmic, eX6Junction, eCsICT};
//vector<int> fSelTypes = {eMMCenter};
//vector<int> fSelTypes = {eMMCenter, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter};
vector<int> fSelTypes = {eMMCenter, eMMLeftSide, eMMLeftCenter, eMMRightSide, eMMRightCenter, efSiJunction, efSiOhmic, efCsI, eX6Ohmic, eX6Junction, eCsICT};
//vector<int> fSelTypes = {eMMCenter,efCsI};

const char *fTypeNames[fNumTypes] = {"MMCenter", "MMLeftSide", "MMLeftCenter", "MMRightSide", "MMRightCenter", "fSiJunction", "fSiOhmic", "fCsI", "X6Ohmic", "X6Junction", "CsICT"};
bool fInvertChannel[fNumTypes] = {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1};
int fTbStart[] = {0,0,0,0,0,2,2,2,0,0,0};
int fPulseCuts[fNumTypes][7] = { // threshold, height1, height2, width1, width2, tb1, tb2
    {700,  700,4000, 20,  40,  10,100},  // eMMCenter      = 0;
    {650,  650,4000, 20,  40, 100,300},  // eMMLeftSide    = 1;
    {650,  650,4000, 20,  40, 100,300},  // eMMLeftCenter  = 2;
    {750,  750,4000, 20,  40, 100,300},  // eMMRightSide   = 3;
    {750,  750,4000, 20,  40, 100,300},  // eMMRightCenter = 4;
    {450,  450,4000, 25,  40,  10, 50},  // efSiJunction   = 5;
    {600,  600,4000, 25,  40,  10, 50},  // efSiOhmic      = 6;
    {600,  800,4000, 50,1000,  20,100},  // efCsI          = 7;
    {650,  800,4000, 20,  40,  10, 50},  // eX6Ohmic       = 8;
    {600,  700,4000, 15,  40,  10, 50},  // eX6Junction    = 9;
    {600,  700,4000, 15,  40,  10, 50}   // eCsICT         = 10;
};

int fFixPedestal[] = {-999,-999,-999,-999,-999,-999,-999,0,-999,-999,-999};

void SetType1()
{
    fSelTypes.clear();
    fAllTypes.push_back(eMMCenter);
}

void SetTypeMM()
{
    fSelTypes.clear();
    fAllTypes.push_back(eMMCenter);
    fAllTypes.push_back(eMMLeftSide);
    fAllTypes.push_back(eMMLeftCenter);
    fAllTypes.push_back(eMMRightSide);
    fAllTypes.push_back(eMMRightCenter);
}

void SetTypeAll()
{
    fSelTypes.clear();
    fAllTypes.push_back(eMMCenter);
    fAllTypes.push_back(eMMLeftSide);
    fAllTypes.push_back(eMMLeftCenter);
    fAllTypes.push_back(eMMRightSide);
    fAllTypes.push_back(eMMRightCenter);
    fAllTypes.push_back(efSiJunction);
    fAllTypes.push_back(efSiOhmic);
    fAllTypes.push_back(efCsI);
    fAllTypes.push_back(eX6Ohmic);
    fAllTypes.push_back(eX6Junction);
    fAllTypes.push_back(eCsICT);
}

int GetType(int cobo, int asad, int aget, int chan) 
{
    if (cobo==0 && (asad==0 || asad==1)) return eMMCenter;
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
