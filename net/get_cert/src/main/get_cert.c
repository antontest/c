#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sys/wait.h>

#define END_ENTITY_HTM_PATH "edit_end_entity.htm"
#define EJBCA_SERVER        "https://172.21.34.86:8443"
#define CERT_GEN_DIR        "."
#define CA_GEN_DIR          "."

#define SSL_CERT_KEY_PATH "/home/anton/certs/ejbca/superadmin/"
#define SSL_CA_CERT       "ssl.ca.crt.pem"
#define SSL_CLI_CERT      "ssl.cli.crt.pem"
#define SSL_CLI_KEY       "ssl.cli.key.pem"

size_t write_data(void* buffer, size_t size, size_t nmemb, void *stream)  
{  
    FILE *fptr = (FILE*)stream;  
    if (!fptr) return -1;
    //fwrite(buffer, size, nmemb, fptr);  
    fputs(buffer, fptr);
    return size * nmemb;  
}  

typedef enum html_input_type_t {
    HTML_UNKOWN = -1,
    HTML_INPUT = 0,
    HTML_SELECT
} html_input_type_t;

typedef struct html_data_t {
    const char *name;
    html_input_type_t type;
    char value[56];
} html_data_t;

static struct html_data_t html_data[] = {
    {"action",                    HTML_INPUT},
    {"hiddenprofile",             HTML_INPUT},
    {"username",                  HTML_INPUT},
    {"selectchangestatus",        HTML_SELECT},
    {"buttonedituser",            HTML_INPUT},
    {"textfieldpassword",         HTML_INPUT},
    {"textfieldconfirmpassword",  HTML_INPUT},
    {"checkboxcleartextpassword", HTML_INPUT},
    {"textfieldsubjectdn13",      HTML_INPUT},
    {"textfieldsubjectdn15",      HTML_INPUT},
    {"textfieldsubjectdn19",      HTML_INPUT},
    {"selectcertificateprofile",  HTML_SELECT},
    {"selecttoken",               HTML_SELECT},
    {"selectca",                  HTML_UNKOWN},
    {"buttonedituser",            HTML_INPUT},
    {NULL}
};

/**
 * @brief generate post request string
 */
char *gen_requst_string()
{
    static char requst_data[1024] = {0};
    html_data_t *p = html_data;
    int len = 0;

    while (p->name != NULL) {
        if (p->type != HTML_UNKOWN) { 
            len += snprintf(requst_data + len, sizeof(requst_data) - len, "%s=%s&", p->name, p->value);
        }
        p++;
    }
    requst_data[len - 1] = '\0';
    //printf("%s\n", requst_data);

    return requst_data;
}

int get_user_data(CURL *curl, const char *username)
{
    char buf[1024] = {0};
    char key[56]   = {0};
    char caid[28]  = {0};
    char url[256]  = {0};
    char *pos = NULL;
    FILE *fp  = NULL;
    char *pvalue;
    struct html_data_t *p = html_data;
    CURLcode res;

    /**
     * open file for saving user information html
     */
    fp = fopen(END_ENTITY_HTM_PATH, "w");
    if (!fp) return -1;

    /**
     * send html get request
     */
    snprintf(url, sizeof(url), EJBCA_SERVER "/ejbca/adminweb/ra/editendentity.jsp?username=%s", username);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);   
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }

    /**
     * deal with requst data
     */
    fp = fopen(END_ENTITY_HTM_PATH, "r");
    if (!fp) return -1;
    while (fgets(buf, sizeof(buf), fp) != NULL && p->name != NULL) {
        if (p->type == HTML_UNKOWN) {
            p++;
            continue;
        }
        
        snprintf(key, sizeof(key), "name=\"%s\"", p->name);
        pos = strstr(buf, key);
        if (!pos) continue;
        
        /**
         * get key value
         */
        if (p->type == HTML_INPUT) {
            pos = strstr(buf, "value");
            if (!pos) return -1;

            pvalue = p->value;
            pos += strlen("value='");
            while (*pos != '\'' && *pos != '\"') {
                *pvalue ++ = *pos++;
            }
            //printf("%s=%s\n", p->name, p->value);
        } else if (p->type == HTML_SELECT) {
            while (fgets(buf, sizeof(buf), fp) != NULL) {
                pos = strstr(buf, "selected");
                if (pos) break;
            }
            if (!pos) return -1;

            pos = strstr(buf, "value");
            if (!pos) return -1;
            pos += strlen("value='");
            pvalue = p->value;
            while (*pos != '\'' && *pos != '\"') {
                *pvalue ++ = *pos++;
            }
            //printf("%s=%s\n", p->name, p->value);
        }

        p++;
    }

    /** 
     * get selected ca id
     */
    fseek(fp, 0, SEEK_SET);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strstr(buf, "selectca");
        if (pos) break;
    }
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strstr(buf, "CAID");
        if (!pos) continue;
        pos = strstr(buf, "==");
        if (!pos) continue;

        pos += 4;
        pvalue = caid;
        while (*pos != '\'' && *pos != '\"') {
            *pvalue ++ = *pos++;
        }
        //printf("%s=%s\n", "selectca", caid);
        break;
    }
    if (!pos) return -1;

    /**
     * set selectca id
     */
    p = html_data;
    while (p->name != NULL) {
        if (!strcmp(p->name, "selectca")) {
            p->type = HTML_INPUT;
            strcpy(p->value, caid);
            break;
        }
        p++;
    }

    return 0;
}

