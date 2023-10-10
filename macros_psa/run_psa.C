void run_psa()
{
    auto run = new LKRun("texat",801,"psa");
    run -> AddDetector(new TexAT2());
    run -> AddInputList("list_801_10");
    run -> AddPar("config.mac");
    run -> Add(new TTPulseAnalysisTask());

    run -> Init();
    run -> Run();
}
