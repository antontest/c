#include <stdio.h>

#define ALGOR_NUM 1
#define ALGOR_VERSION1 "v1.0"
#define ALGOR_VERSION2 "v1.0"
#define ALGOR_VERSION3 "v1.0"
#define ALGOR_CREATE_TIME1 "2015-07-21"
#define ALGOR_CREATE_TIME2 "2015-07-21"
#define ALGOR_CREATE_TIME3 "2015-07-21"
#define PRINT_VER(n) printf("%s\n", ALGOR_VERSION##n)
#define PRINT_VER_INFO(n) \
            printf("alg-%d: %s @ %s by Antonio An\n", \
                        n, ALGOR_VERSION##n, \
                        ALGOR_CREATE_TIME##n)

int fib1(int n);
long long fib2(int n);
