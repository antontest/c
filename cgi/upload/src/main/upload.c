/************************************************************************** 
  2007-1-5 11:42 establish by lzh.A cgi program. 
  get a file from user's explorer. 
 ***************************************************************************/  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <cgi.h>

#define DEAL_BUF_LEN 1024  
#define SIGN_CODE_LEN 100  
#define FILE_NAME_LEN 64  
#define FILE_SAVE_DIR "/home/anton/ftp"  
static int file_upload(char *input, char *errmsg, cgi_form_entry_t *entry)
{
    FILE *fp = NULL;
    char file_path[128] = {0};

    if (!entry->file_name) return -1;
    if (entry->file_size < 1) return 0;

    snprintf(file_path, sizeof(file_path), "%s/%s", FILE_SAVE_DIR, entry->file_name);
    if ((fp = fopen(file_path, "w")) == NULL) return -1;
    fwrite(input, sizeof(char), entry->file_size, fp);
    fclose(fp);
    ALERT("upload %s succ!", entry->file_name);

    return 0;
}

int main(void)  
{  
    cgi_func_tab_t func_tab[] = {
        {"upload",   KEY_IS_FILE, NULL, NULL},
        {"filename", KEY_IS_VAR,  NULL, (void *)file_upload},
        {NULL}
    };

    cgi_t *cgi = cgi_create();
    cgi->parse_form_input(cgi, func_tab);
    cgi->destroy(cgi);

    return 0;
}  
