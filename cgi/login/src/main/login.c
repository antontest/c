/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    char *input = NULL;
    int i = 0, j = 0;

    printf("Content-type: text/html\n\n");
    printf("The following is query reuslt:<br><br>");
    req_method = getenv("REQUEST_METHOD");
    printf("req_method:%s<br><br>", req_method);
    input = getcgidata(stdin, req_method);
    printf("input:%s<br><br>", input);
    for ( i = 9; i < (int)strlen(input); i++ ) {
        if ( input[i] == '&' ) {
            username[j] = '\0';
            break;
        }                   
        username[j++] = input[i];
    }

    for ( i = 19 + strlen(username), j = 0; i < (int)strlen(input); i++ ) {
        password[j++] = input[i];
    }
    password[j] = '\0';

    printf("Your Username is %s<br>Your Password is %s<br> \n", username, password);

    return rt;
}
