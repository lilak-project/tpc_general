//#define KUNPL_NEED_THIS

//#define e_test cout
//#define e_info cout
//#define e_debug cout
//#define lk_debug cout

#ifndef LKWINDOWMANAGER_HH
#define LKWINDOWMANAGER_HH

#include "TObject.h"
#include "LKLogger.h"
#include "TCanvas.h"

#define lk_win LKWindowManager::GetWindowManager()
#define lk_cvs LKWindowManager::GetWindowManager() -> Canvas(name)

class LKWindowManager : public TObject
{
    public:
        LKWindowManager();
        virtual ~LKWindowManager() { ; }
        static LKWindowManager* GetWindowManager();

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        Int_t  GetWDisplay() const  { return fWDisplay; }
        Int_t  GetHDisplay() const  { return fHDisplay; }
        UInt_t GetDWFull() const  { return fDWFull; }
        UInt_t GetDHFull() const  { return fDHFull; }

        void SetDeadFrameLeft(UInt_t val) {
            fDeadFrame[0] = val;
            if (fWCurrent==0)
                fWCurrent = val;
        }
        void SetDeadFrameRight(UInt_t val) {
            fDeadFrame[0] = val;
        }
        void SetDeadFrameBottom(UInt_t val) {
            fDeadFrame[0] = val;
        }
        void SetDeadFrameTop(UInt_t val) {
            fDeadFrame[0] = val;
            if (fHCurrent==0)
                fHCurrent = val;
        }

        void SetDeadFrame(UInt_t left, UInt_t right, UInt_t bottom, UInt_t top) {
            SetDeadFrameLeft(left);
            SetDeadFrameRight(right);
            SetDeadFrameBottom(bottom);
            SetDeadFrameTop(top);
        }

        void SetDWCanvas(UInt_t wCanvas) { fDWCanvas = wCanvas; }
        void SetDHCanvas(UInt_t hCanvas) { fDHCanvas = hCanvas; }
        void SetDWSpacing(UInt_t dwCanvas) { fDWSpacing = dwCanvas; }
        void SetDHSpacing(UInt_t dhCanvas) { fDHSpacing = dhCanvas; }

        void SetGlobalScale(Double_t xScale, Double_t yScale) {
            fWGlobalScale = xScale;
            fHGlobalScale = yScale;
        }

        TCanvas *Canvas   (const char *name)                  { return CanvasR(name, 0, 0, 0, 0, 0, 0); }
        TCanvas *CanvasRH (const char *name, Double_t rh=0.5) { return CanvasR(name, 0, 0, 0, 0, 0, rh); }
        TCanvas *CanvasRW (const char *name, Double_t rw=0.4) { return CanvasR(name, 0, 0, 0, 0, rw, 0); }
        TCanvas *CanvasFull(const char *name)                 { return CanvasR(name, 0, 0, 0, 0, 1, 1); }

        /**
         * @param w
         *      - w(x) position of the canvas in the display 
         * @param h
         *      - h(y) position of the canvas in the display 
         * @param ww
         *      - width of the canvas
         * @param wh
         *      - height of the canvas
         * @param rw
         *      - Ratio of canvas width relative to the full display size
         *      - If rw>0 and rh<=0, canvas size is (rw*fDWFull, rw*fDWFull*ww/wh)
         *      (fDWFull and fDHFull is the width and height of the current display)
         *      - If both rw and rh are >0, canvas size is (rw*fDWFull, rh*fDHFull)
         * @param rh
         *      - Ratio of canvas height relative to the full display size
         *      - If rw<=0 and rh>0, canvas size is (rh*fDHFull, rh*fDHFull*wh/ww)
         *      (fDWFull and fDHFull is the width and height of the current display)
         *      - If both rw and rh are >0, canvas size is (rw*fDWFull, rh*fDHFull)
         */
        TCanvas *CanvasR(const char *name, Int_t w=0, Int_t h=0, Int_t ww=0, Int_t wh=0, Double_t rw=0, Double_t rh=0);

    private:
        void ConfigureDisplay();

    private:
        static LKWindowManager* fInstance;

        Int_t        fWDisplay = -1; /// relative width  position of current display from main display
        Int_t        fHDisplay = -1; /// relative height position of current display from main display

        UInt_t       fDWFull = 0; /// width  of current display
        UInt_t       fDHFull = 0; /// height of current display

        UInt_t       fDeadFrame[4] = {0}; /// height of the top bar

        Int_t        fWCurrent = 0;  /// width  position of next canvas
        Int_t        fHCurrent = 0;  /// height position of next canvas

        Int_t        fDWCanvas = 600; /// default width  of canvas
        Int_t        fDHCanvas = 450; /// default height of canvas

        Int_t        fDWSpacing = 25; /// default width  spacing of canvas
        Int_t        fDHSpacing = 25; /// default height spacing of canvas