/**
 * @brief change cert issuer CA
 */
int change_ca(CURL *curl, const char *ca_name, const char *user)
{
    struct html_data_t *p = html_data;
    FILE *fp = NULL;
    char buf[1024] = {0};
    char key[128] = {0};
    char url[256] = {0};
    char *pos = NULL;
    char *requst_data = NULL;
    CURLcode res;

    if (!ca_name || !p) return -1;
    fp = fopen(END_ENTITY_HTM_PATH, "r");
    if (!fp) return -1;

    /**
     * find ca name position
     */
    snprintf(key, sizeof(key), "[CANAME] = \"%s\"", ca_name);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strstr(buf, ca_name);
        if (pos) break;
    }
    if (!pos) return -1;

    /**
     * get ca id
     */
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strstr(buf, "CAID");
        if (!pos) continue;

        pos = strrchr(buf, ';');
        if (pos) *pos = '\0';
        pos = strstr(buf, "=");
        if (!pos) break;

        while (p->name != NULL) {
            if (!strcmp(p->name, "selectca")) 
                break;
            p++;
        }
        if (!p->name) return -1;

        if (!strcmp(p->value, pos + 2)) return 0;
        p->type = HTML_INPUT;
        strcpy(p->value, pos + 2);
        break;
    }
    if (!pos) return -1;

    /**
     * change edituser event to change issuer ca
     */
    p = html_data;
    while (p->name != NULL) {
        if (!strcmp(p->name, "buttonedituser")) { 
            p->type = HTML_UNKOWN;
            break;
        }
        p++;
    }
    p++;
    while (p->name != NULL) {
        if (!strcmp(p->name, "buttonedituser")) { 
            p->type = HTML_INPUT;
            strcpy(p->value, "Save");
            break;
        }
        p++;
    }

    /**
     * post to change issuer ca
     */
    snprintf(url, sizeof(url), "%s/ejbca/adminweb/ra/editendentity.jsp?username=%s", EJBCA_SERVER, user);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    requst_data = gen_requst_string();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requst_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1); 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }
    printf("change ca \"%s\" succ\n", ca_name);

    return 0;
}

/**
 * @brief change user status to "New" 
 */
int change_user_status(CURL *curl)
{
    struct html_data_t *p = html_data;
    char *post_data = NULL;
    CURLcode res;

    /**
     * change user status
     */
    p = html_data;
    while (p->name != NULL && p->type != HTML_UNKOWN) {
        if (!strcmp(p->name, "selectchangestatus")) {
            if (!strcmp(p->value, "10")) return 0;
            strcpy(p->value, "10");
            break;
        }
        p++;
    }

    /**
     * change edituser event to change user status 
     */
    p = html_data;
    while (p->name != NULL) {
        if (!strcmp(p->name, "buttonedituser")) {
            break;
        }
        p++;
    }
    p++;
    while (p->name != NULL) {
        if (!strcmp(p->name, "buttonedituser")) {
            p->type = HTML_UNKOWN;
            break;
        }
        p++;
    }

    /**
     * post ca change request
     */
    post_data = gen_requst_string();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1); 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }
    
    return 0;
}

/**
 * download user cert
 */
