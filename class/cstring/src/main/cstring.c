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

    cstring *s = create_cstring(7);
    s->set(s, " hi, %s", "ad");
    printf("%s\n", s->get(s));
    //printf("all trim: %s\n", s->all_trim(s));
    //printf("right trim: %s\n", s->right_trim(s));
    printf("left trim: %s\n", s->left_trim(s));
    printf("%s\n", s->add(s, "ssss"));
    printf("%s\n", s->toupper(s));
    printf("%s\n", s->tolower(s));
    s->resize(s, 15);
    printf("%s\n", s->set(s, "hi, %s", "world!"));
    printf("%s\n", s->all_trim(s));
    printf("len: %d\n", s->length(s));
    printf("%s\n", s->mid(s, 4, 0));
    printf("del: %s\n", s->delete(s, 1, 1));
    printf("insert: %s\n", s->insert(s, 1, "%s %d ", "hi", 111));
    printf("right: %s\n", s->right(s, 6));
    printf("left: %s\n", s->left(s, 3));
    printf("len: %d\n", s->length(s));
    s->set(s, "123s");
    printf("%d\n", s->toint(s));
    s->destroy(s);

    return rt;
}
