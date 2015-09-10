#include "dir.h"

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
/**
 * @brief get type of file 
 *
 * @param pathname [in] pathname of file
 *
 * @return file type, if succ; -1, if failed.
 */
file_type get_file_type(const char *pathname)
{
    struct stat st = {0};

    if (pathname == NULL) return F_UNKNOWN;
    if (lstat(pathname, &st) < 0) return F_UNKNOWN;

    if (S_ISREG(st.st_mode)) return F_REG;
    else if (S_ISDIR(st.st_mode)) return F_DIR;
    else if (S_ISCHR(st.st_mode)) return F_CHR;
    else if (S_ISBLK(st.st_mode)) return F_BLK;
    else if (S_ISFIFO(st.st_mode)) return F_FIFO;
    else if (S_ISSOCK(st.st_mode)) return F_SOCK;
    else if (S_ISLNK(st.st_mode)) return F_LNK;
    else return F_UNKNOWN;
}

/**
 * @brief get size of dir 
 *
 * @param pathname [in] path of dir
 *
 * @return size of dir, if succ; -1, if failed.
 */
int get_dir_size(const char *pathname)
{
    struct dirent *dir = NULL;
    struct stat st = {0};
    DIR *d = NULL;
    int total_size = 0;
    char buf[512] = {0};

    if (pathname == NULL) return -1;
    if (lstat(pathname, &st) < 0) return -1;
    if (!S_ISDIR(st.st_mode)) return -1;
    if ((d = opendir(pathname)) == NULL) return -1;

    while ((dir = readdir(d)) != NULL)
    {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;
        
        sprintf(buf, "%s/%s", pathname, dir->d_name);
        if (lstat(buf, &st) >= 0) 
        {
            if (S_ISDIR(st.st_mode)) 
            {
                total_size += get_dir_size(dir->d_name);
            }
            else total_size += st.st_size;
        }
    }
    closedir(d);

    return total_size;
}

/**
 * @brief get size of file
 *
 * @param pathname [in] pathname of file
 *
 * @return size, if succ; -1, if failed.
 */
int get_file_size(const char *pathname)
{
    struct stat st = {0};

    if (pathname == NULL) return -1;
    if (lstat(pathname, &st) < 0) return -1;

    if (S_ISDIR(st.st_mode)) return get_dir_size(pathname);
    else return st.st_size;

    return -1;
}

/**
 * @brief detect whether file is exist
 *
 * @param pathname [in] path of file
 *
 * @return 1, if exist; 0, if no exist; -1, if failed.
 */
int file_is_exist(const char *pathname)
{
    if (pathname == NULL) return -1;

    if (!access(pathname, F_OK)) return 1;

    return 0;
}

/**
 * @brief print file name of the dir
 *
 * @param pathname [in] path of dir
 */
void print_dir(const char *pathname)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat st = {0};

    if (pathname == NULL) return;

    if (lstat(pathname, &st) < 0) return;
    if (!S_ISDIR(st.st_mode)) return;

    if ((dir = opendir(pathname)) == NULL) return;
    if (chdir(pathname) !=0 ) return;

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);

    return;
}

/**
 * @brief create a multi directoy
 *
 * @param pathname [in] path of directory
 * @param mode     [in] mode of dir
 *
 * @return 0, if succ; -1, if failed.
 */
int make_dir(char *pathname, mode_t mode)
{
    int i;
    int size = (pathname != NULL) ? strlen(pathname) : -1;
    if (size <= 0) return -1;

    for (i = 1; i < size; i++)
    {
        if (pathname[i] == '/')
        {
            pathname[i] = '\0';
            if (file_is_exist(pathname) != 1) 
                mkdir(pathname, mode);
            pathname[i] = '/';
        }
    }
   
    if (file_is_exist(pathname) != 1) 
        mkdir(pathname, mode);
    
    if (file_is_exist(pathname) == 1) return 0;

    return -1;
}

/**
 * @brief remove directory
 *
 * @param pathname [in] path of directory
 *
 * @return 0, if succ; -1, if failed.
 */
int remove_dir(const char *pathname)
{
    struct dirent *dir = NULL;
    struct stat st = {0};
    DIR *d = NULL;
    char buf[512] = {0};

    if (pathname == NULL) return -1;
    if (lstat(pathname, &st) < 0) return -1;
    if (!S_ISDIR(st.st_mode)) return -1;
    if ((d = opendir(pathname)) == NULL) return -1;

    while ((dir = readdir(d)) != NULL)
    {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;
        
        sprintf(buf, "%s/%s", pathname, dir->d_name);
        if (lstat(buf, &st) >= 0) 
        {
            if (S_ISDIR(st.st_mode)) 
            {
                remove_dir(buf);
                rmdir(buf);
            }
            else remove(buf);
        }
    }
    if (file_is_exist(pathname) == 1) rmdir(pathname);
    closedir(d);
    
    if (file_is_exist(pathname) == 1) return -1;

    return 0;
}

/**
 * @brief remove file or dir
 *
 * @param pathname [in] path of file or directoy
 *
 * @return 0, if succ; -1, if failed.
 */
int remove_file(const char *pathname)
{
    struct stat st = {0};
    if (pathname == NULL) return -1;
    if (lstat(pathname, &st) < 0) return -1;

    if (S_ISDIR(st.st_mode)) return remove_dir(pathname);
    else remove(pathname);

    return 0;
}


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
