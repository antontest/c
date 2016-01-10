/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
size_t write_data(void* buffer,size_t size,size_t nmemb,void *stream)  
{  
    FILE *fptr = (FILE*)stream;  
    fwrite(buffer, size, nmemb, fptr);  
    return size * nmemb;  
}  

enum html_input_type_t {
    HTML_INPUT = 0,
    HTML_SELECT
};
struct html_data_t {
    const char *name;
    enum html_input_type_t type;
    char value[56];
};

char *process_data()
{
    char buf[1024] = {0};
    char key[56]   = {0};
    char caid[28]  = {0};
    char *ret = NULL;
    char *pos = NULL;
    FILE *fp  = NULL;
    int len   = 0;
    char *pvalue;
    static char post_data[1024]    = {0};
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
        {NULL}
    };
    struct html_data_t *p = html_data;

    fp = fopen("tmp.htm", "r");
    if (!fp) return NULL;

    while (fgets(buf, sizeof(buf), fp) != NULL && p->name != NULL) {
        snprintf(key, sizeof(key), "name=\"%s\"", p->name);
        pos = strstr(buf, key);
        if (!pos) continue;
        
        if (p->type == HTML_INPUT) {
            pos = strstr(pos, "value");
            if (!pos) goto over;

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
            if (!pos) goto over;

            pos = strstr(buf, "value");
            if (!pos) goto over;
            pos += strlen("value='");
            pvalue = p->value;
            while (*pos != '\'' && *pos != '\"') {
                *pvalue ++ = *pos++;
            }
            //printf("%s=%s\n", p->name, p->value);
        } else {
            goto over;
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
    if (!pos) goto over;

    p = html_data;
    while (p->name != NULL) {
        len += snprintf(post_data + len, sizeof(post_data) - len, "%s=%s&", p->name, p->value);
        p++;
    }
    len += snprintf(post_data + len, sizeof(post_data) - len, "selectca=%s", caid);
    printf("%s\n", post_data);
    ret = post_data;

over:
    fclose(fp);
    return ret;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    FILE *fp = NULL;
    char *post_data = NULL;
    CURL *curl;
    CURLcode res;

    if ((fp = fopen("tmp.htm", "w")) == NULL) {  
        fprintf(stderr,"fopen file error:%s\n", "post_result");  
        return -1;  
    }  
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "https://192.168.0.106:8443/ejbca/adminweb/ra/editendentity.jsp?username=lte");
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/home/anton/download/req/file.sa.crt.pem");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "/home/anton/download/req/file.sa.crt.pem");
    curl_easy_setopt(curl, CURLOPT_SSLCERT, "/home/anton/download/req/file.cli.crt.pem"); 
    curl_easy_setopt(curl, CURLOPT_SSLKEY, "/home/anton/download/req/file.cli.key.pem"); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);   
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);   
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);   
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        goto cleanup;
    }

    post_data = process_data();
    if (!post_data) goto cleanup;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1); 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

cleanup:
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return rt;
}
