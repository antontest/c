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

/*         struct cfg_t *cfg = create_cfg("t.txt"); */
/*         printf("value: %s\n", cfg->get_value(cfg, "name")); */
/*         cfg->set_value(cfg, "name", ",:=", "antonio"); */
/*         cfg->set_value(cfg, "name", ",:=", NULL); */
/*         cfg->destroy(cfg); */
/*         struct ini_t *ini = create_ini("t.txt"); */
    
/*         printf("value: %s\n", ini->get_value(ini, "info", "name1")); */
/*         ini->set_value(ini, "info2", "name", "antonio"); */
/*         ini->destroy(ini); */

    struct fileio_t *fp = fileio_create();
    if (fp->open(fp, "t.txt", "r+") == NULL) return -1;
    struct filelock_t *lock = filelock_create(fp->get_file_handle(fp));
    
    printf("pid: %d\n", getpid());
    lock->lock_register(lock, F_WRLCK);
    int ret = lock->is_write_lockable(lock, 0, SEEK_SET, 0);
    printf("ret: %d\n", ret);
    if (ret == 1) {
        if (!lock->write_lock_all(lock)) printf("lock succ\n"); 
    } else {
        printf("write lock by pid %d\n", ret);
    }
    ret = lock->is_write_lockable(lock, 0, SEEK_SET, 0);
    printf("ret: %d\n", ret);
    if (lock->is_write_lockable(lock, 1, SEEK_SET, 1)) printf("can write\n");
    else printf("can not write\n");
    printf("sleep start\n");    
    sleep(atoi(agrv[1]));
    printf("sleep end\n");    
    lock->destroy(lock);
    printf("destroy lock succ\n");
    fp->destroy(fp);

    return rt;
}
