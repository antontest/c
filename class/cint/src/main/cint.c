/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cint.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    cint_t *cint = cint_create(10);
    if (!cint) {
        printf("cint_create failed\n");
        return -1;
    }

    cint->add(cint, 1);
    cint->add(cint, 2);
    cint->add(cint, 3);
    cint->add(cint, 4);
    cint->print(cint);
    cint->insert(cint, 2, 5);
    cint->add(cint, 6);
    cint->print(cint);
    cint->remove_at(cint, 0);
/*     printf("%d\n", cint->get_at(cint, 0)); */
/*     printf("%d\n", cint->get_at(cint, 1)); */
/*     printf("%d\n", cint->get_at(cint, 2)); */
    printf("len: %d\n", cint->get_length(cint));
    cint->print(cint);
    printf("first: %d\n", cint->get_first(cint));
    printf("last: %d\n", cint->get_last(cint));
    cint->remove_last(cint);
    cint->print(cint);

    int n = 0;
    while (cint->enumerate(cint, &n)) {
        printf("e: %d\n", n);
    }
    cint->reset_size(cint, 2);
    cint->print(cint);
    
    cint_t *new = cint->clone(cint);
    new->print(new);
    cint->destroy(cint);
    new->destroy(new);

    return rt;
}
