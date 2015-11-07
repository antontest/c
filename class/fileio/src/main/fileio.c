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

/*     struct fileio_t *fp = create_fileio("t.txt", "r+"); */
/*     struct fileio_t *fp = create_fileio(NULL, NULL); */
/*     fp->open(fp, "t.txt", "r+"); */
/*     printf("%s", fp->read(fp)); */
/*     printf("%s", fp->read(fp)); */
/*     fp->vwrite(fp, "Hello World! --- %d\n", 100); */
/*     fp->write(fp, "Hello World! --- 100\n"); */
/*     fp->close(fp); */

    struct cfg_t *cfg = create_cfg("t.txt");
    printf("value: %s\n", cfg->get_value(cfg, "name", ",:="));
    //cfg->set_value(cfg, "name", ",:=", "antonio");
    cfg->set_value(cfg, "name", ",:=", NULL);
    cfg->destroy(cfg);

    return rt;
}