        Double_t     fWGlobalScale = 1; /// default width  spacing of canvas
        Double_t     fHGlobalScale = 1; /// default height spacing of canvas

    ClassDef(LKWindowManager,1);
};

#include "TGClient.h"
#include "TGWindow.h"
#include "TVirtualX.h"
//#include "LKWindowManager.h"
#include <iostream>
using namespace std;

ClassImp(LKWindowManager);

LKWindowManager* LKWindowManager::fInstance = nullptr;

LKWindowManager* LKWindowManager::GetWindowManager() {
    if (fInstance == nullptr)
        fInstance = new LKWindowManager();
    return fInstance;
}

LKWindowManager::LKWindowManager()
{
    Init();
}

bool LKWindowManager::Init()
{
    // Put intialization todos here which are not iterative job though event
    cout << "Initializing LKWindowManager" << std::endl;

    ConfigureDisplay();

    return true;
}

void LKWindowManager::Clear(Option_t *option)
{
    TObject::Clear(option);
    fWDisplay = -1;
    fHDisplay = -1;
    fDWFull = -1;
    fDHFull = -1;
    fDeadFrame[0] = 0;
    fDeadFrame[1] = 0;
    fDeadFrame[2] = 0;
    fDeadFrame[3] = 0;
    fWCurrent = 0;
    fHCurrent = 0;
    fDWCanvas = 600;
    fDHCanvas = 450;
    fDWSpacing = 25;
    fDHSpacing = 25;
    fWGlobalScale = 1;
    fHGlobalScale = 1;
}

void LKWindowManager::Print(Option_t *option) const
{
    cout << "fWDisplay    " << fWDisplay     << endl;
    cout << "fHDisplay    " << fHDisplay     << endl;
    cout << "fDWFull      " << fDWFull       << endl;
    cout << "fDHFull      " << fDHFull       << endl;
    cout << "fDeadFrame-L " << fDeadFrame[0] << endl;
    cout << "fDeadFrame-R " << fDeadFrame[1] << endl;
    cout << "fDeadFrame-B " << fDeadFrame[2] << endl;
    cout << "fDeadFrame-T " << fDeadFrame[3] << endl;
    cout << "fWCurrent    " << fWCurrent     << endl;
    cout << "fHCurrent    " << fHCurrent     << endl;
    cout << "fDWCanvas    " << fDWCanvas     << endl;
    cout << "fDHCanvas    " << fDHCanvas     << endl;
    cout << "fDWSpacing   " << fDWSpacing    << endl;
    cout << "fDHSpacing   " << fDHSpacing    << endl;
}

void LKWindowManager::ConfigureDisplay()
{
    Drawable_t id = gClient->GetRoot()->GetId();
    gVirtualX -> GetWindowSize(id, fWDisplay, fHDisplay, fDWFull, fDHFull);

    fDWFull = fDWFull*0.95;
    fDHFull = fDHFull*0.95;

    fDWCanvas = fDWFull*0.55;
    fDHCanvas = fDWCanvas*450/600;

    fWCurrent = fDWFull*0.05;
    fHCurrent = fDHFull*0.1;

    cout << "Display position = (" << fWDisplay << ", " << fHDisplay << "), size = (" << fDWFull << ", " << fDHFull << ")" << endl;
}

TCanvas *LKWindowManager::CanvasR(const char *name, Int_t w, Int_t h, Int_t ww, Int_t wh, Double_t rw, Double_t rh)
{
    if (w<0) { w = fWCurrent; }
    if (h<0) { h = fHCurrent; }

    if (ww<=0 && wh<=0) {
        ww = fDWCanvas;
        wh = fDHCanvas;
    }
    else if (ww<=0) { ww = wh * (double(fDWCanvas)/fDHCanvas); }
    else if (wh<=0) { wh = ww * (double(fDHCanvas)/fDWCanvas); }

    Int_t dwFinal = fDWCanvas;
    Int_t dhFinal = fDHCanvas;

    if (rw>1) rw = 0.01*rw;
    if (rh>1) rh = 0.01*rh;
    if (rw>0 && rw<=1 && rh>0 && rh<=1) {
        dwFinal = rw * fDWFull;
        dhFinal = rh * fDHFull;
    }
    else if (rw>0 && rw<=1) {
        dwFinal = rw * fDWFull;
        dhFinal = dwFinal * (double(wh)/ww);
    }
    else if (rh>0 && rh<=1) {
        dhFinal = rh * fDHFull;
        dwFinal = dhFinal * (double(ww)/wh);
    }

    dwFinal = dwFinal * fWGlobalScale;
    dhFinal = dhFinal * fHGlobalScale;

    //cout << dwFinal << " " << dhFinal << endl;
    auto cvs = new TCanvas(name, name, w, h, dwFinal, dhFinal);

    fWCurrent = fWCurrent + fDWSpacing; 
    fHCurrent = fHCurrent + fDHSpacing; 

    return cvs;
}
#endif


