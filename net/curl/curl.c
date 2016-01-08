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
int write_data(void *buffer, int size, int nmemb, void *stream)
{
    FILE *fp = (FILE *)stream;
    fwrite(buffer, size, nmemb, fp);
    return size * nmemb;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    CURL *curl = NULL;
    CURLcode res;
    FILE *fp;

    fp = fopen("./post", "wb");
    if (!fp) return -1;
    
    curl = curl_easy_init();
    if (!curl) return -1;

    //curl_easy_setopt(curl, CURLOPT_URL, "http://172.21.34.55:8000");
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "username=admin&password=admin");

    /*
    curl_easy_setopt(curl, CURLOPT_URL, "https://172.21.34.81");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "login_name=debug&login_pwd=kzbyor5b1h09k7h3");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);  
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/home/anton/ftp/");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "mini_httpd.pem");  
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    */

    curl_easy_setopt(curl, CURLOPT_URL, "https://172.21.34.121:8443/ejbca/enrol/server.jsp");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "user=anton&password=123456&resulttype=1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);  
    curl_easy_setopt(curl, CURLOPT_SSLCERT, "file.crt.pem");
    curl_easy_setopt(curl, CURLOPT_SSLKEY, "file.key.pem");
    curl_easy_setopt(curl, CURLOPT_SSLCERTPASSWD, "ejbca");
    curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, "ejbca");
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/home/anton/ftp/");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "file.crt.pem");  
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        switch (res) {
            case CURLE_UNSUPPORTED_PROTOCOL:
                fprintf(stderr, "not support\n");
                break;
            case CURLE_COULDNT_CONNECT:
                fprintf(stderr, "not connect\n");
                break;
            case CURLE_HTTP_RETURNED_ERROR:
                fprintf(stderr, "http return error\n");
                break;
            case CURLE_READ_ERROR:
                fprintf(stderr, "read local file error\n");
                break;
            default:
                fprintf(stderr, "ret: %d", res);
                break;
        }
    }
    curl_easy_cleanup(curl);

    return rt;
}
