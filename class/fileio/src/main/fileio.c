/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fileio.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    struct fileio_t *fp = create_fileio("t.txt", "r");
    printf("%s", fp->read(fp));
    printf("%s", fp->read(fp));
    fp->close(fp);

    return rt;
}
