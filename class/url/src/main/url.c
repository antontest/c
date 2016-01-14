#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <url.h>
#include <utils/get_args.h>
#include <utils/utils.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    int help_flag  = 0;
    int ejbca_flag = 0;
    int post_flag  = 0;
    int https_flag = 0;
    char *url      = NULL;
    char *ssl_info = NULL;
    char *ssl_cert = NULL;
    char *ssl_key  = NULL;
    char *user     = NULL;
    char *password = NULL;
    char *data     = NULL;
    char http_url[512] = {0};
    char http_htm[128] = {0};
    struct options opt[] = {
        {"-r", "--url",      1, RET_STR, ADDR_ADDR(url)},
        {"-i", "--sslinfo",  1, RET_STR, ADDR_ADDR(ssl_info)},
        {"-c", "--sslcert",  1, RET_STR, ADDR_ADDR(ssl_cert)},
        {"-k", "--sslkey",   1, RET_STR, ADDR_ADDR(ssl_key)},
        {"-u", "--user",     1, RET_STR, ADDR_ADDR(user)},
        {"-p", "--password", 1, RET_STR, ADDR_ADDR(password)},
        {"-e", "--ejbca",    0, RET_INT, ADDR_ADDR(ejbca_flag)},
        {"-d", "--data",     1, RET_STR, ADDR_ADDR(data)},
        {NULL, "--post",     0, RET_INT, ADDR_ADDR(post_flag)},
        {"-h", "--help",     0, RET_INT, ADDR_ADDR(help_flag)},
        {NULL},
    };
    struct usage usg[] = {
        {"-r, --url",      "http or https url"},
        {"-i, --sslinfo",  "ssl ca cert path"},
        {"-c, --sslcert",  "ssl client cert"},
        {"-k, --sslkey",   "ssl client key"},
        {"-u, --user",     "user name"},
        {"-p, --password", "user password"},
        {"-e, --ejbca",    "download cert from ejbca server"},
        {"-d, --data",     "post data"},
        {"-h, --help",     "show usage"},
        {NULL},
    };
    url_t *curl = NULL;

    /**
     * parse cmdline
     */
    get_args(agrc, agrv, opt);
    if (help_flag) {
        print_usage(usg);
        return 0;
    }
    if (!url && !ejbca_flag) return -1;
    if (ejbca_flag && (!user || !password)) return -1;
    if (!url) url = "https://172.21.34.86:8443/ejbca/adminweb/ra/editendentity.jsp";
    if (!ssl_info) ssl_info = "/home/anton/certs/ejbca/superadmin/ssl.ca.crt.pem";
    if (!ssl_cert) ssl_cert = "/home/anton/certs/ejbca/superadmin/ssl.cli.crt.pem";
    if (!ssl_key)  ssl_key  = "/home/anton/certs/ejbca/superadmin/ssl.cli.key.pem";

    curl = url_create();
    if (!curl) return -1;

    https_flag = strcasestr(url, "https") ? 1: 0;
    if (https_flag) curl->ssl_init(curl, ssl_info, ssl_cert, ssl_key);
    if (ejbca_flag) {
        snprintf(http_url, sizeof(http_url), "%s?username=%s", url, user);
        snprintf(http_htm, sizeof(http_htm), "%s.htm", user);
    } else {
        snprintf(http_url, sizeof(http_url), "%s", url);
        snprintf(http_htm, sizeof(http_htm), "tmp.htm");
    }
    curl->set_url(curl, http_url, http_htm);
    
    if (!post_flag) curl->get(curl);
    else {
        if (!data) return -1;
        curl->post(curl, data);
    }
    curl->parse_form_data(curl);
    curl->list_data(curl);

    /*
    url->set_url(url, "http://172.21.34.27:8000", "index.htm");
    url->get(url);
    */
    
    /*
    url->set_url(url, "http://172.21.34.27:8000/setup.cgi", "login.htm");
    url->post(url, "username=admin&password=admin&h_rem=1&todo=save&next_file=htmlcontrol.htm&this_file=index.htm");
    */

    /*
    url->set_url(url, "http://172.21.34.27:8000/upload.htm", "upload.htm");
    url->get(url);
    */

    /*
    url->set_url(url, "http://172.21.34.27:8000/upload.cgi", NULL);
    url->form_add(url, FORM_TEXT, "next_path", "upload.htm");
    url->form_add(url, FORM_FILE, "FILE1", "index.htm");
    url->form_add(url, FORM_TEXT, "todo", "save");
    url->form_add(url, FORM_TEXT, "next_file", "upload.htm");
    url->form_add(url, FORM_TEXT, "this_file", "upload.htm");
    url->form_post(url);
    */

    curl->destroy(curl);
    return rt;
}
