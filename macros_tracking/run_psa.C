void run_psa()
{
    auto run = new LKRun("texat",0,"psa");
    run -> AddDetector(new TexAT2());
    run -> AddInputList("list_conv");
    run -> AddPar("config.mac");
    run -> Add(new TTPulseAnalysisTask());

    run -> Init();
    run -> Run();
}
