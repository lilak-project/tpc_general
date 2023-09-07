

















void ha()
{
    double x1 = -120;
    double x2 = 120;
    int nx = 12;
    double dx = (x2-x1)/nx;
    auto hist = new TH1D("hist","",nx,x1,x2);
    for (double x=-120; x<120; x+=5)
    {
        auto a = floor((x-x1)/dx)+1;
        auto b = hist -> FindBin(x);
        cout << x << " " << a << " " << b << endl;
    }
}
