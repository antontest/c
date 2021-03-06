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

#define DEAL_BUF_LEN 1024  
#define SIGN_CODE_LEN 100  
#define FILE_NAME_LEN 64  
#define FILE_SAVE_DIR "/home/anton/downloads/"  
static int file_upload(char *input, char *errmsg, cgi_form_entry_t *entry)
{
    FILE *fp = NULL;
    char file_path[128] = {0};
    int writed_size = 0;

    CONTENT_TEXT;
    if (!strcmp(entry->form_name, "upload_xls")) {
#define XLS_FILE_DIR "/home/anton/web/html/xls"
#define XLS_FILE_NAME "t.xls"
        if (access(XLS_FILE_DIR, W_OK)) {
            if (mkdir(XLS_FILE_DIR, 0777)) {
                ALERT("创建目录失败!");
                return -1;
            }
        }
        snprintf(file_path, sizeof(file_path), "%s/%s", XLS_FILE_DIR, XLS_FILE_NAME);
    } else {
        snprintf(file_path, sizeof(file_path), "%s/%s", FILE_SAVE_DIR, entry->file_name);
    }
    if ((fp = fopen(file_path, "w")) == NULL) return -1;

    writed_size = fwrite(input, sizeof(char), entry->file_size, fp);
    fclose(fp);
    if (writed_size == entry->file_size) {
        ALERT("upload %s succ!", entry->file_name);

        if (!strcmp(entry->form_name, "upload_xls")) {
            HTML_GOTO("setup.cgi?next_file=xls.html");
        }
    } else {
        ALERT("%s upload failed!", entry->file_name);
    }

    return 0;
}

static int todo(char *input, char *errmsg, cgi_form_entry_t *entry)
{
    if (!strcmp(input, "download")) {
        send_file_to_brower("/home/anton/下载/rofi-0.15.12.tar.gz");
    }

    return 0;
}

int main(void)  
{  
    cgi_func_tab_t func_tab[] = {
        {"upload",     KEY_IS_FILE, NULL               , NULL},
        {"filename",   KEY_IS_VAR,  (void *)file_upload, (void *)file_upload},
        {"todo",       KEY_IS_VAR,  NULL, (void *)todo},
        {"download",   KEY_IS_FILE, NULL               , NULL},
        {"todo",       KEY_IS_VAR,  NULL, (void *)todo},
        {"xls_upload", KEY_IS_FILE, NULL, NULL},
        {"filename",   KEY_IS_VAR,  NULL, (void *)file_upload},
        {"todo",       KEY_IS_VAR,  NULL, (void *)todo},
        {NULL}
    };

    cgi_t *cgi = cgi_create();
    cgi->parse_input(cgi);
    cgi->handle_entry(cgi, func_tab);
    cgi->destroy(cgi);

    return 0;
}  
