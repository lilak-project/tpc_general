void run_TTPulseExtractionTask()
{
    auto run = new LKRun("texat",0,"ttpulseextractiontask");
    run -> AddDetector(new TexAT2());
    run -> AddInputList("list_conv");
    run -> AddPar("config.mac");
    run -> Add(new TTPulseExtractionTask());

    run -> Init();
    run -> Run(0,100);
}
