#ifndef TTMICROMEGASCENTERCHAIN_HH
#define TTMICROMEGASCENTERCHAIN_HH

#include "LKLogger.h"
#include "LKPadPlane.h"

/*
 * Remove this comment block after reading it through
 * or use print_example_comments=False option to omit printing
 *
 * # Example LILAK pad plane class
 *
 * # Given members in LKDetectorPlane class
 *
 * ## public:
 * virtual void Print(Option_t *option = "") const;
 * virtual void Clear(Option_t *option = "");
 * virtual Int_t FindPadID(Double_t i, Double_t j) { return (LKPad *) FindChannelID(i,j); }
 * virtual Int_t FindPadID(Int_t section, Int_t row, Int_t layer) { return (LKPad *) FindChannelID(section,row,layer); }
 * LKPad *GetPadFast(Int_t padID);
 * LKPad *GetPad(Int_t padID);
 * LKPad *GetPad(Double_t i, Double_t j);
 * LKPad *GetPad(Int_t section, Int_t row, Int_t layer);
 * void SetPadArray(TClonesArray *padArray);
 * void SetHitArray(TClonesArray *hitArray);
 * Int_t GetNumPads();
 * void FillBufferIn(Double_t i, Double_t j, Double_t tb, Double_t val, Int_t trackID = -1);
 * void SetPlaneK(Double_t k);
 * Double_t GetPlaneK();
 * virtual void ResetHitMap();
 * virtual void ResetEvent();
 * void AddHit(LKTpcHit *hit);
 * virtual LKTpcHit *PullOutNextFreeHit();
 * void PullOutNeighborHits(vector<LKTpcHit*> *hits, vector<LKTpcHit*> *neighborHits);
 * void PullOutNeighborHits(TVector2 p, Int_t range, vector<LKTpcHit*> *neighborHits);
 * void PullOutNeighborHits(Double_t x, Double_t y, Int_t range, vector<LKTpcHit*> *neighborHits);
 * void PullOutNeighborHits(LKHitArray *hits, LKHitArray *neighborHits);
 * void PullOutNeighborHits(Double_t x, Double_t y, Int_t range, LKHitArray *neighborHits);
 * void GrabNeighborPads(vector<LKPad*> *pads, vector<LKPad*> *neighborPads);
 * TObjArray *GetPadArray();
 * bool PadPositionChecker(bool checkCorners = true);
 * bool PadNeighborChecker();
 *
 * ## private:
 * virtual void ClickedAtPosition(Double_t x, Double_t y);
 * Int_t fEFieldAxis = -1;
 * Int_t fFreePadIdx = 0;
 * bool fFilledPad = false;
 * bool fFilledHit = false;
 * TCanvas *fCvsChannelBuffer = nullptr;
 * TGraph *fGraphChannelBoundary = nullptr;
 * TGraph *fGraphChannelBoundaryNb[20] = {0};
 *
 */

class TTFrontPlane : public LKPadPlane
{
    public:
        static TTFrontPlane* GetMMCC();

        TTFrontPlane();
        virtual ~TTFrontPlane() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        bool IsInBoundary(Double_t x, Double_t y);
        Int_t FindChannelID(Double_t x, Double_t y);
        Int_t FindChannelID(Int_t section, Int_t row, Int_t layer);

        TCanvas* GetCanvas(Option_t *option="");
        TH2* GetHist(Option_t *option="");

        bool SetDataFromBranch();
        void DrawFrame(Option_t *option="");
        void Draw(Option_t *option="");

        void SelectAndDrawChannel(bool isChain=0, Int_t bin=-1);
        void SelectAndDrawChannelChain(Int_t bin=-1) { SelectAndDrawChannel(true,bin); }
        void FillDataToHist();

        static void MouseClickEventChain();
        //void ClickedAtPosition(Double_t x, Double_t y);

    private:
        TH2Poly* fHistPlaneChain = nullptr;
        TH1D*    fHistChannelChain = nullptr;
        double_t fXMin = DBL_MAX;
        Double_t fXMax = -DBL_MAX;
        Double_t fZMin = DBL_MAX;
        Double_t fZMax = -DBL_MAX;

        TGraph *fGraphChannelBoundaryChain = nullptr;

        TClonesArray* fBufferArray = nullptr;
        TClonesArray* fHitCenterArray = nullptr;
        TClonesArray* fHitLChainArray = nullptr;
        TClonesArray* fHitRChainArray = nullptr;

        std::map<int, int> fMapBinToCAACChain;
        std::map<int, int> fMapCAACToBinChain;
        std::map<int, double> fMapBinToX1Chain;
        std::map<int, double> fMapBinToX2Chain;
        std::map<int, double> fMapBinToZ1Chain;
        std::map<int, double> fMapBinToZ2Chain;

        static TTFrontPlane* fInstance;

    ClassDef(TTFrontPlane,1);
};

#endif
