#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cgi/cgi.h>
#include <xls/xls.h>

#define XLS_FILE "/home/anton/web/html/xls/t.xls"
int xls_file(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    if (access(XLS_FILE, R_OK)) {
        ALERT("Execl文档不存在，请先上传!");
        HTML_GOTO("setup.cgi?next_file=xls_upload.htm");
        return -1;
    }

    HTML_GOTO("setup.cgi?next_file=xls.html");

    return 0;
}

int xls_data(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    char *xls_file = "t.xls";
    xls_t *xls     = NULL;
    int col, row;
    char *str;

    xls = xls_create();
    if (!xls) {
        ALERT("内存溢出!");
        return -1;
    }

    if (xls->open(xls, xls_file, O_RDONLY) < 0) {
        ALERT("xls文档不存在!");
        return -1;
    }

    if (xls->open_sheet(xls, 3)) {
        ALERT("xls文档解析失败!");
        return -1;
    }

    if (xls->get_row_cnt(xls) < 1 || xls->get_col_cnt(xls) < 1) {
        ALERT("xls文档为空!");
        return -1;
    }

    for (row = 3; row < xls->get_row_cnt(xls); row++) {
        printf("<tr>");
        for (col = 0; col < xls->get_col_cnt(xls); col++) {
            printf("<td>");
            str = xls->read(xls, row, col);
            if (str) {
                printf("%s", str);
            } else {
                printf(" ");
            }
            printf("</td>");

        }
        printf("</tr>");
    }

    xls->destroy(xls);
    
    return 0;
}