#ifndef __EJUNGWOO_A__
#define __EJUNGWOO_A__

#ifndef KUNPL_NEED_THIS
    THttpServer* eServer = nullptr;
    TObjArray* eArray = nullptr;
#endif

#define e_batch()                   ejungwoo::Batch();
#define e_add(obj,subfolder)        ejungwoo::Add(obj,subfolder);
#define e_cvs(name)                 ejungwoo::Canvas(name,1);
#define e_cvs_full(name)            ejungwoo::Canvas(name,0);
#define e_save(obj)                 ejungwoo::Save(obj);
#define e_save_all()                ejungwoo::SaveAll();
#define e_frame(graph,name,title)   ejungwoo::Frame(graph,name,title);
#define e_fitg(hist,sigmaWidth)     ejungwoo::FitG(hist,sigmaWidth);

namespace ejungwoo
{
    void     Batch();
    void     Add    (TObject* object, const char *subfolder="");
    void     Save   (TObject *object, const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false);
    TCanvas* Canvas (const char* name="", int wx=800, int wy=680, int dx=1, int dy=1);
    void     SaveAll(const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false);
    TH2D*    Frame  (TGraph *graph, const char* name);
    TH2D*    Frame  (TGraph *graph, const char* name, const char* title="");
    TF1*     FitG   (TH1D* hist, double sigmaWidth, Option_t *opt="RQ0");
}

void ejungwoo::Batch()
{
#ifdef LKWINDOWMANAGER_HH
    LKWindowManager::GetWindowManager();
#endif
    gROOT -> SetBatch(1);
}

void ejungwoo::Add(TObject* object, const char *subfolder="")
{
#ifdef KUNPL_NEED_THIS
    THttpServer* eServer = (THttpServer*) gROOT->FindObject("http");
    TObjArray* eArray = (TObjArray*) gROOT->FindObject("ejObjArray");
#endif
    if (eServer==nullptr) {
        eServer = new THttpServer("http:8080");
        gROOT -> Add(eServer);
        //gROOT -> ls();
        cout << "Http eServer : http://localhost:8080/" << endl;
    }

    if (eArray==nullptr) {
        eArray = new TObjArray();
        eArray -> SetName("ejObjArray");
        gROOT -> Add(eArray);
    }

    eServer -> Register(subfolder, object);
    eArray -> Add(object);
}

TCanvas *ejungwoo::Canvas(const char* name="", int wx=800, int wy=680, int dx=1, int dy=1)
{
    const char* title = name;
    TCanvas* cvs0 = nullptr;
#ifdef LKWINDOWMANAGER_HH
         if (wx==0) cvs0 = LKWindowManager::GetWindowManager() -> CanvasFull(name);
    else if (wx==1) cvs0 = LKWindowManager::GetWindowManager() -> Canvas(name);
    else            cvs0 = LKWindowManager::GetWindowManager() -> CanvasR(name, wx, wy);
#else
         if (wx==0) cvs0 = new TCanvas(name,name,1250,720);
    else if (wx==1) cvs0 = new TCanvas(name,name,800,650);
    else            cvs0 = new TCanvas(name,name,wx,wy);
#endif
    //auto cvs0 = new TCanvas(name, title, wx, wy);
    if (dx>1||dy>1) {
        cvs0 -> Divide(dx,dy);
        for (auto ix=0; ix<dx; ++ix)
            for (auto iy=0; iy<dy; ++iy) {
                auto cvsIn = cvs0 -> cd(iy*dx+ix+1);
                cvsIn -> SetLeftMargin(0.12);
                cvsIn -> SetRightMargin(0.15);
            }
    }
    else {
        cvs0 -> SetLeftMargin(0.12);
        cvs0 -> SetRightMargin(0.15);
    }
    Add(cvs0);
    return cvs0;
}

