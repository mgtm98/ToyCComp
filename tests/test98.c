void main()
{
    int **nnn;
    int *nn;
    int n;
    nn = &n;
    nnn = &nn;
    n = 7;
    print(*nn);
    print(**nnn);
    n = 8;
    print(*nn);
    print(**nnn);
}