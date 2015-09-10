#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <getopt.h>
#include <time.h>

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/
#define DEBUG_ERROR(...) \
    do { \
        fprintf(stderr, "\033[1;35m[ Function %s ] [ line %d ] \033[0m", \
                __func__, __LINE__); \
        fprintf(stderr, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0);

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

void member_set(void *id, int idlen, void *value, int vlen)
{
    int len = (idlen < vlen) ? idlen : vlen;
    
    if (id == NULL || value == NULL) return;
    
    memset(id, 0, idlen);
    memcpy(id, value, len);

    return;
}

#define SET(ID, BUF, LEN) member_set(&(ID), sizeof(ID), BUF, LEN)
#define par(i) (par_##i)
#define STR(x) (#x)

struct pre
{
    int __dirty;
    void *par;
};

struct par 
{
    int __dirty;
    void  *par;
    int id;
    int age;
    struct par_chl {
        int id;
        int age;
    }par_chl;
    struct par_chl1 {
        int __dirty;
        void  *par;
        int id;
        int age;
        struct par_chl1_chl {
            int __dirty;
            void  *par;
            int id;
            int age;
        }par_chl1_chl;

        struct par_chl1_chl1 {
            int __dirty;
            void  *par;
            int id;
            int age;
        }par_chl1_chl1;

    }par_chl1;
}par;

void set_dirty(void *id)
{
    int rt = 1;
    void *par = NULL;
    struct pre *p = NULL;
    if (id == NULL) return;

    par = id;
    while (par != NULL)
    {
        p = (struct pre *)par;
        memcpy(p, &rt, sizeof(rt));
        par =  p->par;
    }
    
    return;
}

struct stu
{
    int id;
    char name[20];
} ;

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 10; /* return value of function main */

    struct stu st = {
        .id = 1,
        .name = "anton"
    };

    printf("stu id: %d\n", st.id);
    printf("stu name: %s\n", st.name);
    /*
    unsigned long int addr = 0;

    addr = (unsigned long int)&par.__dirty;
    memcpy((void *)addr, &rt, sizeof(rt));
    printf("par.__dirty = %d\n", par.__dirty);
    */

    par.par = NULL;

    
    par.par_chl1.par = (void *)&par;

    par.par_chl1.par_chl1_chl.par = (void *)&par.par_chl1;
    
    par.par_chl1.par_chl1_chl1.par = (void *)&par.par_chl1;

    //memcpy(par.par_chl1.par.addr, &rt, sizeof(rt));
    //printf("par.__dirty = %d\n", par.__dirty);
    set_dirty(par.par_chl1.par_chl1_chl.par);
    //set_dirty(&par.par_chl1.par);
    //set_dirty(par.par);
    printf("par.__dirty = %d\n", par.__dirty);
    printf("par.par_chl1.__dirty = %d\n", par.par_chl1.__dirty);

    return rt;
}
