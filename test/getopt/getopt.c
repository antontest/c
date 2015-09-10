#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/**
 * @brief print_usage 
 *
 * @param program_name
 */
void print_usage(const char *program_name) {  
    printf("%s 1.0.0 (2015-06-02)\n", program_name);  
    printf("This is a program decoding a BER encoded CDR file\n");  
    printf("Usage: %s -f <file_name> -o <output_name> [-c <config_name>] [-k <keyword>]\n", program_name);  
    printf("    -f --file       the CDR file to be decoded\n");  
    printf("    -o --output     the output file in plain text format\n");  
    printf("    -c --config     the description file of the CDR file, if not given, use default configuration\n");  
    printf("    -k --keyword    the keyword to search, if not given, all records will be written into output file\n");  
} 

/**
 * @brief parser_args 
 *
 * @param agrc
 * @param agrv[]
 *
 * @return 
 */
static int parser_args(int agrc, char *agrv[])
{
    int opt = 0;
    struct option opts[] = {
        { "help"   , no_argument      , 0, 'h'},
        { "file"   , required_argument, 1, 'f'},
        { "output" , no_argument      , 0, 'o'},
        { "keyword", no_argument      , 0, 'k'},
        {     0    ,       0          , 0,  0 }
    };

    while ( ( opt = getopt_long( agrc, agrv, "hf:ok", opts, NULL ) ) != -1 ) {
        switch(opt) {
            case 'h':
                print_usage(__FILE__);
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
                print_usage(__FILE__);
        }
    }
}

int main(int agrc, char *agrv[])
{
    parser_args(agrc, agrv);

    return 0;
}
