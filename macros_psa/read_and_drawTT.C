#include "ejungwooA.h"

void read_and_drawTT()
{
    int cobo, asad, aget, chan, dumi;
    double x0, x1, x2, z0, z1, z2;
    auto histL = new TH2Poly("hist_TexAT_v2_left", "Strip and Center;x;z",-125,125,180,420);
    ifstream fileCS("hist_channels_center_and_strip.txt");
    while (fileCS >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) histL -> AddBin(x1,z1,x2,z2);
    auto histR = new TH2Poly("hist_TexAT_v2_right","Chain and Center;x;z",-125,125,180,420);
    ifstream fileCC("hist_channels_center_and_chain.txt");
    while (fileCC >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) histR -> AddBin(x1,z1,x2,z2);

    auto cvs = ejungwoo::Canvas("cvsXZ",100,70,2,1);
    cvs -> cd(1); histL -> Draw("");
    cvs -> cd(2); histR -> Draw("");
}
