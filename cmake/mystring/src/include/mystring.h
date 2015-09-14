/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

/**
 * @brief int2string 
 *
 * @param num
 *
 * @return string
 */
char *int2string(const int num);

/**
 * @brief ip2string 
 *
 * @param ip
 *
 * @return string of ip
 */
char *ip2string(const unsigned char *ip);

/**
 * @brief mac2string 
 *
 * @param mac
 *
 * @return string of mac
 */
char *mac2string(const unsigned char *mac);

/**
 * @brief format2string 
 *
 * @param fmt
 * @param ...
 *
 * @return string
 */
char *format2string(const char *fmt, ...);
