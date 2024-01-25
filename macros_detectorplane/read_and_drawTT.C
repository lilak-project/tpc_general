#include "ejungwooA.h"

void read_and_drawTT()
{
    int cobo, asad, aget, chan, dumi;
    double x0, x1, x2, z0, z1, z2;
    auto cvs = ejungwoo::Canvas("cvsXZ",100,70,2,1);

    /*
    auto histL = new TH2Poly("hist_TexAT_v2_left", "Strip and Center;x;z",-125,125,180,420);
    histL -> SetStats(0);
    ifstream fileCS("position_channels_center_and_strip.txt");
    while (fileCS >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) histL -> AddBin(x1,z1,x2,z2);
    cvs -> cd(1); histL -> Draw("");

    auto histR = new TH2Poly("hist_TexAT_v2_right","Chain and Center;x;z",-125,125,180,420);
    histR -> SetStats(0);
    ifstream fileCC("position_channels_center_and_chain.txt");
    while (fileCC >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) histR -> AddBin(x1,z1,x2,z2);
    cvs -> cd(2); histR -> Draw("");
    */

    auto histL = new TH2Poly("hist_TexAT_v2_left", "Strip and Center;x;z",-125,125,180,420);
    histL -> SetStats(0);
    ifstream fileCS("position_channels_center_and_strip.txt");
    //ofstream fileZ("position_channels_along_z.txt");
    int countY = 0;
    while (fileCS >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) {
        if (x0>8&&x0<10) {
            //fileZ << z0 << " " << z1 << " " << z2 << endl;
            //cout << z0 << " " << z1 << " " << z2 << endl;
            histL -> AddBin(x1,z1,x2,z2);
            countY++;
        }
    }
    cout << countY << endl;
    cvs -> cd(1); histL -> Draw("");

    auto histR = new TH2Poly("hist_TexAT_v2_right","Chain and Center;x;z",-125,125,180,420);
    histR -> SetStats(0);
    ifstream fileCC("position_channels_center_and_chain.txt");
    //ofstream fileX("position_channels_along_x.txt");
    int countX = 0;
    while (fileCC >> cobo >> asad >> aget >> chan >> x0 >> x1 >> x2 >> z0 >> z1 >> z2) {
        if (z2-z1>50 || (z2-z1<50)&&z0<193) {
            //fileX << x0 << " " << x1 << " " << x2 << endl;
            //cout << x0 << " " << x1 << " " << x2 << endl;
            histR -> AddBin(x1,z1,x2,z2);
            countX++;
        }
    }
    cout << countX << endl;
    cvs -> cd(2); histR -> Draw("");
}
