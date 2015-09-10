#ifndef __CFG_H__
#define __CFG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <getopt.h>
#include <time.h>

/**
 * @brief trim blank char at left side
 *
 * @param s [in] string
 */
void l_trim(char *s);

/**
 * @brief trim blank char at right side
 *
 * @param s [in] string
 */
void r_trim(char *s);

/**
 * @brief trim blank char at middle string 
 *          if blank is more than two
 *
 * @param s [in] string
 */
void mid_trim(char *s);

/**
 * @brief remove all blank from string
 *
 * @param s [in] string
 */
void a_trim(char *s);

/**
 * @brief get config info from line buffer
 *
 * @param line  [in] config line info
 * @param split [in] search by split char 
 * @param name  [out] left value splited by char
 * @param value [out] right value splited by char
 * 
 * @return  num of split char's position; -1, if not found or error
 */
int cfg_line_split(char *line, const char *split, char **name, char **value);

/**
 * @brief get the value from config
 *
 * @param key_name [in] key name
 * @param split    [in] split char
 * @param value[]  [out] key value
 * @param path     [in] path of config file
 *
 * @return 0, if succ; -1, if fail
 */
int cfg_value_gain(const char *key_name, const char *split, char value[], const char *path) ;

/**
 * @brief set the value from config
 *
 * @param key_name [in] key name
 * @param split    [in] split char
 * @param value    [in] key value
 * @param path     [in] path of config file
 *
 * @return 0, if succ; -1, if fail
 */
int cfg_value_set(const char *key_name, const char *split, const char *value, const char *path);

/**
 * @brief get the value from file_name.ini
 *
 * @param app_name [in] app name
 * @param key_name [in] key name
 * @param value[]  [out] key value
 * @param path     [in] path of config file
 *
 * @return 0, if succ; -1, if fail
 */
int ini_value_gain(const char *app_name, const char *key_name, char value[], const char *path);

/**
 * @brief set the value from file_name.ini
 *
 * @param app_name [in] app name
 * @param key_name [in] key name
 * @param value    [in] key value
 * @param path     [in] path of config file
 *
 * @return 0, if succ; -1, if fail
 */
int ini_value_set(const char *app_name, const char *key_name, const char *value, const char *path);

#endif
