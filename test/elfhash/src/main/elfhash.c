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
int elfhash(char *key)
{
    unsigned long h = 0;
    unsigned long x = 0;
    
    while (*key) {
        h = (h << 4) + (*key++);
        if ((x = h & 0xF0000000L) != 0) {
            h ^= (x >> 24);
            h &= ~x;
        }
    }
    return h % 100;
}

int time33_hash(char *key)
{
    unsigned long hash = 0;
    while (*key) {
        hash = (hash<<5) + hash + *key++;
    }
    return hash % 100;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int num = 100;
    char s[32] = {0};
    int i = 0;
    
    for (i = 0; i < num; i++) {
        snprintf(s, sizeof(s), "test%i", i);
        printf("%s hash result: %d\n", s, (time33_hash(s) & elfhash(s)) % 100);
    }

    return rt;
}