int download_cert(CURL *curl, const char *user, const char *password)
{
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr  = NULL;
    struct curl_slist *headerlist  = NULL;
    //struct html_data_t *p = html_data;
    const char buf[] = "Expect:";
    char cmd_buf[512] = {0};
    char key_path[128] = {0};
    char req_file_path[128] = {0};
    char cert_path[128] = {0};
    FILE *fp = NULL;
    CURLcode res;

    /**
     * 1. check cert whether is exist
     * 2. open file for getting OPERATOR_CERT
     */
    snprintf(cert_path, sizeof(cert_path), CERT_GEN_DIR "/%s_cert.pem", user);
    if (!access(cert_path, R_OK)) goto over;
    if ((fp = fopen(cert_path, "w")) == NULL) {  
        fprintf(stderr, "fopen file error:%s\n", cert_path);  
        goto over;
    }  
    
    /**
     * change ca to VendorCA
     */
    if (change_ca(curl, "OperatorCA", user) < 0) {
        goto over;
    }

    /* Fill in the user field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "user",
            CURLFORM_COPYCONTENTS, user,
            CURLFORM_END);

    /* Fill in the password field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "password",
            CURLFORM_COPYCONTENTS, password,
            CURLFORM_END);
    
    /**
     * generate request file
     */
    snprintf(key_path, sizeof(key_path), "%s/%s_key.pem", CERT_GEN_DIR, user);
    snprintf(cmd_buf, sizeof(cmd_buf), "ipsec pki --gen --size 2048 --outform pem > %s", key_path);
    if (WEXITSTATUS(system(cmd_buf)) < 0) {
        fprintf(stderr, "generate cert key failed!\n");
        return -1;
    }
    snprintf(req_file_path, sizeof(req_file_path), "%s/%s_req.pem", CERT_GEN_DIR, user);
    snprintf(cmd_buf, sizeof(cmd_buf), "ipsec pki --req --in %s --dn \"C=CN, O=Sercomm, CN=%s\" --outform pem > %s", key_path, user, req_file_path);
    if (WEXITSTATUS(system(cmd_buf)) < 0) {
        fprintf(stderr, "generate cert request file failed!\n");
        return -1;
    }

    /* Fill in the file upload field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "pkcs10file",
            CURLFORM_FILE, req_file_path,
            CURLFORM_END);

    /* Fill in the filename field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "filename",
            CURLFORM_COPYCONTENTS, "req_file.pem",
            CURLFORM_END);

    /* Fill in the password field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "resulttype",
            CURLFORM_COPYCONTENTS, "1",
            CURLFORM_END);

    /**
     * post OPERATOR_CERT generate request
     */
    headerlist = curl_slist_append(headerlist, buf);
    curl_easy_setopt(curl, CURLOPT_URL, EJBCA_SERVER "/ejbca/certreq");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 

    /**
     * Perform the request, res will get the return code 
     */ 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    printf("download %s_cert.pem succ\n", user);

    /**
     * change user status
     */
    /*
    p = html_data;
    while (p->name != NULL && p->type != HTML_UNKOWN) {
        if (!strcmp(p->name, "selectchangestatus")) {
            strcpy(p->value, "40");
            break;
        }
        p++;
    }
    */

over:
    if (fp) fclose(fp);
    curl_formfree(formpost);
    curl_slist_free_all (headerlist);
    return 0;
}

/**
 * download ca cert
 */
int download_ca_cert(CURL *curl, const char *ca_name)
{
    FILE *fp = NULL;
    char url[256] = {0};
    char dn[56] = {0};
    char ca_cert[128] = {0};
    char *dncode = NULL;
    CURLcode res;

    /**
     * 1. check ca cert whether is exist 
     * 2. open ca cert
     */
    snprintf(ca_cert, sizeof(ca_cert), "%s/%s.pem", CA_GEN_DIR, ca_name);
    if (!access(ca_cert, R_OK)) return 0;
    fp = fopen(ca_cert, "w");
    if (!fp) return -1;

    /**
     * encode url
     */
    snprintf(dn, sizeof(dn), "CN=%s,O=Sercomm,C=CN", ca_name);
    dncode = curl_easy_escape(curl, dn, strlen(dn));
    if (!dncode) return -1;
    snprintf(url, sizeof(url), "%s/ejbca/publicweb/webdist/certdist?cmd=cacert&issuer=%s&level=0", EJBCA_SERVER, dncode);
    curl_free(dncode);

    /**
     * send file download request
     */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(curl, CURLOPT_URL, url);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }
    printf("download ca %s.pem succ\n", ca_name);

    return 0;
}

