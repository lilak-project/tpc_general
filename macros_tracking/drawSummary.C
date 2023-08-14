#ifndef USING_EJUNGWOO
TCanvas* e_cvs(const char* name="") { return (new TCanvas(name)); }
void e_add(TObject *,const char* opt="") {}
void e_save_all(TObject *) {}
#endif

#include "setChannels.h"

void drawSummary()
{
    int fnw = 40;
    double fw1 = 20;
    double fw2 = 60;
    //e_batch();

    //for (auto iType : fAllTypes)
    for (auto iType : {7})
    {
        int nw = fnw;
        double w1 = fw1;
        double w2 = fw2;
        if (iType==7) { nw = 350; w1 = 0; w2 = 350; }
        auto file = new TFile(Form("data100/summary_%s.root",fTypeNames[iType]));
        auto tree = (TTree *) file -> Get("pulse");

        auto cvsSummary = e_cvs(Form("cvsSummary_%s",fTypeNames[iType]),"",2800,2000,3,2);
        {
            auto cvs_width = cvsSummary -> cd(1);
            auto hist_width = new TH1D("hist_width",Form("%s;width;",fTypeNames[iType]),nw,w1,w2);
            tree -> Draw("width>>hist_width","isCollected");

            auto cvs_width_height = cvsSummary -> cd(2);
            auto hist_width_height = new TH2D("hist_width_height",Form("%s;width;height",fTypeNames[iType]),nw,w1,w2,100,500,4000);
            tree -> Draw("height:width>>hist_width_height","isCollected","colz");

            auto cvs_width_asad = cvsSummary -> cd(3);
            auto hist_width_asad = new TH2D("hist_width_asad",Form("%s;width;asad",fTypeNames[iType]),nw,w1,w2,4,0,4);
            tree -> Draw("asad:width>>hist_width_asad","isCollected","colz");

            auto cvs_width_aget = cvsSummary -> cd(4);
            auto hist_width_aget = new TH2D("hist_width_aget",Form("%s;width;aget",fTypeNames[iType]),nw,w1,w2,4,0,4);
            tree -> Draw("aget:width>>hist_width_aget","isCollected","colz");

            auto cvs_width_channel = cvsSummary -> cd(5);
            auto hist_width_channel = new TH2D("hist_width_channel",Form("%s;width;channel",fTypeNames[iType]),nw,w1,w2,70,0,70);
            hist_width_channel -> SetStats(0);
            tree -> Draw("channel:width>>hist_width_channel","isCollected","colz");
        }

        //auto cvsAll_channelWidth = e_cvs(Form("cvs_channelWidth_%s",fTypeNames[iType])); 
        //for (auto channel=0; channel<4; ++channel)
        //{
        //    auto cvs_channelWidth = cvsAll_channelWidth -> cd(channel+1);
        //    auto hist_channelWidth = new TH1D(Form("hist_channelWidth_%d",channel),Form("Channel %d;width;",channel),nw,w1,w2);
        //    //hist_channelWidth -> SetStats(0);
        //    tree -> Draw(Form("width>>hist_channelWidth_%d",channel),Form("isCollected&&channel==%d",channel),"");
        //}
    }

    e_save_all();
}
