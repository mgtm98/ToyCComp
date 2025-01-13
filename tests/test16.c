int d, f;
int *e;

int main()
{
    int a, b, c;
    b = 3;
    c = 5;
    a = b + c * 10;
    print(a);

    d = 12;
    print(d);
    e = &d;
    f = *e;
    print(f);
    return (0);
}