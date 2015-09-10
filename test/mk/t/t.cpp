#include <iostream>
using namespace std;
#include <stdio.h>

void echo(int *num)
{
    printf("%d\n", *num);
    return;
}

int main(int agrc, char *agrv[])
{
    cout << "hello" << endl;
    int num = 2;
    echo(&(++num));

    return 0;
}
