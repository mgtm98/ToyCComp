int main()
{
    char a;
    char *b;
    char c;

    int d;
    int *e;
    int f;

    a = 18;
    print(a);
    b = &a;
    c = *b;
    print(c);

    d = 12;
    print(d);
    e = &d;
    f = *e;
    print(f);
    return (0);
}