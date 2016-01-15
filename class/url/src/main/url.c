#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <url.h>
#include <utils/get_args.h>
#include <utils/utils.h>
#include <data/linked_list.h>


typedef struct ca_info_t ca_info_t;
struct ca_info_t {
    char name[56];
    char id[28];
};
typedef struct info_t info_t;
struct info_t {
    linked_list_t *ca;
    char selected_ca_id[28];
    char status[10];
};

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
int get_user_status(FILE *fp, char *status)
{
    char *pstatus = status;
    char buf[1024] = {0};
    char *pos = NULL;

    fseek(fp, 0, SEEK_SET);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strcasestr(buf, "<form");
        if (pos) break;
    }
    if (!pos) return -1;

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strcasestr(buf, "selectchangestatus");
        if (pos) break;
        pos = strcasestr(buf, "</select>");
        if (pos) {
            pos = NULL;
            break;
        }
    }
    if (!pos) return -1;

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strcasestr(buf, "selected");
        if (pos) break;
        pos = strcasestr(buf, "</select>");
        if (pos) {
            pos = NULL;
            break;
        }
    }
    if (!pos) return -1;

    pos = strcasestr(buf, "value");
    if (!pos) return -1;
    pos = strchr(pos, '=');
    if (!pos) return -1;
    pos++;

    while (*pos == ' ') pos++;
    if (*pos == '\'' || *pos == '\"') pos++;
    while (*pos != '\'' && *pos != '\"' && *pos != '\0') {
        *pstatus = *pos;
        pos++;
        pstatus++;
    }
    *pstatus = '\0';
 
    return 0;
}

ca_info_t *ca_info_create(char *name, char *id)
{
    ca_info_t *ca = NULL;

    ca = (ca_info_t *)malloc(sizeof(ca_info_t));
    if (!ca) return NULL;
    if (name) strncpy(ca->name, name, sizeof(ca->name));
    if (id) strncpy(ca->id, id, sizeof(ca->id));

    return ca;
}

int gather_info(info_t *info, const char *file)
{
    int ret = -1;
    FILE *fp = NULL;
    char *pos = NULL;
    char buf[1024] = {0};
    ca_info_t *ca = NULL;
    char *name_start_pos = NULL;
    char *name_end_pos   = NULL;
    char *id_start_pos   = NULL;
    char *id_end_pos     = NULL;
    char *pselectedcaid  = info->selected_ca_id;

    fp = fopen(file, "r");
    if (!fp) return -1;

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strcasestr(buf, "function");
        if (pos) break;

        pos = strcasestr(buf, "[CANAME]");
        if (!pos) continue;
        if (get_html_value(buf, "[CANAME]", &name_start_pos, &name_end_pos) < 0)
            goto over;

        *name_end_pos = '\0';
        ca = ca_info_create(name_start_pos, NULL);
        if (!fgets(buf, sizeof(buf), fp)) break;
        if (get_html_value(buf, "[CAID]", &id_start_pos, &id_end_pos) < 0)
            goto over;

        *id_end_pos = '\0';
        strncpy(ca->id, id_start_pos, sizeof(ca->id));
        //printf("name: %s, id: %s\n", ca->name, ca->id);
        info->ca->insert_last(info->ca, ca);
    }
    if (!pos) goto over;
    
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strcasestr(buf, "availablecas[i][CAID] ==");
        if (!pos) continue;

        pos = strchr(buf, '=');
        if (!pos) goto over;
        while (*pos == '=') pos++;
        while (*pos == ' ') pos++;
        if (*pos == '\'' || *pos == '\"') pos++;
        while (*pos != '\'' && *pos != '\"') {
            *pselectedcaid = *pos;
            pos++;
            pselectedcaid++;
        }
        *pselectedcaid = '\0';
    }
    if (pselectedcaid == info->selected_ca_id) goto over;
    //printf("selectedcaid: %s\n", info->selected_ca_id);

    if (get_user_status(fp, info->status) < 0) goto over;
    //printf("status: %s\n", info->status);
    ret = 0;
over:
    if (fp) fclose(fp);
    return ret;
}

int change_user_status(url_t *url, info_t *info)
{
    char *user_status = NULL;
    char *post_request = NULL;

    user_status = info->status;
    if (!user_status) return -1;
    if (!strcmp(user_status, "10")) return 0;

    /**
     * change user status
     */
    url->set_value(url, "selectchangestatus", "10");
    post_request = url->gen_post_request(url);
    if (!post_request) return -1;
    strcat(post_request, "&buttonedituser=Save");
    return url->post(url, post_request);
}

int change_ca(url_t *url, info_t *info, const char *ca_name)
{
    int ca_cnt = 0;
    ca_info_t *ca = NULL;
    char *selectcaname = NULL;
    char *post_request = NULL;

    info->ca->reset_current(info->ca);
    ca_cnt = info->ca->get_count(info->ca);
    while (ca_cnt-- > 0) {
        if (info->ca->get_next(info->ca, (void **)&ca) == NOT_FOUND)
            break;

        if (!strcmp(ca->id, info->selected_ca_id))
            break;
    }
    if (!ca) return -1;
    selectcaname = ca->name;
    if (!strcmp(selectcaname, ca_name)) return 0;

    info->ca->reset_current(info->ca);
    ca_cnt = info->ca->get_count(info->ca);
    while (ca_cnt-- > 0) {
        if (info->ca->get_next(info->ca, (void **)&ca) == NOT_FOUND)
            break;

        if (!strcmp(ca->name, ca_name))
            break;
    }
    if (!ca) return -1;

    url->set_value(url, "buttonedituser", NULL);
    url->set_value(url, "selectca", ca->id);
    post_request = url->gen_post_request(url);
    return url->post(url, post_request);    
}

