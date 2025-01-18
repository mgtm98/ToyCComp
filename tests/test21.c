int main()
{
    char c;
    char *st;
    c = '\n';
    print_char(c);

    for (st = "Hello world"; *st != 0; st = st + 1)
    {
        print_char(*st);
    }
    print_char(c);
    print_ln("Hello again");
    return (0);
}