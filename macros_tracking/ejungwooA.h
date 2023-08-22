#ifndef __EJUNGWOO_A__
#define __EJUNGWOO_A__

#ifndef KUNPL_NEED_THIS
    THttpServer* eServer = nullptr;
    TObjArray* eArray = nullptr;
#endif

//void e_batch();
//void e_add(TObject* object, const char *subfolder="");
//TCanvas *e_cvs(const char* name="", const char* title="", int wx=800, int wy=680, int dx=1, int dy=1);
//void e_save(TObject *object, const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false);
//void e_save_all(const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false);
//TH2D *e_hist(TGraph *graph, const char* name, const char* title="");
//TF1 *e_fit_gaus(TH1D* hist, double sigmaWidth, Option_t *opt="RQ0")

void e_batch()
{
    gROOT -> SetBatch(1);
}

void e_add(TObject* object, const char *subfolder="")
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

TCanvas *e_cvs(const char* name="", const char* title="", int wx=800, int wy=680, int dx=1, int dy=1)
{
    if (TString(title).IsNull()) title = name;
    auto cvs0 = new TCanvas(name, title, wx, wy);
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
    e_add(cvs0);
    return cvs0;
}

void e_save(TObject *object, const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false)
{
    if (TString(nameVersion).IsNull()) {
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

    if (TString(fileType).IsNull())  {
        //e_save(object,"pdf");
        //e_save(object,"eps");
        e_save(object,"png");
        e_save(object,"root");
        return;
    }
    else if (TString(fileType)=="pdf")  { pathToData = Form("%s/pdf",pathToVersion); }
    else if (TString(fileType)=="png")  { pathToData = Form("%s/png",pathToVersion); }
    else if (TString(fileType)=="eps")  { pathToData = Form("%s/eps",pathToVersion); }
    else if (TString(fileType)=="root") { pathToData = Form("%s/roo",pathToVersion); }
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
    if (TString(fileType)=="root")  {
        auto fileOut = new TFile(fileName, "recreate");
        object -> Write(nameWrite);
        fileOut -> Close();
    }
    else {
        object -> SaveAs(fileName);
    }
}

void e_save_all(const char* fileType="", const char* nameVersion="", bool savePrimitives=false, bool simplifyNames=false)
{
#ifdef KUNPL_NEED_THIS
    TObjArray* eArray = (TObjArray*) gROOT->FindObject("ejObjArray");
#endif
    TIter next(eArray);
    while (auto obj = next())
        e_save(obj,fileType,nameVersion,savePrimitives,simplifyNames);
}

TH2D *e_hist(TGraph *graph, const char* name, const char* title="")
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

TF1 *e_fit_gaus(TH1D* hist, double sigmaWidth=1.5, Option_t *opt="RQ0")
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

#endif
