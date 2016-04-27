#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <xls/xls.h>
#include <locale.h>
#include <iconv.h>

int main(int argc, char **argv)
{
    xls_t *xls = NULL;

    xls = xls_create();
    xls->open(xls, "tt.xls", O_WRONLY);
    xls->open_sheet(xls, 3);
    //xls->write(xls, 3, 0, "测试1");
    printf("row count: %d\n", xls->get_row_cnt(xls));
    //xls->insert_row(xls, 1, 1);
    //xls->delete_row(xls, 3, 5);
    //xls->save(xls, "tt.xls");

    /**
     * for read
     */
    char *str;
    while ((str = xls->enumerate(xls))) {
        printf("%s ", str);
    }
    xls->destroy(xls);
    
    return 0;
}
