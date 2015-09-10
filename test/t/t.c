#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "head.h"

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
struct mod_state  mod_st_tbl[2] = {
    {1, 0},
    {2, 1}
};


/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/

void update_tbl(int mod_id, int state)
{
    int size = sizeof(mod_st_tbl) / sizeof(mod_st_tbl[0]);
    int i = 0;

    for (i = 0; i < size; i++)
    {
        if (mod_st_tbl[i].mod_id == mod_id)
        {
            mod_st_tbl[i].app_state = state;
            break;
        }
    }

    return;
}
