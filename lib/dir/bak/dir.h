#ifndef __DIR_H__
#define _DIR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <dirent.h>

typedef enum file_type {
    F_UNKNOWN   = -1, 
    F_REG       = 0 ,
    F_DIR           ,
    F_CHR           ,
    F_BLK           ,
    F_FIFO          ,
    F_LNK           ,
    F_SOCK
} file_type;

/**
 * @brief get type of file 
 *
 * @param pathname [in] pathname of file
 *
 * @return file type, if succ; -1, if failed.
 */
file_type get_file_type(const char *pathname);

/**
 * @brief get size of dir 
 *
 * @param pathname [in] path of dir
 *
 * @return size of dir, if succ; -1, if failed.
 */
int get_dir_size(const char *pathname);

/**
 * @brief get size of file
 *
 * @param pathname [in] pathname of file
 *
 * @return size, if succ; -1, if failed.
 */
int get_file_size(const char *pathname);

/**
 * @brief detect whether file is exist
 *
 * @param pathname [in] path of file
 *
 * @return 1, if exist; 0, if no exist; -1, if failed.
 */
int file_is_exist(const char *pathname);

/**
 * @brief print file name of the dir
 *
 * @param pathname [in] path of dir
 */
void print_dir(const char *pathname);

/**
 * @brief create a multi directoy
 *
 * @param pathname [in] path of directory
 * @param mode     [in] mode of dir
 *
 * @return 0, if succ; -1, if failed.
 */
int make_dir(char *pathname, mode_t mode);

#endif
