/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <cgi.h>
#include <ftp.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
static int set_rem(char *input, char *errmsg)
{
    //ALERT(input);
    return 0;
}

static int get_rem(char *input, char *errmsg)
{
    strcpy(input, "123");
    return 0;
}


static int set_username(char *input, char *errmsg)
{
    //ALERT(input);
    return 0;
}

static int get_username(char *input, char *errmsg)
{
    strcat(input, "admin");
    return 0;
}

static int set_password(char *input, char *errmsg)
{
    //ALERT(input);
    return 0;
}

static int get_ipsec_mode(char *input, char *errmsg)
{
    strcpy(input, "aka");
    return 0;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    cgi_t *cgi = cgi_create();
    cgi_func_tab_t data[] = {
        {"h_rem",         VAR_IS_VAR,  (void *)get_rem,           (void *)set_rem},
        {"h_ipsec_mode",  VAR_IS_VAR,  (void *)get_ipsec_mode,    NULL},
        {"username",      VAR_IS_VAR,  (void *)get_username,      (void *)set_username},
        {"password",      VAR_IS_VAR,  NULL,                      (void *)set_password},
        {"ftp_dir",       VAR_IS_FILE, (void *)get_ftp_dir,       NULL},
        {"ftp_file_name", VAR_IS_FILE, (void *)get_ftp_file_name, NULL},
        {"ftp_file_size", VAR_IS_FILE, (void *)get_ftp_file_size, NULL},
        {NULL,            VAR_IS_VAR,  NULL,                      NULL}
    };

    cgi->get_data(cgi);
    cgi->read_back(cgi, data);
    cgi->write_back(cgi, data);
    cgi->destroy(cgi);

    return rt;
}
