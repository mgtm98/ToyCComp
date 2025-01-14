int c;
int d;
int *e;

int main()
{
    c = 12;
    d = 18;
    print(c);
    e = &c + 1;
    print(*(&c + 1));
    print(*e);
    return (0);
}