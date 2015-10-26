/**
 * usual head files
 */
#include <dir.h>
#include <getopt.h>

/*********************************************************
 ****************    Variable Defination    **************
 *********************************************************/
static char perm[4] = {0};
static char path[256] = {0};
static int perm_flag = 0;
static int size_flag = 0;
static int basename_flag = 0;
static int dirname_flag = 0;
static int create_flag =0;

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
static void print_usage();
static int parser_args(int agrc, char *agrv[]);

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    char buf[128] = {0};

    /**
     * Get paramters from the command line
     */
    if (parser_args(agrc, agrv) < 0) {
        print_usage();
        rt = -1;
        goto error;
    };

    if (perm_flag) {
        rt = detect_permission(path, perm);
        switch (rt) {
            case 0:
                printf("\033[1;36m%s has permission %s? : \033[1;35mno\n\033[0m", path, perm);
                break;
            case 1:
                printf("\033[1;36m%s has permission %s? : \033[1;35myes\n\033[0m", path, perm);
                break;
            default:
                printf("\033[1;36m%s has permission %s? : \033[1;31mfailed\n\033[0m", path, perm);
                break;
        }
    }

    if (size_flag) {
        rt = get_file_size(path);
        if (rt > 0) printf("\033[1;34msize of %s is: \033[1;35m%d bytes(%d Kb)\n\033[0m", path, rt, rt / 1024);
        else printf("\033[1;31m%s get size failed\n\033[0m", path);
    }

    if (basename_flag) {
        if (basename(path, buf, sizeof(buf)) != NULL)
            printf("\033[1;34mbasename of %s is: \033[1;35m%s\n\033[0m", path, buf);
        else printf("\033[1;31m%s get basename failed\n\033[0m", path);
    }

    if (dirname_flag) {
        if (dirname(path, buf, sizeof(buf)) != NULL)
            printf("\033[1;34mdirname of %s is: \033[1;35m%s\n\033[0m", path, buf);
        else printf("\033[1;31m%s get dirname failed\n\033[0m", path);
    }

    if (create_flag) {
        if (!make_dir(path, 0755)) 
            printf("\033[1;35mcreate dir %s success\n\033[0m", path);
        else printf("\033[1;31mcreate dir %s failed\n\033[0m", path);
    }

error:
    /**
     * error handling
     */

    return rt;
}

/**
 * @brief print usage of the pragram 
 */
static void print_usage() 
{
    printf("\033[0;31m/********************Program Usage***********************/\033[0m\n");  
    printf("\033[0;31mFile  : \033[0;32m%s\033[0m\n", __FILE__);  
    printf("\033[0;31mVers  : \033[0;32m1.0.0\033[0m\n");
    printf("\033[0;31mTime  : \033[0;32m2015.06.04\033[0m\n");
    printf("\033[0;31mBrief : \033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m[-p | --permission] <file_permission> path\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m[-s | --size]       path\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m[-b | --basename]   path\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m[-d | --dirname]    path\033[0m\n");  
    printf("\033[0;31mParam : \033[0m\n");
    printf("\033[0;31m        \033[0;32m-p --permission     detect file whether has the permission\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-s --size           get size of file or directory\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-b --basename       get basename of path\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-c --create         create dir\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-d --dirname        get dirname of path\033[0m\n");  
    printf("\033[0;31m/********************Program Usage***********************/\033[0m\n");  
} 

/**
 * @brief Get parameters from the command line 
 *
 * @param agrc   [in] the count of paramters
 * @param agrv[] [in] parameters array
 *
 * @return 0, if succ
 *        -1, if failed
 */
static int parser_args(int agrc, char *agrv[])
{
    int opt = 0;
    const char *optstr = "hb:c:d:p:s";
    struct option opts[] = {
        { "help"    , no_argument      , 0, 'h'},
        { "perm"    , required_argument, 0, 'p'},
        { "create"  , required_argument, 0, 'c'},
        { "size"    , no_argument      , 0, 's'},
        { "basename", no_argument      , 0, 'b'},
        { "dirname" , no_argument      , 0, 'd'},
        {     0     ,       0          , 0,  0 }
    };

    if (agrc < 2) return -1;

    while ( ( opt = getopt_long( agrc, agrv, optstr, opts, NULL ) ) != -1 ) {
        switch(opt) {
            case 'h':
                return -1;
                break;
            case 'b':
                if (optarg != NULL) {
                    basename_flag = 1;
                    strcpy(path, optarg);
                }
                break;
            case 'c':
                if (optarg != NULL) {
                    create_flag = 1;
                    strncpy(path, optarg, sizeof(path));
                }
                break;
            case 'd':
                if (optarg != NULL) {
                    dirname_flag = 1;
                    strcpy(path, optarg);
                }
                break;
            case 'p':
                if (optarg != NULL) {
                    perm_flag = 1;
                    strcpy(perm, optarg);
                }
                break;
            case 's':
                size_flag = 1;
                break;
            case '?':
            default:
                return -1;
        }
    }

    if (agrv[optind] != NULL) strcpy(path, agrv[optind]);

    return 0;
}
