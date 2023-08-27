#ifndef TTPULSEEXTRACTIONTASK_HH
#define TTPULSEEXTRACTIONTASK_HH

#include "TClonesArray.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"
#include "LKRun.h"
#include "LKTask.h"
#include "LKPulseAnalyzer.h"
#include "MMChannel.h"

#define fNumTypes 18

class TTPulseExtractionTask : public LKTask
{
    public:
        TTPulseExtractionTask();
        virtual ~TTPulseExtractionTask() { ; }

        bool Init();
        void Exec(Option_t *option="");
        bool EndOfRun();

        int GetType(int cobo, int asad, int aget, int chan);

    private:
        TClonesArray* fChannelArray = nullptr;

        //const int fNumTypes = 18;
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

        LKPulseAnalyzer *fPulseAnalyzer[fNumTypes];

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

        int fExtractionCuts[fNumTypes][10] = {
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


    ClassDef(TTPulseExtractionTask,1);
};

#endif
