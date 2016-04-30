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
        HTML_GOTO("setup.cgi?next_file=xls_upload.html");
        return -1;
    }

    return 0;
}

enum XLS_COLUMN_NAME{
    OPENING_DATE    = 7,
    SETTLEMENT_DATE = 13,
    SUBMITTAL_DATE  = 15,
};

int xls_data(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    char *xls_file = "xls/t.xls";
    xls_t *xls     = NULL;
    int col, row;
    char *str;
    int start_row = 3;

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

    for (row = start_row; row < xls->get_row_cnt(xls); row++) {
        printf("<tr>\n");
        printf("<td><input type=\"checkbox\" name=\"checkbox%d\" value=\"checkbox\" /></td>\n", row);
        for (col = 0; col < xls->get_col_cnt(xls); col++) {
            switch (col) {
                case OPENING_DATE:
                case SETTLEMENT_DATE:
                case SUBMITTAL_DATE:
                    printf("<td EditType=\"Date\">");
                    break;
                default:
                    printf("<td EditType=\"TextBox\">");
                    break;
            }

            str = xls->read(xls, row, col);
            if (str) {
                printf("%s", str);
            } else {
                printf(" ");
            }
            printf("</td>\n");

        }
        printf("</tr>\n");
    }

    xls->destroy(xls);
    
    return 0;
}

int xls_change_data(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    char buf[100]  = {0};
    snprintf(buf, sizeof(buf), "echo \"%s\" > /home/anton/cgi.log", outbuf);
    if (system(buf)) {}
    ALERT("%s", outbuf);
    return 0;
}
