int main()
{
    int a[8];
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;
    a[4] = 4;
    print(a[0]);
    print(a[1]);
    print(a[0 + a[a[a[a[1]]]] + a[2]]);
    return (0);
}