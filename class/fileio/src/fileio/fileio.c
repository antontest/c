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
#include <utils/utils.h>
#include "fileio.h"

#define DEFAULT_FILE_BUFFER_SIZE (1024)
typedef struct private_fileio_t private_fileio_t;
struct private_fileio_t {
    /**
     * @brief public interface
     */
    fileio_t public;

    /**
     * @brief file handle
     */
    FILE *fp;

    /**
     * @brief buffer of file dealing
     */
    char *read_buffer;
};

METHOD(fileio_t, open_, FILE *, private_fileio_t *this, const char *filename, const char *mode)
{
    if (!filename || !mode) return NULL;
    this->fp = fopen(filename, mode);
    return this->fp;
}

METHOD(fileio_t, read_, char *, private_fileio_t *this)
{
    if (!this->fp) return NULL;
    if (!this->read_buffer) this->read_buffer = malloc(DEFAULT_FILE_BUFFER_SIZE);
    if (!this->read_buffer) return NULL;

    return fgets(this->read_buffer, DEFAULT_FILE_BUFFER_SIZE, this->fp);
}

METHOD(fileio_t, write_, int, private_fileio_t *this, const char *buf)
{
    if (!buf || !this->fp) return -1;
    
    return fputs(buf, this->fp);
}

METHOD(fileio_t, close_, void, private_fileio_t *this)
{
    if (this->read_buffer != NULL) free(this->read_buffer);
    this->read_buffer = NULL;

    if (this->fp != NULL) fclose(this->fp);
    this->fp = NULL;

    free(this);
}

/**
 * @brief create fileio instance
 *
 * @param filename     file name
 */
fileio_t *create_fileio(const char *filename, const char *mode)
{
    private_fileio_t *this;

    INIT(this,
        .public = {
            .open  = _open_,
            .read  = _read_,
            .write = _write_,
            .close = _close_,
        },
        .fp = NULL,
        .read_buffer = NULL,
    );

    if (filename != NULL && mode != NULL)
        this->fp = fopen(filename, mode);
    return &this->public;
}

/**
 * @brief trim blank char at left side
 *
 * @param s [in] string
 */
void l_trim(char *s)
{
    char *p = s, *q = s;;

    /* avoid NULL pointer error and init */
    if (s == NULL) return ;

    /* left shift */
    while(*p == ' ') p++;
    while((*q++ = *p++) != '\0') ;
}

/**
 * @brief trim blank char at right side
 *
 * @param s [in] string
 */
void r_trim(char *s)
{
    char *p = s;
    if (p == NULL) return ;

    /**
     * 1. move at the end of string
     * 2. right shitf until char not equal to blank
     * 3. add '\0' at current pointer position
     */
    while (*p != '\0') p++;
    while (--p != s && *p == ' ') ;
    *(++p) = '\0';
}

/**
 * @brief trim blank char at middle string 
 *          if blank is more than two
 *
 * @param s [in] string
 */
void mid_trim(char *s)
{
    char *a = s, *b = s;
    if (a == NULL) return;
    
    /* right shift until not equal to blank */
    while (*a != '\0' && *a == ' ') a++;
    *b = *a;

    /* remove blank in middle string */
    while (*(++a) != '\0') {
        if (*a != ' ' || *b != ' ')
            *(++b) = *a;
    }

    /* avoid the last char is blank */
    if (*b == ' ') *b = '\0';
    else *(++b) = '\0';
}

/**
 * @brief remove all blank from string
 *
 * @param s [in] string
 */
void a_trim(char *s)
{
    char *b = s, *p = s;
    if (p == NULL) return ;

    /* remove all blank from string */
    while (*p != '\0') {
        if (*p != ' ') *b++ = *p;
        p++;
    }
    *b = '\0';
}

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
int cfg_line_split(char *line, const char *split, char **name, char **value)
{
    char ch = split[0];
    char *p = line;

    /* avoid NULL pointer error */
    if (p == NULL || split == NULL) return -1;
    
    /* remove blank */
    /* mid_trim(p); */

    /* search split char */
    while (*p != '\0' && *p != ch) p++;

    /* decide whether found split or not. if not, return */
    if (*p == '\0') return -1;

    /* get value */
    *name = line;
    *p = '\0';
    *value = ++p;

    return (p - line - 1);
}

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
int cfg_value_gain(const char *key_name, const char *split, char value[], const char *path) 
{
    FILE *fp = NULL;        /* file */
    char *k_name = NULL;
    char *k_value = NULL;   /* tmp variable */
    char buf[1024] = {0};   /* read buffer */
    int size = sizeof(buf); /* size of read buffer */
    int len = 0;            
    int rt = -1;            /* return value */

    /* guarantee safty */
    if (path == NULL || key_name == NULL || split == NULL) goto free;

    /* open file */
    if ((fp = fopen(path, "r")) == NULL) goto free;

    /* search key name */
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        
        /* split line */
        if (cfg_line_split(buf, split, &k_name, &k_value) < 0)
            continue;
        
        mid_trim(k_name); /* remove all blank */
        if (strncmp(k_name, key_name, strlen(k_name))) 
            continue;

        /* remove '\n' or '\t' at end of k_value */
        if (value == NULL) break;
        len = strlen(k_value);
        if (k_value[len - 1] == '\n' || k_value[len - 1] =='\t')
            k_value[len - 1] = '\0';

        /* cpoy value */
        strcpy(value, k_value);
        rt = 0;

        break;
    }