int download_cert(url_t *url, const char *user, const char *password)
{
    char http_url[256] = {0};
    char cert[56] = {0};
    char key_path[128] = {0};
    char cmd_buf[256] = {0};
    char req_file_path[128] = {0};

    snprintf(http_url, sizeof(http_url), "https://172.21.34.86:8443/ejbca/certreq");
    snprintf(cert, sizeof(cert), "%s_cert.pem", user);

    if (!access(cert, R_OK)) return 0;
    snprintf(key_path, sizeof(key_path), "%s_key.pem", user);
    snprintf(cmd_buf, sizeof(cmd_buf), "ipsec pki --gen --size 2048 --outform pem > %s", key_path);
    if (system(cmd_buf) != 0) return -1; 
    snprintf(req_file_path, sizeof(req_file_path), "%s_req.pem", user);
    snprintf(cmd_buf, sizeof(cmd_buf), "ipsec pki --req --in %s --dn \"C=CN, O=Sercomm, CN=%s\" --outform pem > %s", key_path, user, req_file_path); 
    if (system(cmd_buf) != 0) return -1; 

    url->form_add(url,
        FORM_TEXT, "user", user,
        FORM_TEXT, "password", password,
        FORM_FILE, "pkcs10file", req_file_path,
        FORM_TEXT, "filename", "tmp.pem",
        FORM_TEXT, "resulttype", "1"
    );
    url->set_url(url, http_url, cert);
    url->save_file(url);
    return url->form_post(url);
}

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
    char *hurl      = NULL;
    char *ssl_info = NULL;
    char *ssl_cert = NULL;
    char *ssl_key  = NULL;
    char *user     = NULL;
    char *password = NULL;
    char *data     = NULL;
    char http_url[512] = {0};
    char http_htm[128] = {0};
    struct options opt[] = {
        {"-r", "--url",      1, RET_STR, ADDR_ADDR(hurl)},
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
    url_t *url = NULL;
    info_t info = {
        .ca = linked_list_create(),
        .status = {0},
    };

    /**
     * parse cmdline
     */
    get_args(agrc, agrv, opt);
    if (help_flag) {
        print_usage(usg);
        return 0;
    }
    if (!url && !ejbca_flag) return -1;
    if (ejbca_flag && !user) return -1;
    if (!hurl) hurl = "https://172.21.34.86:8443/ejbca/adminweb/ra/editendentity.jsp";
    if (!ssl_info) ssl_info = "/home/anton/certs/ejbca/superadmin/ssl.ca.crt.pem";
    if (!ssl_cert) ssl_cert = "/home/anton/certs/ejbca/superadmin/ssl.cli.crt.pem";
    if (!ssl_key)  ssl_key  = "/home/anton/certs/ejbca/superadmin/ssl.cli.key.pem";

    url = url_create();
    if (!url) return -1;

    https_flag = strcasestr(hurl, "https") ? 1: 0;
    if (https_flag) url->ssl_init(url, ssl_info, ssl_cert, ssl_key);
    if (ejbca_flag) {
        snprintf(http_url, sizeof(http_url), "%s?username=%s", hurl, user);
        snprintf(http_htm, sizeof(http_htm), "%s.htm", user);
    } else {
        snprintf(http_url, sizeof(http_url), "%s", hurl);
        snprintf(http_htm, sizeof(http_htm), "tmp.htm");
    }
    url->set_url(url, http_url, http_htm);
    
    if (!post_flag) url->get(url);
    else {
        if (!data) return -1;
        url->post(url, data);
    }

    if (ejbca_flag) {
        /**
         * parser html data 
         */
        if (url->parse_form_data(url) < 0) 
            goto over;

        /**
         * gather_info 
         */ 
        if (gather_info(&info, http_htm) < 0)
            goto over;
        //url->list_data(url);
        
        /**
         * change user status
         */
        if (url->set_value(url, "selectca", info.selected_ca_id) < 0)
            goto over;
        if (change_user_status(url, &info) < 0) 
            goto over;
        
        /**
         * change ca to OperatorCA
         */
        if (change_ca(url, &info, "OperatorCA") < 0)
            goto over;

        /**
         * download user cert
         */
        password = url->get_value(url, "textfieldpassword");
        if (!password) goto over;
        if (download_cert(url, user, password) < 0)
            goto over;
    }

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
    url->form_add(url, FORM_FILE, "FILE1", "CMakeLists.txt");
    url->form_add(url, FORM_TEXT, "todo", "save");
    url->form_add(url, FORM_TEXT, "next_file", "upload.htm");
    url->form_add(url, FORM_TEXT, "this_file", "upload.htm");
    */
    /*
    url->form_add(url, 
        FORM_TEXT, "next_path", "upload.htm",
        FORM_FILE, "FILE1", "CMakeLists.txt",
        FORM_TEXT, "todo", "save",
        FORM_TEXT, "next_file", "upload.htm",
        FORM_TEXT, "this_file", "upload.htm"
    );
    url->form_post(url);
    */

    rt = 0;
over:
    if (info.ca) info.ca->destroy(info.ca);
    url->destroy(url);
    return rt;

}
