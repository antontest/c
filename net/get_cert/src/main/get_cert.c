#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

static const char *edit_endentity_htm = "editendentity.htm";
static const char *cert = "operator_cert.pem";
size_t write_data(void* buffer,size_t size,size_t nmemb,void *stream)  
{  
    FILE *fptr = (FILE*)stream;  
    //fwrite(buffer, size, nmemb, fptr);  
    fputs(buffer, fptr);
    return size * nmemb;  
}  

enum html_input_type_t {
    HTML_UNKOWN = -1,
    HTML_INPUT = 0,
    HTML_SELECT
};
struct html_data_t {
    const char *name;
    enum html_input_type_t type;
    char value[56];
};

char *gen_requst_string(struct html_data_t *html_data)
{
    static char requst_data[1024] = {0};
    struct html_data_t *p = html_data;
    int len = 0;

    if (!html_data) return NULL;
    while (p->name != NULL) {
        if (p->type == HTML_UNKOWN) {
            p++;
            continue;
        }
        len += snprintf(requst_data + len, sizeof(requst_data) - len, "%s=%s&", p->name, p->value);
        p++;
    }
    requst_data[len - 1] = '\0';
    printf("%s\n", requst_data);

    return requst_data;
}

int get_end_entiry_data(CURL *curl, struct html_data_t *html_data)
{
    char buf[1024] = {0};
    char key[56]   = {0};
    char caid[28]  = {0};
    char *pos = NULL;
    FILE *fp  = NULL;
    char *pvalue;
    struct html_data_t *p = html_data;
    CURLcode res;

    fp = fopen(edit_endentity_htm, "w");
    if (!fp) return -1;

    curl_easy_setopt(curl, CURLOPT_URL, "https://192.168.0.106:8443/ejbca/adminweb/ra/editendentity.jsp?username=lte");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);   
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }

    fp = fopen(edit_endentity_htm, "r");
    if (!fp) return -1;
    while (fgets(buf, sizeof(buf), fp) != NULL && p->name != NULL) {
        if (p->type == HTML_UNKOWN)
            goto next;
        
        snprintf(key, sizeof(key), "name=\"%s\"", p->name);
        pos = strstr(buf, key);
        if (!pos) continue;
        
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

next:
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
     * change end entity status
     */
    p = html_data;
    while (p->name != NULL && p->type != HTML_UNKOWN) {
        if (!strcmp(p->name, "selectchangestatus")) {
            strcpy(p->value, "10");
            break;
        }
        p++;
    }

    /**
     * add selectca
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

int change_ca(CURL *curl, const char *ca_name, struct html_data_t *html_data)
{
    struct html_data_t *p = html_data;
    FILE *fp = NULL;
    char buf[1024] = {0};
    char key[128] = {0};
    char *pos = NULL;
    char *requst_data = NULL;
    CURLcode res;

    if (!ca_name || !p) return -1;
    fp = fopen(edit_endentity_htm, "r");
    if (!fp) return -1;

    snprintf(key, sizeof(key), "[CANAME] = \"%s\"", ca_name);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strstr(buf, ca_name);
        if (pos) break;
    }
    if (!pos) return -1;

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
        p->type = HTML_INPUT;
        strcpy(p->value, pos + 2);
        break;
    }
    if (!pos) return -1;

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
     * post to change end entity status
     */
    curl_easy_setopt(curl, CURLOPT_URL, "https://192.168.0.106:8443/ejbca/adminweb/ra/editendentity.jsp?username=lte");
    requst_data = gen_requst_string(html_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requst_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1); 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

int change_end_entiry_status(CURL *curl, struct html_data_t *html_data)
{
    struct html_data_t *p = html_data;
    char *post_data = NULL;
    CURLcode res;

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

    post_data = gen_requst_string(html_data);
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

int get_cert(CURL *curl)
{
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr  = NULL;
    struct curl_slist *headerlist  = NULL;
    const char buf[] = "Expect:";
    FILE *fp = NULL;
    CURLcode res;

    /* Fill in the user field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "user",
            CURLFORM_COPYCONTENTS, "lte",
            CURLFORM_END);

    /* Fill in the password field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "password",
            CURLFORM_COPYCONTENTS, "123456",
            CURLFORM_END);

    /* Fill in the file upload field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "pkcs10file",
            CURLFORM_FILE, "/home/anton/download/req/lte_req.pem",
            CURLFORM_END);

    /* Fill in the filename field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "filename",
            CURLFORM_COPYCONTENTS, "lte_req.pem",
            CURLFORM_END);

    /* Fill in the password field */ 
    curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "resulttype",
            CURLFORM_COPYCONTENTS, "1",
            CURLFORM_END);

    /**
     * open file for getting cert
     */
    if ((fp = fopen(cert, "w")) == NULL) {  
        fprintf(stderr,"fopen file error:%s\n", "post_result");  
        goto over;
    }  
    
    /**
     * post cert generate request
     */
    headerlist = curl_slist_append(headerlist, buf);
    curl_easy_setopt(curl, CURLOPT_URL, "https://192.168.0.106:8443/ejbca/certreq");
    //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
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

over:
    if (fp) fclose(fp);
    curl_formfree(formpost);
    curl_slist_free_all (headerlist);
    return 0;
}

int main(int argc, char *argv[])
{
    CURL *curl;
    struct html_data_t html_data[] = {
        {"action", HTML_INPUT},
        {"hiddenprofile", HTML_INPUT},
        {"username", HTML_INPUT},
        {"selectchangestatus", HTML_SELECT},
        {"buttonedituser", HTML_INPUT},
        {"textfieldpassword", HTML_INPUT},
        {"textfieldconfirmpassword", HTML_INPUT},
        {"checkboxcleartextpassword", HTML_INPUT},
        {"textfieldsubjectdn13", HTML_INPUT},
        {"textfieldsubjectdn15", HTML_INPUT},
        {"textfieldsubjectdn19", HTML_INPUT},
        {"selectcertificateprofile", HTML_SELECT},
        {"selecttoken", HTML_SELECT},
        {"selectca", HTML_UNKOWN},
        {"buttonedituser", HTML_INPUT},
        {NULL}
    };

    /**
     * libcurl init
     */
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "libcurl init failed\n");
        return -1;
    }

    /**
     * libcurl https SSL init
     */
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/home/anton/download/req/file.sa.crt.pem");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "/home/anton/download/req/file.sa.crt.pem");
    curl_easy_setopt(curl, CURLOPT_SSLCERT, "/home/anton/download/req/file.cli.crt.pem"); 
    curl_easy_setopt(curl, CURLOPT_SSLKEY, "/home/anton/download/req/file.cli.key.pem"); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);   
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);   

    /**
     * process html returned data
     */
    if (get_end_entiry_data(curl, html_data) < 0)
        goto cleanup;

    /**
     * post to change end entity status
     */
    if (change_end_entiry_status(curl, html_data) < 0) 
        goto cleanup;

    /**
     * change ca to VendorCA
     */
    if (change_ca(curl, "VendorCA", html_data) < 0)
        goto cleanup;

    /**
     * gen cert
     */
    if (get_cert(curl) < 0)
        goto cleanup;

    /**
     * change ca to VendorCA
     */
    change_ca(curl, "OperatorCA", html_data);

    /**
     * cleanup and free memory
     */
cleanup:
    if (!access(edit_endentity_htm, R_OK)) unlink(edit_endentity_htm);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