free:
    /* close file */
    if (fp != NULL) fclose(fp);
    fp = NULL;

    return rt;
}

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
int cfg_value_set(const char *key_name, const char *split, const char *value, const char *path) 
{
    FILE *fp = NULL;        /* file */
    char buf[1024] = {0};   /* read buffer */
    char *sum_buf = NULL;   /* save buffer */
    char *k_name = NULL;
    char *k_value = NULL;   /* tmp variable */
    int size = sizeof(buf); /* size of read buffer */
    int cfg_size = 0;       /* size of config file */
    int rt = -1;            /* return value */
    int is_set = 0;         /* is found key_name and set */
    
    /* guarantee safty */
    if (path == NULL || key_name == NULL || split == NULL || value == NULL) goto free;

    /* open file */
    if ((fp = fopen(path, "r+")) == NULL) goto free;

    /* gain size of file */
    fseek(fp, 0, SEEK_END);
    cfg_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (cfg_size < 2) goto free;

    /* apply for memory */
    sum_buf = (char *)malloc(cfg_size + strlen(value) + 1);
    if (sum_buf == NULL) goto free;
    strcpy(sum_buf, "");

    /* search key name */
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        /* splite line, if failed, then save and continue */
        if (cfg_line_split(buf, split, &k_name, &k_value) < 0) {
            strcat(sum_buf, buf);
            continue;
        }
        
        /* strcat k_name and "=" */
        strcat(sum_buf, k_name);
        strcat(sum_buf, "=");

        /* macth key name */
        mid_trim(k_name); /* remove all blank */
        if (strncmp(k_name, key_name, strlen(k_name))){
            strcat(sum_buf, k_value);
            continue;
        }

        /* if value equal to t_value, then nothing need to change */
        if (!strncmp(k_value, value, strlen(k_value)))
            break;

        /* set new value */
        strcat(sum_buf, value);
        strcat(sum_buf, "\n");
        is_set = 1;
        
        break;
    }

    /* if not found, then over */
    if (!is_set) goto free;

    /* save the rest of the content */
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        strcat(sum_buf, buf);
    }

    /* write into file */
    if (fp != NULL) fclose(fp);
    fp = NULL;
    if ((fp = fopen(path, "w")) == NULL) goto free;
    fputs(sum_buf, fp);
    rt = 0;

free:
    /* close file */
    if (fp != NULL) fclose(fp);
    fp = NULL;

    /* free memory */
    if (sum_buf != NULL) free(sum_buf);
    sum_buf = NULL;

    return rt;
}


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
int ini_value_gain(const char *app_name, const char *key_name, char value[], const char *path) 
{
    FILE *fp = NULL;        /* file */
    char buf[1024] = {0};   /* read buffer */
    char *app = NULL;
    char *k_name = NULL;
    char *k_value = NULL;   /* tmp variable */
    int size = sizeof(buf); /* size of read buffer */
    int len = 0;
    int rt = -1;            /* return value */

    /* guarantee safty */
    if (path == NULL || key_name == NULL || app_name == NULL) goto free;

    /* app name */
    app = (char *)malloc(strlen(app_name) + 2 + 1);
    if (app == NULL) goto free;
    sprintf(app, "[%s]", app_name);
    strcat(app, "\0");
    len = strlen(app);

    /* open file */
    if ((fp = fopen(path, "r")) == NULL) goto free;

    /* search app name */
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        /* decide whether is comment or not */
        l_trim(buf);
        if (buf[0] == '#' || buf[0] == ';') continue;

        /* search app */
        if (!strncmp(buf, app, len)) {
            break;
        }
    }

    /* goto free when fp at the end of file */
    if (feof(fp)) goto free;

    /* search key name */
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        /* decide whether is comment or not */
        l_trim(buf);
        if (buf[0] == '#' || buf[0] == ';') continue;
        if (buf[0] == '[') break;

        /* split line */
        if (cfg_line_split(buf, "=", &k_name, &k_value) < 0)
            continue;
        
        mid_trim(k_name); /* remove all blank */
        if (strncmp(k_name, key_name, strlen(k_name))) 
            continue;

        /* remove '\n' or '\t' at end of k_value */
        if (value == NULL) break;
        len = strlen(k_value);
        if (k_value[len - 1] == '\n' || k_value[len - 1] =='\t')
            k_value[len - 1] = '\0';

        /* cpoy value */
        strcpy(value, k_value);
        rt = 0;

        break;
    }

