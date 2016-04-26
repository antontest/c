/************************************************************************** 
  2007-1-5 11:42 establish by lzh.A cgi program. 
  get a file from user's explorer. 
 ***************************************************************************/  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <cgi/cgi.h>

#define DEAL_BUF_LEN 1024  
#define SIGN_CODE_LEN 100  
#define FILE_NAME_LEN 64  
#define FILE_SAVE_DIR "/home/anton/downloads/"  
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

static int todo(char *input, char *errmsg, cgi_form_entry_t *entry)
{
    FILE *fp = NULL;
    int len = 0;
    char c;

    if (!strcmp(input, "download")) {
        fp = fopen("/home/anton/downloads/ssl-client.c", "rb");
        if (!fp) return -1;

        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        if (len < 1) goto over;
        fseek(fp, 0, SEEK_SET);

        fprintf(stdout, "Content-Disposition:attachment;filename=%s\r\n", "test");
        cgi_header_content_length(len);
        cgi_header_content_type("application/octet-stream");

        c = getc(fp);
        while (!feof(fp)) {
            putc(c, stdout);
            c = getc(fp);
        }
        fflush(fp);
    } else if (!strcmp(input, "test")) {
        ALERT("test");
    }

over:
    if (fp) fclose(fp);
    return 0;
}

int main(void)  
{  
    cgi_func_tab_t func_tab[] = {
        {"upload",   KEY_IS_FILE, NULL, NULL},
        {"filename", KEY_IS_VAR,  NULL, (void *)file_upload},
        {"todo",     KEY_IS_VAR,  NULL, (void *)todo},
        {NULL}
    };

    CONTENT_TEXT;
    cgi_t *cgi = cgi_create();
    cgi->parse_form_input(cgi, func_tab);
    cgi->destroy(cgi);

    return 0;
}  
