/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cgi.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
char* getcgidata(FILE* fp, char* requestmethod)
{
    char* input;
    int len;
    int size = 1024;
    int i = 0;

    if (!strcmp(requestmethod, "GET")) { //从这里可以看出来，GET在cgi中传递的Username="admin"&Password="aaaaa"被放置在环境变量QUERY_STRING中了。
        input = getenv("QUERY_STRING");
        return input;
    } else if (!strcmp(requestmethod, "POST")) {
        len = atoi(getenv("CONTENT_LENGTH"));
        printf("CONTENT_LENGTH:%d<br>", len);
        input = (char*)malloc(sizeof(char)*(len + 1));
        memset(input, 0, len + 1);
                                                                              
        if (len == 0) {
            input[0] = '\0';
            return input;
        }

        if (fread(input, sizeof(char), len, stdin) == len) {
            input[len] = '\0';
            printf("input:%s<br>", input);
        } else {
            printf("read error<br>");
        }
        while(1) { //从这里可以看出来，POST在cgi中传递的Username="admin"&Password="aaaaa"被写入stdin标准输入流中了。
            input[i] = (char)fgetc(fp);
            if (i == size) {
                input[i+1] = '\0';
                return input;
            }

            --len;
            if (feof(fp) || (!(len))) {
                i++;
                input[i] = '\0';
                return input;
            }
            i++;

        }
    }
    return NULL;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    char *req_method = NULL;
    char username[64] = {0};
    char password[64] = {0};
    char rem[2] = {0};
    char *input = NULL;
    cgi_t *cgi = cgi_create();
    data_parser_t data[] = {
        {"rem", rem, NULL, NULL},
        {"Username", username, NULL, NULL},
        {"Password", password, NULL, NULL},
        {NULL, NULL, NULL, NULL}
    };

    printf("Content-type: text/html\n\n");
    printf("The following is query reuslt:<br><br>");
    req_method = getenv("REQUEST_METHOD");
    printf("req_method:%s<br><br>", req_method);
    //input = getcgidata(stdin, req_method);
    input = cgi->get_data(cgi);
    printf("input:%s<br><br>", input);
    cgi->parser_data(cgi, data);
    printf("username:%s<br>", username);
    printf("password:%s<br>", password);
    printf("rem:%s<br><br>", rem);

    printf("Your Username is %s<br>Your Password is %s<br> \n", username, password);
    cgi->destroy(cgi);

    return rt;
}
