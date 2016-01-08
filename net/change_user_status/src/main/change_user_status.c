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

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "https://192.168.0.105:8443/ejbca/adminweb/ra/editendentity.jsp");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "action=edituser&hiddenprofile=1&username=lte&selectchangestatus=10&buttonedituser=Save&textfieldpassword=123456&textfieldconfirmpassword=123456&radiomaxfailedlogins=unlimited&checkboxcleartextpassword=true&textfieldsubjectdn13=lte&textfieldsubjectdn15=Sercomm&textfieldsubjectdn19=CN&selectcertificateprofile=1&selectca=1866329767&selecttoken=1"); 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/home/anton/download/req/file.sa.crt.pem");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "/home/anton/download/req/file.sa.crt.pem");
    curl_easy_setopt(curl, CURLOPT_SSLCERT, "/home/anton/download/req/file.cli.crt.pem"); 
    curl_easy_setopt(curl, CURLOPT_SSLKEY, "/home/anton/download/req/file.cli.key.pem"); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_POST, 1); 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);   
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);   
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);   
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return rt;
}
