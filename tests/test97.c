void main()
{
    int n;
    int *nn;
    nn = &n;
    *nn = 7;
    print(n);
    *nn = 88;
    print(n * *nn);
}