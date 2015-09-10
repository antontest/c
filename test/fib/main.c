#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "fib_log.h"
#include "fib_algor.h"


static int  algor_num = 0;
static char log_path[128] = {0};
static int t_flag = 0;


/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
static void print_usage();
static int parser_args(int agrc, char *agrv[]);

long long fib(int algor_num, int n)
{
    int rt = 0;

    switch (algor_num) 
    {
        case 0:
            rt = fib1(n);
            break;
        case 1:
            rt = fib2(n);
            break;
        default:
            break;
    }

    return rt;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    long long rt = 0; /* return value of function main */
    int n = -3;
    char buf[128] = {0};

    /**
     * Get paramters from the command line
     */
    if (parser_args(agrc, agrv) < 0) {
        print_usage();
        rt = -1;
        goto error;
    };

    if (!t_flag) return 0;

    if (t_flag && strlen(log_path) < 1) 
    {
        print_usage();
        goto error;
    }


    /**
     * log init
     */
    fib_log_init(log_path);

    while ((rt = fib(algor_num, n++)) >= -1)
    {
        sprintf(buf, "fib %d = %lld\n", n - 1, rt);
        fib_log_write(buf);
        usleep(200);
    }

error:
    /**
     * close log file
     */
    fib_log_deinit();

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
    printf("\033[0;31m(c) Copyright 2015, Sercomm. All rights reserved.\033[0m\n");  
    printf("\033[0;31mfib %s (for Linux) @ 2015-07-21 designed by Antonio An\033[0m\n\n", ALGOR_VERSION1);  
    printf("\033[0;31mBrief : \033[0;32mThe maximum value for the reality in the Fibonacci\033[0m\n");  
    printf("\033[0;31m        \033[0;32m sequence can achieve\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m\033[0m\n");  
    printf("\033[0;31m        \033[0;32mfib -n <number>\033[0m\n");  
    printf("\033[0;31m        \033[0;32mfib -t <alg #> -f <filename>\033[0m\n");  
    printf("\033[0;31m        \033[0;32mfib -[hTvV]\033[0m\n");  
    printf("\033[0;31mParam : \033[0m\n");
    printf("\033[0;31m        \033[0;32m-f --file   The log file to be writed\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-h --help   Show help information\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-t          Test algorithm #, output to file\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-v          Show simple version number\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-V          Show detail version information\033[0m\n");  
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
    const char *optstr = "hvVTf:t:n:";
    struct option opts[] = {
        { "help"   , no_argument      , 0, 'h'},
        {  NULL    , required_argument, 0, 'n'},
        {  NULL    , required_argument, 0, 'T'},
        {  NULL    , required_argument, 0, 't'},
        { "file"   , required_argument, 0, 'f'},
        {  NULL    , required_argument, 0, 'v'},
        {     0    ,       0          , 0,  0 }
    };

    while ( ( opt = getopt_long( agrc, agrv, optstr, opts, NULL ) ) != -1 ) {
        switch(opt) {
            case 'h':
                print_usage();
                break;
            case 'v':
                PRINT_VER(1); 
                break;
            case 'V':
                PRINT_VER_INFO(1); 
                break;
            case 'f':
                strcpy(log_path, optarg);
                break;
            case 't':
                if (optarg != NULL)
                    t_flag = 1;
                if (agrv[optind] != NULL)
                    algor_num = atoi(agrv[optind]);
                else return -1;
                break;
            case 'n':
                if (optarg != NULL)
                    printf("num: %lld\n", fib2(atoi(optarg)));
                break;
            case '?':
            default:
                return -1;
        }
    }

    return 0;
}
