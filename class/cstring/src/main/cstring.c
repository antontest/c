/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cstring.h"

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    /*
    char *p = (char *)malloc(10);
    char *pp = (char *)realloc(p, 100);
    if (!pp) free(p);
    else free(pp);
    return 0;
    */

    cstring *s = create_cstring(6);
    s->set(s, "hi, %s", "ad");
    printf("%s\n", s->get(s));
    printf("%s\n", s->add(s, "ssss"));
    printf("%s\n", s->toupper(s));
    s->resize(s, 10);
    printf("%s\n", s->set(s, "hi, %s", "world!"));
    printf("%s\n", s->all_trim(s));
    printf("len: %d\n", s->length(s));
    printf("%s\n", s->offset(s, 4, 0));
    s->set(s, "123s");
    printf("%d\n", s->toint(s));
    s->destroy(s);

    return rt;
}
