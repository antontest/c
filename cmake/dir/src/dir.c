#include "dir.h"

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    file_type type = -1;

    make_dir(agrv[1], 0775);
    if (file_is_exist(agrv[1]) != 1) return -1;

    printf("file exist: %s\n", agrv[1]);
    printf("access: %d\n", detect_permission(agrv[1], agrv[2]));

    type = get_file_type(agrv[1]);
    if (type == F_DIR) printf("dir\n");
    else if (type == F_REG) printf("reg\n");
    else if (type == F_FIFO) printf("fifo\n");
    else if (type == F_LNK) printf("lnk\n");
    else printf("other\n");

    printf("file size: %d\n", get_file_size(agrv[1]));
    remove_file("test");

    return rt;
}
