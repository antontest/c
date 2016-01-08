#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

size_t write_data(void* buffer,size_t size,size_t nmemb,void *stream)  
{  
    FILE *fptr = (FILE*)stream;  
    fwrite(buffer, size, nmemb, fptr);  
    return size * nmemb;  
}  

int main(int argc, char *argv[])
{
    CURL *curl;
    CURLcode res;

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;
    static const char buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);

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

    FILE *fp = NULL;
    if ((fp = fopen("post_result", "w")) == NULL)  
    {  
        fprintf(stderr,"fopen file error:%s\n", "post_result");  
        return -1;  
    }  
    curl = curl_easy_init();
    /* initialize custom header list (stating that Expect: 100-continue is not
       wanted */ 
    headerlist = curl_slist_append(headerlist, buf);
    if(curl) {
        /* what URL that receives this POST */ 
        curl_easy_setopt(curl, CURLOPT_URL, "https://192.168.0.105:8443/ejbca/certreq");
        if ( (argc == 2) && (!strcmp(argv[1], "noexpectheader")) )
            /* only disable 100-continue header if explicitly requested */ 
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_CAPATH, "/home/anton/download/req/file.sa.crt.pem");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "/home/anton/download/req/file.sa.crt.pem");
        curl_easy_setopt(curl, CURLOPT_SSLCERT, "/home/anton/download/req/file.cli.crt.pem"); 
        curl_easy_setopt(curl, CURLOPT_SSLKEY, "/home/anton/download/req/file.cli.key.pem"); 
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //对返回的数据进行操作的函数地址  
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); //这是write_data的第四个参数值  
        //curl_easy_setopt(curl, CURLOPT_HEADER, 1); 
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        /* Perform the request, res will get the return code */ 
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        else
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */ 
        curl_easy_cleanup(curl);

        /* then cleanup the formpost chain */ 
        curl_formfree(formpost);
        /* free slist */ 
        curl_slist_free_all (headerlist);
    }
    return 0;
}
