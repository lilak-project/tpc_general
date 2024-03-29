void next(int i=-2) { LKRun::GetRun() -> RunSelectedEvent("EventHeader[0].IsGoodEvent()"); }
void write(int i=-2) {
    auto det = LKRun::GetRun() -> GetDetector();
    auto mm = (TTMicromegas*) det -> GetDetectorPlane();
    mm -> WriteCurrentChannel();
}

void drawDetectorPlane()
{
    auto run = new LKRun();
    run -> AddInputFile("~/data/texat/reco/texat_0801.all.root");
    run -> AddFriend("~/data/texat/reco/texat_0801.ht.root");
    run -> AddPar("config_eve.mac");
    auto tt = new TexAT2();
    run -> AddDetector(tt);
    run -> Add(new LKEveTask);
    run -> Init();

    next();
}
