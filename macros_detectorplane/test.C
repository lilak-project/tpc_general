void test()
{
    int cobo, asad, aget, chan, dumi;
    double x0, x1, x2, z0, z1, z2;
    auto hist1 = new TH2Poly("hist_TexAT_v2_left", "Strip and Center;x (strip);z (beam-axis)",-125,125,180,420);
    hist1 -> GetXaxis() -> SetTitleOffset(1.20);
    hist1 -> SetStats(0);
    ifstream fileCS("position_channels_center_and_strip.txt");
    while (fileCS >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) hist1 -> AddBin(x1,z1,x2,z2);

    auto hist2 = new TH1D("hist_channel_buffer","channel buffer;time-bucket;charge",360,0,360);
    hist2 -> SetStats(0);
    hist2 -> GetXaxis() -> SetLabelSize(0.07);
    hist2 -> GetYaxis() -> SetLabelSize(0.07);
    hist2 -> GetXaxis() -> SetTitleSize(0.07);
    hist2 -> GetYaxis() -> SetTitleSize(0.07);
    hist2 -> GetXaxis() -> SetTitleOffset(1.20);
    hist2 -> GetYaxis() -> SetTitleOffset(0.68);

    auto cvs = new TCanvas("cvs","",550,700);
    auto pad1 = new TPad("pad1","",0,230./700,1,1);
    pad1 -> SetMargin(0.12,0.15,0.1,0.1);
    pad1 -> SetNumber(1);
    pad1 -> Draw();
    auto pad2 = new TPad("pad2","",0,0,1,230./700);
    pad2 -> SetMargin(0.12,0.1,0.20,0.12);
    pad2 -> SetNumber(2);
    pad2 -> Draw();

    cvs -> Modified();

    cvs -> cd(1);
    hist1 -> Draw("colz");

    cvs -> cd(2);
    hist2 -> Draw();
}
