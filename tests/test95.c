int main()
{
    char c;
    char *st;
    for (st = "Hello world"; 1; st = st + 1)
    {
        print_char(*st);
    }
    int a = 0;
    print(a);
}