#include "ejungwooA.h"
#include "setChannels.h"

void drawExtractSummary()
{
    int fnw = 40;
    double fw1 = 20;
    double fw2 = 60;
    //e_batch();

    //TCut cut0 = "isCollected&&tbAtMax>200&&tbAtMax<210";
    TCut cut0 = "isCollected";

    SetType(0);
    for (auto iType : fSelTypes)
    {
        int nw = fnw;
        double w1 = fw1;
        double w2 = fw2;
        //if (iType==7) { nw = 350; w1 = 0; w2 = 350; }
        auto file = new TFile(Form("data100/summary_%s.root",fTypeNames[iType]));
        auto tree = (TTree *) file -> Get("pulse");

        auto cvsSummary = e_cvs_full(Form("cvsSummary_%s",fTypeNames[iType]));
        cvsSummary -> Divide(3,2);
        int iPad = 1;

        if (1) {
            auto cvs_width = cvsSummary -> cd(iPad++);
            auto hist_width = new TH1D(Form("hist_width_%s",fTypeNames[iType]),Form("%s;width;",fTypeNames[iType]),nw,w1,w2);
            tree -> Draw(Form("width>>hist_width_%s",fTypeNames[iType]),cut0);

            auto cvs_width_height = cvsSummary -> cd(iPad++);
            auto hist_width_height = new TH2D(Form("hist_width_height_%s",fTypeNames[iType]),Form("%s;width;height",fTypeNames[iType]),nw,w1,w2,100,500,4000);
            tree -> Draw(Form("height:width>>hist_width_height_%s",fTypeNames[iType]),cut0,"colz");

            auto cvs_width_asad = cvsSummary -> cd(iPad++);
            auto hist_width_asad = new TH2D(Form("hist_width_asad_%s",fTypeNames[iType]),Form("%s;width;asad",fTypeNames[iType]),nw,w1,w2,4,0,4);
            tree -> Draw(Form("asad:width>>hist_width_asad_%s",fTypeNames[iType]),cut0,"colz");

            auto cvs_width_aget = cvsSummary -> cd(iPad++);
            auto hist_width_aget = new TH2D(Form("hist_width_aget_%s",fTypeNames[iType]),Form("%s;width;aget",fTypeNames[iType]),nw,w1,w2,4,0,4);
            tree -> Draw(Form("aget:width>>hist_width_aget_%s",fTypeNames[iType]),cut0,"colz");
        }

        if (1) {
            auto cvs_width_channel = cvsSummary -> cd(iPad++);
            auto hist_width_channel = new TH2D(Form("hist_width_channel_%s",fTypeNames[iType]),Form("%s;width;channel",fTypeNames[iType]),nw,w1,w2,70,0,70);
            hist_width_channel -> SetStats(0);
            tree -> Draw(Form("channel:width>>hist_width_channel_%s",fTypeNames[iType]),cut0,"colz");
        }

        if (1) {
            auto cvs_width_tbAtMax = cvsSummary -> cd(iPad++);
            auto hist_width_tbAtMax = new TH2D(Form("hist_width_tbAtMax_%s",fTypeNames[iType]),Form("%s;width;tb",fTypeNames[iType]),nw,w1,w2,350,0,350);
            hist_width_tbAtMax -> SetStats(0);
            tree -> Draw(Form("tbAtMax:width>>hist_width_tbAtMax_%s",fTypeNames[iType]),"","colz");
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
