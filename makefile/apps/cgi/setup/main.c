/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <cgi/cgi.h>
#include <ftp.h>
#include <login.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    cgi_t *cgi = cgi_create();
    cgi_func_tab_t data[] = {
        {"index",         KEY_IS_FILE, NULL,                     NULL},
        {"h_rem",         KEY_IS_VAR,  (void *)get_rem,           (void *)set_rem},
        {"username",      KEY_IS_VAR,  (void *)get_username,      (void *)set_username},
        {"password",      KEY_IS_VAR,  NULL,                      (void *)set_password},
        {"ftp",           KEY_IS_FILE, NULL,                     NULL},
        {"ftp_dir",       KEY_IS_VAR, (void *)get_ftp_dir,       NULL},
        {"ftp_file_name", KEY_IS_VAR, (void *)get_ftp_file_name, NULL},
        {"ftp_file_size", KEY_IS_VAR, (void *)get_ftp_file_size, NULL},
        {NULL,            KEY_IS_UNKOWN,      NULL,              NULL}
    };

    cgi_header_content_type("text/html");
    cgi->parse_form_input(cgi, data);
    cgi->write_to_html(cgi, data);
    cgi->destroy(cgi);

    return rt;
}
