#include <mystring.h>

#define BUF_SIZE 512
char buf[BUF_SIZE] = {0};

/**
 * @brief int2string 
 *
 * @param num
 *
 * @return string
 */
char *int2string(const int num)
{
    sprintf(buf, "%d", num);
    
    return buf;
}

/**
 * @brief ip2string 
 *
 * @param ip
 *
 * @return string of ip
 */
char *ip2string(const unsigned char *ip)
{
    sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    
    return buf;
}

/**
 * @brief mac2string 
 *
 * @param mac
 *
 * @return string of mac
 */
char *mac2string(const unsigned char *mac)
{
    sprintf(buf, "%02x.%02x.%02x.%02x.%02x.%02x", mac[0], mac[1], mac[2], 
            mac[3], mac[4], mac[5]);
    
    return buf;
}

/**
 * @brief format2string 
 *
 * @param fmt
 * @param ...
 *
 * @return string
 */
char *format2string(const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vsnprintf(buf, sizeof(buf), fmt, arg);
    va_end(arg);

    return buf;
}
