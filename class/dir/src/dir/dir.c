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
    char buf[1024] = {0};

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
            if (S_ISDIR(st.st_mode)) {
                total_size += get_dir_size(buf);
            } else {
                total_size += st.st_size;
            }
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

/**
 * @brief judge file permission
 *
 * @param path       [in] path
 * @param permission [in] string of permission, like "frwx" 
 *
 * @return 1, if have the permission, 0, if have no; -1, if failed
 */
int detect_permission(const char *path, const char *permission)
{
    int iperm = 0;
    
    if (path == NULL || permission == NULL) return -1;
    if (!strlen(permission) || !strlen(path)) return -1;
    
    while (*permission != '\0')
    {
        switch (*permission)
        {
            case 'r':
                iperm |= R_OK;
                break;
            case 'w':
                iperm |= W_OK;
                break;
            case 'x':
                iperm |= X_OK;
                break;
            case 'f':
                iperm |= F_OK;
                break;
            case '?':
            default:
                //printf("permission error!\n");
                return -1;
                break;
        }
        permission++;
    }

    if (!access(path, iperm)) return 1;
    return 0;
}

/**
 * @brief basename -- get basename of path, like /home/anton/test -- test
 *
 * @param path [in]  path
 * @param name [out] basename of path
 * @param size [in]  buffer size
 *
 * @return basename, if succ; -1, if failed
 */
char *basename(char *path, char *name, int size)
{
    char *p = path;

    if (path == NULL || name == NULL || size <= 0) return NULL;
    
    /* move to end of path */
    while (*p != '\0') p++;
    /* at last of char  */
    while (*(--p) == '/') ;

    /* move to pos of char '/' before last string of path */
    while (*p != '/' && p >= path) p--;
    /* string copy */
    while (*(++p) != '/' && size-- > 1) *name++ = *p;
    /* if string is all '/' */
    if (p == path) *name++ = '/';
    *name = '\0';

    return p;
}

/**
 * @brief dirname -- get dirname of path, like /home/anton/test -- test
 *
 * @param path [in]  path
 * @param name [out] dirname of path
 * @param size [in]  buffer size
 *
 * @return dirname, if succ; -1, if failed
 */
char *dirname(char *path, char *name, int size)
{
    char *p = path;
    char *pb = path;

    if (path == NULL || name == NULL || size <= 0) return NULL;
    
    /* move to end of path */
    while (*p != '\0') p++;
    /* at last of char  */
    while (*(--p) == '/' && p >= path) ;

    /* move to pos of char '/' before last string of path */
    while (*p != '/' && p > path) p--;
    while (*path == '/') path++;
    if (*path != *pb) *name++ = *pb;
    while (path < p && size-- > 1) *name++ = *path++;
    if (p == pb && path == pb) *name++ = '.';
    *name = '\0';

    return p;
}