free:
    /* close file */
    if (fp != NULL) fclose(fp);
    fp = NULL;

    /* free memory */
    if (app != NULL) free(app);
    app = NULL;

    return rt;
}

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
int ini_value_set(const char *app_name, const char *key_name, const char *value, const char *path) 
{
    FILE *fp = NULL;        /* file */
    char *app = NULL;
    char *k_name = NULL;
    char *k_value = NULL;   /* tmp variable */
    char buf[1024] = {0};   /* read buffer */
    char *sum_buf = NULL;   /* ini file buffer */
    int size = sizeof(buf); /* size of read buffer */
    int ini_size = 0;
    int len = 0;
    int rt = -1;            /* return value */
    int is_set = 0;         /* is found key_name and set */

    /* guarantee safty */
    if (path == NULL ||  app_name == NULL) goto free;

    /* app name */
    app = (char *)malloc(strlen(app_name) + 2 + 1);
    if (app == NULL) goto free;
    sprintf(app, "[%s]", app_name);
    strcat(app, "\0");
    len = strlen(app);

    /* open file */
    if ((fp = fopen(path, "r")) == NULL) goto free;

    /* gain size of file */
    fseek(fp, 0, SEEK_END);
    ini_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* apply for memory */
    sum_buf = (char *)malloc(ini_size + len  + \
            (key_name != NULL ? strlen(key_name) : 0) + \
            (value != NULL ? strlen(value) : 0) + 1);
    if (sum_buf == NULL) goto free;
    strcpy(sum_buf, "");

    /* search app name */
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        /* decide whether is comment or not */
        l_trim(buf);
        if (buf[0] == '#' || buf[0] == ';') continue;

        /* search app */
        if (!strncmp(buf, app, len)) {
            break;
        }

        strcat(sum_buf, buf);
    }

    /* when not arived at the end of file */
    if (!feof(fp)) {
        /* if key name == NULL, remove app */
        if (key_name == NULL) {
            while(fgets(buf, size, fp) != NULL) {
                l_trim(buf);
                if (buf[0] == '[') break;
        }

        if (!feof(fp)) strcat(sum_buf, buf);
        goto rest;

        } else {
            strcat(sum_buf, buf);
        }
    }

    /* when fp at the end of file , add app and key at end*/
    else {
        /* if value==NULL and fp is at end of file, then over */
        if (key_name == NULL || value == NULL) {
            goto free;
        }

        strcat(sum_buf, app);
        strcat(sum_buf, "\n");

        /* strcat k_name and "=" */
        strcat(sum_buf, key_name);
        strcat(sum_buf, "=");
        
        /* set new value */
        strcat(sum_buf, value);
        strcat(sum_buf, "\n");

        goto write;
    }

    /* search key name */
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        /* decide whether is comment or not */
        l_trim(buf);
        if (buf[0] == '#' || buf[0] == ';') {
            strcat(sum_buf, buf);
            continue;
        }
        if (buf[0] == '[') break;

        /* split line */
        if (cfg_line_split(buf, "=", &k_name, &k_value) < 0) {
            strcat(sum_buf, buf);
            continue;
        }
        
        /* macth key name */
        mid_trim(k_name); /* remove all blank */
        if (strncmp(k_name, key_name, strlen(k_name))){
            strcat(sum_buf, k_name);
            strcat(sum_buf, "=");
            strcat(sum_buf, k_value);

            continue;
        }
        
        /* if value equal to NULL, then delete this key and value */
        if (value == NULL) {
            is_set = 1;
            break;
        }

        /* strcat k_name and "=" */
        strcat(sum_buf, k_name);
        strcat(sum_buf, "=");

        /* if value equal to t_value, then nothing need to change */
        if (!strncmp(k_value, value, strlen(k_value))) {
            is_set = -1;
            break;
        }
        
        /* set new value */
        strcat(sum_buf, value);
        strcat(sum_buf, "\n");
        is_set = 1;

        break;
    }

    /* if not found, then add at the end of the app */
    if (is_set == 0 && value != NULL) {
        /* strcat k_name and "=" */
        strcat(sum_buf, key_name);
        strcat(sum_buf, "=");
        
        /* set new value */
        strcat(sum_buf, value);
        strcat(sum_buf, "\n");

        strcat(sum_buf, buf);
    }
    
    if (is_set == -1 || (is_set == 0 && value == NULL)) /* -1, then no need to change */
        goto free;

    /* save the rest of the content */
rest:
    while (fgets(buf, size, fp) != NULL) { /* read one line */
        strcat(sum_buf, buf);
    }

    /* write into file */
write:
    if (fp != NULL) fclose(fp);
    fp = NULL;
    if ((fp = fopen(path, "w")) == NULL) goto free;
    fputs(sum_buf, fp);
    rt = 0;

free:
    /* close file */
    if (fp != NULL) fclose(fp);
    fp = NULL;

    /* free memory */
    if (app != NULL) free(app);
    app = NULL;
    if (sum_buf != NULL) free(sum_buf);
    sum_buf = NULL;

    return rt;
}
