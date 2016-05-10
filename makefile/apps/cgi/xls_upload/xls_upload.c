/************************************************************************** 
  2007-1-5 11:42 establish by lzh.A cgi program. 
  get a file from user's explorer. 
 ***************************************************************************/  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cgi/cgi.h>
#include "../cgi_common.h"

#define DEAL_BUF_LEN 1024  
#define SIGN_CODE_LEN 100  
#define FILE_NAME_LEN 64  
#define FILE_SAVE_DIR "/home/anton/downloads/"  
static int file_upload(char *input, char *errmsg, cgi_form_entry_t *entry)
{
    FILE *fp        = NULL;
    int writed_size = 0;

    if (!strstr(cgi_form_info.multipart_content_type, "excel")) {
        ALERT("上传文件格式不正确!");
        HTML_BACK();
        return -1;
    }

    if ((fp = fopen(DFT_XLS_SAVE_PATH DFT_XLS_FILE_NAME, "wb+")) == NULL) {
        return -1;
    }

    writed_size = fwrite(input, sizeof(char), entry->file_size, fp);
    fflush(fp);
    fclose(fp);

    if (writed_size == entry->file_size) {
        ALERT("upload %s succ!", entry->file_name);
        HTML_GOTO("admin_page.html");
    } else {
        ALERT("%s upload failed!", entry->file_name);
        HTML_BACK();
    }

    return 0;
}

int main(void)  
{  
    cgi_func_tab_t func_tab[] = {
        {"po_maintain", KEY_IS_FILE, NULL, NULL},
        {"filename",   KEY_IS_VAR,  (void *)file_upload, NULL},
        {NULL}
    };

    CONTENT_TEXT;
    HTML_UTF8();
    cgi_t *cgi = cgi_create();
    cgi->parse_input(cgi);
    cgi->handle_entry(cgi, func_tab);
    cgi->destroy(cgi);

    return 0;
}  
