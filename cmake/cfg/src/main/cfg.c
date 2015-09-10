/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

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

    /**
     * Get paramters from the command line
     */
    if (parser_args(agrc, agrv) < 0) {
        print_usage();
        rt = -1;
        goto error;
    };

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
    printf("\033[0;31mUsage : \033[0;32m-f <file_name> -o <output_name>\033[0m\n");  
    printf("\033[0;31mParam : \033[0m\n");
    printf("\033[0;31m        \033[0;32m-f --file       the CDR file to be decoded\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-o --output     the output file in plain text format\033[0m\n");  
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
    const char *optstr = "hf:o";
    struct option opts[] = {
        { "help"   , no_argument      , 0, 'h'},
        { "file"   , required_argument, 0, 'f'},
        { "output" , no_argument      , 0, 'o'},
        {     0    ,       0          , 0,  0 }
    };

    if (agrc < 2) return -1;

    while ( ( opt = getopt_long( agrc, agrv, optstr, opts, NULL ) ) != -1 ) {
        switch(opt) {
            case 'h':
                return -1;
                break;
            case 'f':
                printf("Input file is %s.\n", optarg);
                break;
            case 'o':
                printf("Output file is %s.\n", optarg);
                break;
            case 'k':
                printf("Keyword is %s.\n", optarg);
                break;
            case '?':
            default:
                return -1;
        }
    }

    return 0;
}
