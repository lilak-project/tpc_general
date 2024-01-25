#include "httask.cpp"

void ha()
{
    gStyle -> SetOptStat(0);

    auto run = new LKRun("texat",0,"drawhit");
    run -> AddInputFile("/home/ejungwoo/data/texat/reco/texat_0801.all.root");
    run -> Add(new httask);
    run -> Init();
    run -> RunSelectedEvent("EventHeader[0].IsGoodEvent()");
    auto branchA = run -> GetBranchA("EventHeader");
    ((TTEventHeader*) branchA -> At(0)) -> Print();
}
