#include <stdio.h>
#include <stdlib.h>
#include "head.h"

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
extern struct mod_state mod_st_tbl[2];

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int i = 0;

    update_tbl(1, 3);
    for (i = 0; i < (sizeof(mod_st_tbl) / sizeof(mod_st_tbl[0])); i++)
        printf("id: %d, state: %d\n", mod_st_tbl[i].mod_id, mod_st_tbl[i].app_state);
    

    return rt;
}
