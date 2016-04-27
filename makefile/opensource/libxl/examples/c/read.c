#include <stdio.h>
#include "libxl.h"

int main(int argc, char **argv)
{
    SheetHandle sheet;
    BookHandle book = xlCreateBook();

    if (!book) return -1;

    xlBookLoad(book, "example.xls");
    sheet = xlBookGetSheet(book, 3);
    int row = xlSheetLastRow(sheet);
    int col = xlSheetLastCol(sheet);
    int i = 0, j = 0;
    char *str = NULL;
    int num;

    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            if (i != 0 && ( j == 0 || j == 4 || j == 5)) {
                num = xlSheetReadNum(sheet, i, j, 0);
                if (num > 0)
                    printf("%d ", num);
                else printf(" ");
            } else {
                str = (char *)xlSheetReadStr(sheet, i, j, 0);
                if (!str) {
                } else {
                    printf("%s ", str);
                }
            }
        }
        printf("\n");
    }
    //xlSheetWriteNum(sheet, 1, 1, 1111, 0);
    //xlBookSave(book, "example.xls");
    xlBookRelease(book);

    return 0;
}
