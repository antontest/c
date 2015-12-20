#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <cgi.h>
#include "ftp.h"

#define FTP_PATH "/home/anton/ftp/"
int get_ftp_file(char *outbuf, char *errmsg)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    int len = 0;

    if (!(dir = opendir(FTP_PATH))) return -1;
    while ((entry = readdir(dir)) != NULL) {
        if (!strncmp(entry->d_name, ".", 1)) continue;
        len += sprintf(outbuf + len, "\t\t%s<br>\n", entry->d_name);
    }
    closedir(dir);
    return 0;
}
