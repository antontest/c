#include "fib_algor.h"

int fib1(int n)
{
    int sum = 1;
    int sum_bak = 1;
    int pre = -1;

    if (n < 0) return -1;

    while (n-- >= 0)
    {
        sum_bak = sum;
        sum += pre;
        if (sum < 0) return -2;
        pre = sum_bak;
    }

    return sum;
}

long long fib2(int n)
{
    long long sum = 1;
    long long sum_bak = 1;
    long pre = -1;

    if (n < 0) return -1;

    while (n-- >= 0)
    {
        sum_bak = sum;
        sum += pre;
        if (sum < 0) return -2;
        pre = sum_bak;
    }

    return sum;
}

/*
int fib2(int n)
{
    if (n < 0) return -1;
    if (n == 0) return 0;
    if (n == 1 || n == 2) return 1;

    if ((fib2(n - 2) + fib2(n - 1)) < 0) return -2;
    else return fib2(n - 2) + fib2(n - 1);

}
*/
