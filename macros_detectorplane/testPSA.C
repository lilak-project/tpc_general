void testPSA()
{
    auto file = new TFile("selected_channel.root");
    auto channel = (MMChannel*) file -> Get("channel");
    auto par = (LKParameterContainer*) file -> Get("ParameterContainer");
    auto buffer = channel -> GetWaveformY();

    auto tt = new TexAT2(par);
    auto ana = tt -> GetChannelAnalyzer(channel);
    ana -> Analyze(buffer);
    ana -> Draw();
}
