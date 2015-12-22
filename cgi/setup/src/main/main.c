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
#include <login.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
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
        {"h_rem",         KEY_IS_VAR,  (void *)get_rem,           (void *)set_rem},
        {"h_ipsec_mode",  KEY_IS_VAR,  (void *)get_ipsec_mode,    NULL},
        {"username",      KEY_IS_VAR,  (void *)get_username,      (void *)set_username},
        {"password",      KEY_IS_VAR,  NULL,                      (void *)set_password},
        {"ftp_dir",       KEY_IS_VAR, (void *)get_ftp_dir,       NULL},
        {"ftp_file_name", KEY_IS_VAR, (void *)get_ftp_file_name, NULL},
        {"ftp_file_size", KEY_IS_VAR, (void *)get_ftp_file_size, NULL},
        {NULL,            KEY_IS_VAR,  NULL,                      NULL}
    };

    cgi->get_form_data(cgi);
    cgi->read_action(cgi, data);
    cgi->write_action(cgi, data);
    cgi->destroy(cgi);

    return rt;
}