/**
 * libcurl init
 */
CURL *curl_init()
{
    CURL *curl;

    /**
     * check ssl cert whether is exist
     */
    if (access(SSL_CERT_KEY_PATH SSL_CA_CERT, R_OK)) {
        fprintf(stderr, "ssl ca cert \"%s\" does not exist!\n", SSL_CERT_KEY_PATH SSL_CA_CERT);
        return NULL;
    }
    if (access(SSL_CERT_KEY_PATH SSL_CLI_CERT, R_OK)) {
        fprintf(stderr, "ssl client cert \"%s\" does not exist!\n", SSL_CERT_KEY_PATH SSL_CLI_CERT);
        return NULL;
    }
    if (access(SSL_CERT_KEY_PATH SSL_CLI_KEY, R_OK)) {
        fprintf(stderr, "ssl cert key \"%s\" does not exist!\n", SSL_CERT_KEY_PATH SSL_CLI_KEY);
        return NULL;
    }

    /**
     * libcurl init
     */
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl) return NULL;

    /**
     * libcurl https SSL init
     */
    curl_easy_setopt(curl, CURLOPT_CAPATH, SSL_CERT_KEY_PATH);
    curl_easy_setopt(curl, CURLOPT_CAINFO, SSL_CERT_KEY_PATH SSL_CA_CERT);
    curl_easy_setopt(curl, CURLOPT_SSLCERT, SSL_CERT_KEY_PATH SSL_CLI_CERT); 
    curl_easy_setopt(curl, CURLOPT_SSLKEY, SSL_CERT_KEY_PATH SSL_CLI_KEY); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);   
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);   
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5); /*timeout of connect */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); /* timeout of data recv */

    return curl;
}

/**
 * process of getting cert from ejbca 
 *
 * 1. acquire user data from ejbca server;
 * 2. if user status is generated, change to "New";
 * 3. if user issuer is not "VendorCA", change to "VendorCA";
 * 4. if cert is not exist, download cert
 * 5. change user issuer to "OperatorCA" for cmpv2;
 * 6. if OperatorCA is not exist, download for ipsec
 */
int get_cert_from_ejbca(const char *user, const char *password)
{
    int ret = -1;
    char err_msg[256] = {0};
    CURL *curl;

    /**
     * check user and password
     */
    if (!user || !password) return -1;

    /**
     * lincurl init
     */
    if (!(curl = curl_init())) {
        snprintf(err_msg, sizeof(err_msg), "libcurl init failed\n");
        goto cleanup;
    } 

    /**
     * process html returned data
     */
    if (get_user_data(curl, user) < 0) {
        snprintf(err_msg, sizeof(err_msg), "get user data failed\n");
        goto cleanup;
    }

    /**
     * post to change user status
     */
    if (change_user_status(curl) < 0) {
        snprintf(err_msg, sizeof(err_msg), "change user status failed\n");
        goto cleanup;
    }

    /**
     * change ca to VendorCA
     */
    /*
    if (change_ca(curl, "VendorCA", user) < 0) {
        snprintf(err_msg, sizeof(err_msg), "change ca to \"VendorCA\" failed\n");
        goto cleanup;
    }
    */

    /**
     * gen OPERATOR_CERT
     */
    if (download_cert(curl, user, password) < 0) {
        snprintf(err_msg, sizeof(err_msg), "get cert failed\n");
        goto cleanup;
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    curl = curl_init();

    /**
     * change ca to VendorCA
     */
    if (change_ca(curl, "OperatorCA", user) < 0) {
        snprintf(err_msg, sizeof(err_msg), "change ca to \"OperatorCA\" failed\n");
        goto cleanup;
    }
    
    /**
     * get ca cert
     */
    if (download_ca_cert(curl, "OperatorCA")) {
        snprintf(err_msg, sizeof(err_msg), "get ca cert \"OperatorCA\" failed\n");
        goto cleanup;
    }
    ret = 0;

    /**
     * cleanup and free memory
     */
cleanup:
    if (ret) printf("%s", err_msg);
    //else printf("gen cert succ\n");
    if (!access(END_ENTITY_HTM_PATH, R_OK)) unlink(END_ENTITY_HTM_PATH);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: get_cert <user> <password>\n");
        return -1;
    }
    return get_cert_from_ejbca(argv[1], argv[2]);
}