void ejungwoo::Save(TObject *object, const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false)
{
    //if (TString(nameVersion).IsNull()) {
    if (strcmp(nameVersion,"")==0) {
        TTimeStamp timeStamp;
        UInt_t year, month, day;
        timeStamp.GetDate(true, 0, &year, &month, &day);
        nameVersion = Form("save_%04d_%02d_%02d", year, month, day);
    }
    bool home_is_ejungwoo = false;
    if (TString(gSystem->Getenv("HOME")).Index("ejungwoo")>=0)
        home_is_ejungwoo = true;
    const char* pathToData = nameVersion;
    const char* pathToVersion = nameVersion;
    if (home_is_ejungwoo) {
        pathToData = Form("%s/data/auto/%s",gSystem->Getenv("HOME"),nameVersion);
        pathToVersion = Form("%s/data/auto/%s",gSystem->Getenv("HOME"),nameVersion);
    }

    //if (TString(fileType).IsNull())  {
    if (strcmp(fileType,"")==0) {
        //Save(object,"pdf");
        //Save(object,"eps");
        Save(object,"png");
        Save(object,"root");
        return;
    }
    //else if (TString(fileType)=="pdf")  { pathToData = Form("%s/pdf",pathToVersion); }
    //else if (TString(fileType)=="png")  { pathToData = Form("%s/png",pathToVersion); }
    //else if (TString(fileType)=="eps")  { pathToData = Form("%s/eps",pathToVersion); }
    //else if (TString(fileType)=="root") { pathToData = Form("%s/roo",pathToVersion); }
    else if (strcmp(fileType,"pdf")== 0) { pathToData = Form("%s/pdf",pathToVersion); }
    else if (strcmp(fileType,"png")== 0) { pathToData = Form("%s/png",pathToVersion); }
    else if (strcmp(fileType,"eps")== 0) { pathToData = Form("%s/eps",pathToVersion); }
    else if (strcmp(fileType,"root")==0) { pathToData = Form("%s/roo",pathToVersion); }
    else 
        return;
    gSystem -> mkdir(pathToVersion);
    gSystem -> mkdir(pathToData);
    if (home_is_ejungwoo) {
        gSystem -> Symlink(pathToVersion,nameVersion);
    }

    if (object->InheritsFrom(TPad::Class())) {
        auto cvs = (TPad*) object;
        cvs -> Update();
        cvs -> Modified();
    }
    const char* nameObject = object -> GetName();
    const char* nameWrite = object -> GetName();
    if (simplifyNames) {
        if (object->InheritsFrom(TPad::Class()))    nameWrite = "cvs";
        else if (object->InheritsFrom(TH1::Class()))     nameWrite = "hist";
        else if (object->InheritsFrom(TPad::Class()))    nameWrite = "pad";
        else if (object->InheritsFrom(TF1::Class()))     nameWrite = "func";
        else if (object->InheritsFrom(TGraph::Class()))  nameWrite = "graph";
    }
    const char* fileName = Form("%s/%s.%s",pathToData,nameObject,fileType);
    //std::cout << "Creating " << fileName << endl;
    //if (TString(fileType)=="root")  {
    if (strcmp(fileType,"root")==0)  {
        auto fileOut = new TFile(fileName, "recreate");
        object -> Write(nameWrite);
        fileOut -> Close();
    }
    else {
        object -> SaveAs(fileName);
    }
}

void ejungwoo::SaveAll(const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false)
{
#ifdef KUNPL_NEED_THIS
    TObjArray* eArray = (TObjArray*) gROOT->FindObject("ejObjArray");
#endif
    TIter next(eArray);
    while (auto obj = next())
        Save(obj,fileType,nameVersion,savePrimitives,simplifyNames);
}

TH2D *ejungwoo::Frame(TGraph *graph, const char* name, const char* title="")
{
    int nbins = graph -> GetN();
    double x1=DBL_MAX, x2=-DBL_MAX, y1=DBL_MAX, y2=-DBL_MAX;
    for (auto iPoint=0; iPoint<nbins; ++iPoint) {
        double x0, y0;
        graph -> GetPoint(iPoint,x0,y0);
        if (x0<x1) x1 = x0;
        if (x0>x2) x2 = x0;
        if (y0<y1) y1 = y0;
        if (y0>y2) y2 = y0;
    }
    x1 = x1 - (x2-x1) * 0.05;
    x2 = x2 + (x2-x1) * 0.05;
    y1 = y1 - (y2-y1) * 0.05;
    y2 = y2 + (y2-y1) * 0.05;
    auto hist = new TH2D(name,title,100,x1,x2,200,y1,y2);
    return hist;
}

TF1 *ejungwoo::FitG(TH1D* hist, double sigmaWidth=1.5, Option_t *opt="RQ0")
{
    auto binMax = hist -> GetMaximumBin();
    auto max = hist -> GetBinContent(binMax);
    auto xMax = hist -> GetXaxis() -> GetBinCenter(binMax);
    auto xErr = hist -> GetStdDev();
    auto func = new TF1(Form("%s_fitg",hist->GetName()),"gaus(0)",xMax-xErr*sigmaWidth,xMax+xErr*sigmaWidth);
    func -> SetParameters(max,xMax,xErr);
    hist -> Fit(func,opt);
    xMax = func -> GetParameter(1);
    xErr = func -> GetParameter(2);
    func -> SetRange(xMax-sigmaWidth*xErr,xMax+sigmaWidth*xErr);
    hist -> Fit(func,opt);
    return func;
}

//using namespace ejungwoo;

#endif
