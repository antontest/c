/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
#define INIT(this, ...) { (this) = malloc(sizeof(*(this))); \
    *(this) = (typeof(*(this))){__VA_ARGS__};}

#define METHOD(iface, name, ret, this, ...) \
    static ret name(union {iface *_public; this;} \
    __attribute__((transparent_union)), ##__VA_ARGS__); \
    static typeof(name) *_##name = (typeof(name)*)name; \
    static ret name(this, ##__VA_ARGS__)

typedef struct test test;
struct test {
    void (*print)(test *this, int num);
} ;

typedef struct private_test {
    test public;
    int i;
} private_test;


METHOD(test, print, void, test *this, int num)
{
    printf("num: %d\n", num);
}

test * test_create()
{
    private_test *this;
    INIT(this, 
        .public = {
        .print = _print,
        },);

    return &this->public;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    test *this;
    this = test_create();
    this->print(this, 2);
    free(this);

    return rt;
}
