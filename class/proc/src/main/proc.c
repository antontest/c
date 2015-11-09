/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <proc.h>

/*********************************************************
 *****************    Variable Defination    *************
 *********************************************************/
static char name[64] = {0};
static pid_t pid = -1;
static int cmd_flag = 0;
static int pid_by_name_flag = 0;
static int name_by_pid_flag = 0;
static int par_flag = 0;
static int unique_flag = 0;
static int exec_path_flag = 0;

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
static void print_usage();
static int parser_args(int agrc, char *agrv[]);
int get_pids_by_name(const char *proc_name, char *pids, int size);

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    char buf[512] = {0};
    pid_t ppid = -1;

    /*
    struct ipc_t *ipc = create_ipc();
    if (atoi(agrv[1]) == 0) {
        //if (ipc->mkfifo(ipc, "./ipc", O_RDONLY | O_NONBLOCK) < 0) printf("mkfifo failed\n");
        if (ipc->mkfifo(ipc, "./ipc", O_RDONLY | O_NONBLOCK) < 0) printf("mkfifo failed\n");
        while (ipc->read(ipc, buf, sizeof(buf)) == 0);
        //ipc->read(ipc, buf, sizeof(buf));
        printf("buf: %s\n", buf);
        ipc->close(ipc);
    }
    else {
        if (ipc->mkfifo(ipc, "./ipc", O_WRONLY) < 0) printf("mkfifo failed\n");
        ipc->write(ipc, "fifo succ", strlen("fifo succ\n") + 1);
    }
    ipc->destroy(ipc);
    */

    struct ipc_t *ipc = create_ipc();
    ipc->mkshm(ipc, 1234, 100);
    if (atoi(agrv[1]) == 0) {
        int num;
        while (ipc->read(ipc, &num, sizeof(num)) == 0) ;
        printf("buf: %d\n", num);
        ipc->close(ipc);
    } else {
        int num = 100;
        ipc->write(ipc, &num, sizeof(num));
    }
    ipc->destroy(ipc);
    return 0;
    /**
     * Get paramters from the command line
     */
    if (parser_args(agrc, agrv) < 0) {
        print_usage();
        rt = -1;
        goto error;
    };

    if (cmd_flag) {
        if (pid <= 0) {
            char buff[256] = {0};
            rt = get_pids_by_name(name, buff, sizeof(buff));
            if (rt > 0) {
                char *str = NULL, *save_str = NULL;
                for (str = buff; ; str = NULL) {
                    save_str = strtok(str, " ");
                    if (save_str == NULL) break;
                    if ((pid = atoi(save_str)) <= 0) continue;
                    if (get_cmdline(pid, buf, sizeof(buf)))
                        printf("\033[0;35m%s\n\033[0m", buf);
                }
            }
        } else {
            if (get_cmdline(pid, buf, sizeof(buf)))
                printf("\033[0;35m%s\n\033[0m", buf);
        }
    }

    if (pid_by_name_flag) {
        rt = get_pids_by_name(name, buf, sizeof(buf));
        if (rt > 0)
            printf("\033[0;35m%s\n\033[0m", buf);
        rt = 0;
    }

    if (name_by_pid_flag) {
        if (!get_proc_name(pid, buf, sizeof(buf)))
            printf("\033[0;35m%s\n\033[0m", buf);
    }

    if (par_flag) {
        ppid = get_ppid(pid);
        if (ppid > 0) {
            if (!get_proc_name(ppid, buf, sizeof(buf)))
                printf("\033[0;35m%s\n\033[0m", buf);
        }
    }

    if (unique_flag) {
        rt = check_proc_unique(name);
        printf("\033[0;35m%d\n\033[0m", rt);
    }

    if (exec_path_flag) {
        if (!get_exec_path(pid, buf, sizeof(buf)))
            printf("\033[0;36mpath of pid %d is: \033[0;35m%s\n\033[0m", pid, buf);
        else 
            printf("\033[0;31mget path of pid %d failed \n\033[0m", pid);
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
    printf("\033[0;31mProc  : \033[0;32mproc\033[0m\n");  
    printf("\033[0;31mVers  : \033[0;32m1.0.0\033[0m\n");
    printf("\033[0;31mTime  : \033[0;32m2015.06.04\033[0m\n");
    printf("\033[0;31mBrief : \033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m[-i|--pid] <process_name> [-n|--name] <process_id>\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m[-c|--cmdline] <process_id> [-u|--unique] <process_name>\033[0m\n");  
    printf("\033[0;31mParam : \033[0m\n");
    printf("\033[0;31m        \033[0;32m-i --pid        get id of process by name\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-n --name       get name of process by process id\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-c --cmdline    get process cmdline by pid\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-u --unique     check whether is unique of the process\033[0m\n");  
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
    const char *optstr = "hi:nc:pu:e";
    struct option opts[] = {
        { "help"      , no_argument      , 0, 'h'},
        { "pid"       , required_argument, 0, 'i'},
        { "name"      , no_argument      , 0, 'n'},
        { "parent"    , no_argument      , 0, 'p'},
        { "cmdline"   , no_argument      , 0, 'c'},
        { "unique"    , no_argument      , 0, 'u'},
        { "exec_path" , no_argument      , 0, 'e'},
        {     0       ,       0          , 0,  0 }
    };

    if (agrc < 2) return -1;

    while ( ( opt = getopt_long( agrc, agrv, optstr, opts, NULL ) ) != -1 ) {
        switch(opt) {
            case 'h':
                return -1;
                break;
            case 'i':
                if (optarg != 0) {
                    pid_by_name_flag = 1;
                    strcpy(name, optarg);
                }
                break;
            case 'n':
                name_by_pid_flag = 1;
                break;
            case 'c':
                cmd_flag = 1;
                if (optarg != NULL) {
                    if ((pid = atoi(optarg)) <= 0) { 
                        strcpy(name, optarg);
                    }
                }
                break;
            case 'e':
                exec_path_flag = 1;
                break;
            case 'p':
                par_flag = 1;
                break;
            case 'u':
                if (optarg != 0) {
                    unique_flag = 1;
                    strcpy(name, optarg);
                }
                break;
            case 'k':
                printf("Keyword is %s.\n", optarg);
                break;
            case '?':
            default:
                return -1;
        }
    }

    if (agrv[optind] != NULL) pid = atoi(agrv[optind]);

    return 0;
}

/**
 * @brief get process id by process name
 *
 * @param name [in] process name
 *
 * @return pid , if succ; -1, if failed.
 */
int get_pids_by_name(const char *proc_name, char *pids, int size)
{
    struct dirent *dir = NULL;
    DIR *dirp = NULL;
    int cnt = 0;
    FILE *fp = NULL;
    char name[64] = {0};
    char path[128] = {0};

    if ((dirp = opendir("/proc/")) == NULL) return -1;

    strcpy(pids, "\0");
    while ((dir = readdir(dirp)) != NULL)
    {
        if (dir->d_name && (atoi(dir->d_name) > 0))
        {
            sprintf(path, "/proc/%s/status", dir->d_name);
            if ((fp = fopen(path, "r")) == NULL) continue;
            if (fscanf(fp, "Name:%s", name) == -1) 
                name[0] = '\0';
            if (fp != NULL) fclose(fp);
            if (strcmp(name, proc_name)) continue;
            strcat(pids, dir->d_name);
            strcat(pids, " ");
            cnt++;
        }
    }

    if (dirp != NULL) closedir(dirp);

    return cnt;
}

