#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cgi.h>
#include "ftp.h"

#define FTP_PATH "/home/anton/web/html/download"
int get_ftp_dir(char *outbuf, char *errmsg, cgi_form_entry_t *form_entry)
{
    if (form_entry->next_path != NULL && strcmp(form_entry->next_path, ".")) {
        while (*form_entry->next_path == '.') form_entry->next_path++;
        while (*form_entry->next_path == '/') form_entry->next_path++;
        strcpy(outbuf, form_entry->next_path);
    }
    return 0;
}
int get_ftp_file_name(char *outbuf, char *errmsg, cgi_form_entry_t *form_entry)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    int len = 0;
    char path[256] = {0};

    if (!form_entry->next_path) {
        form_entry->next_path = ".";
        snprintf(path, sizeof(path), "%s", FTP_PATH);
    }
    else snprintf(path, sizeof(path), "%s/%s", FTP_PATH, form_entry->next_path);
    if (!(dir = opendir(path))) return -1;
    while ((entry = readdir(dir)) != NULL) {
        if (!strncmp(entry->d_name, ".", 1)) continue;
        if (entry->d_type == DT_DIR) 
            len += sprintf(outbuf + len, "\t\t\t<a href=\"setup.cgi?next_file=ftp.htm&next_path=%s/%s\">%s/</a><br>\n", form_entry->next_path, entry->d_name, entry->d_name);
        else if (entry->d_type == DT_REG)
            len += sprintf(outbuf + len, "\t\t\t<a href=\"download/%s/%s\">%s</a><br>\n", form_entry->next_path, entry->d_name, entry->d_name);
    }
    closedir(dir);
    return 0;
}
static long file_size(char *filename)
{
    long filesize = -1;      
    struct stat statbuff;  
    if(stat(filename, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}

int get_ftp_file_size(char *outbuf, char *errmsg, cgi_form_entry_t *form_entry)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    int len = 0;
    char file_path[128] = {0};
    char path[256] = {0};

    if (!form_entry->next_path) {
        form_entry->next_path = ".";
        snprintf(path, sizeof(path), "%s", FTP_PATH);
    }
    else snprintf(path, sizeof(path), "%s/%s", FTP_PATH, form_entry->next_path);
    if (!(dir = opendir(path))) return -1;
    while ((entry = readdir(dir)) != NULL) {
        if (!strncmp(entry->d_name, ".", 1)) continue;
        if (entry->d_type == DT_DIR) 
            len += sprintf(outbuf + len, "\t\t\t..<br>\n");
        else if (entry->d_type == DT_REG) {
            snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
            len += sprintf(outbuf + len, "\t\t\t%ld<br>\n", file_size(file_path));
        }
    }
    closedir(dir);
    return 0;
}
