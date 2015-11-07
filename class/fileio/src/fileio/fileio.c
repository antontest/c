#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/errno.h>
#include <utils/utils.h>
#include "fileio.h"

#define DEFAULT_FILE_READ_BUFFER_SIZE (1024)
#define DEFAULT_FILE_WRITE_BUFFER_SIZE (1024)
typedef struct private_fileio_t private_fileio_t;
struct private_fileio_t {
    /**
     * @brief public interface
     */
    fileio_t public;

    /**
     * @brief save file name
     */
    char *filename;

    /**
     * @brief file handle
     */
    FILE *fp;

    /**
     * @brief buffer of file dealing
     */
    char *read_buffer;

    /**
     * @brief read buffer size
     */
    unsigned int read_buf_size;

    /**
     * @brief buffer of writing
     */
    char *write_buffer;

    /**
     * @brief write buffer size
     */
    unsigned int write_buf_size;
};

METHOD(fileio_t, open_, FILE *, private_fileio_t *this, const char *filename, const char *mode)
{
    if (this->fp != NULL) return this->fp;
    if (!filename || !mode) return NULL;

    this->fp = fopen(filename, mode);
    if (this->fp != NULL) {
        if (this->filename != NULL) free(this->filename);
        this->filename = strdup(filename);
    }

    return this->fp;
}

METHOD(fileio_t, ropen, FILE *, private_fileio_t *this, const char *mode)
{
    this->fp = freopen(this->filename, mode, this->fp);
    return this->fp;
}

METHOD(fileio_t, read_, char *, private_fileio_t *this)
{
    if (!this->fp) return NULL;
    if (!this->read_buffer) this->read_buffer = malloc(this->read_buf_size);
    if (!this->read_buffer) return NULL;

    return fgets(this->read_buffer, this->read_buf_size, this->fp);
}

METHOD(fileio_t, readn_, char *, private_fileio_t *this, int size)
{
    if (!this->fp) return NULL;
    if (!this->read_buffer) this->read_buffer = malloc(this->read_buf_size);
    if (!this->read_buffer) return NULL;

    fread(this->read_buffer, size < this->read_buf_size ? size : this->read_buf_size, 1, this->fp);
    return this->read_buffer;
}

METHOD(fileio_t, readrl_, char *, private_fileio_t *this, char *buffer, int size)
{
    if (!this->fp) return NULL;
    if (!buffer || size < 1) return NULL;
    return fgets(buffer, size, this->fp);
}

METHOD(fileio_t, readrn_, int, private_fileio_t *this, char *buffer, int size)
{
    if (!this->fp || !buffer || size < 1) return -1;
    return fread(buffer, size, 1, this->fp);
}

METHOD(fileio_t, write_, int, private_fileio_t *this, const char *buf)
{
    int write_size = 0;
    
    if (!buf || !this->fp) return -1;    
    write_size = fputs(buf, this->fp);
    fflush(this->fp);

    return write_size;
}

METHOD(fileio_t, vwrite_, int, private_fileio_t *this, const char *fmt, ...)
{
    va_list arg;
    int write_cnt = 0;

    if (!fmt || !this->fp) return -1;
    va_start(arg, fmt);
    write_cnt = vfprintf(this->fp, fmt, arg);
    fflush(this->fp);
    va_end(arg);

    return write_cnt;
}

METHOD(fileio_t, seek_, int, private_fileio_t *this, int offset, int whence)
{
    fseek(this->fp, offset, whence);
    return ftell(this->fp);
}

METHOD(fileio_t, truncate_, void, private_fileio_t *this, unsigned int length)
{
    ftruncate(fileno(this->fp), length);
    rewind(this->fp);
}

METHOD(fileio_t, close_, void, private_fileio_t *this)
{
    if (this->fp != NULL) fclose(this->fp);
    if (this->read_buffer != NULL) memset(this->read_buffer, 0, this->read_buf_size);
    if (this->write_buffer != NULL) memset(this->write_buffer, 0, this->write_buf_size);
}

METHOD(fileio_t, destroy_, void, private_fileio_t *this)
{
    if (this->fp           != NULL) fclose(this->fp);
    if (this->read_buffer  != NULL) free(this->read_buffer);
    if (this->write_buffer != NULL) free(this->write_buffer);
    if (!this->filename) free(this->filename);
    this->fp           = NULL;
    this->read_buffer  = NULL;
    this->write_buffer = NULL;
    this->filename     = NULL;

    free(this);
}

METHOD(fileio_t, get_read_buffer, char *, private_fileio_t *this)
{
    return this->read_buffer;
}

METHOD(fileio_t, get_write_buffer, char *, private_fileio_t *this)
{
    return this->write_buffer;
}

METHOD(fileio_t, get_file_handle, FILE *, private_fileio_t *this)
{
    return this->fp;
}

METHOD(fileio_t, get_file_no, int, private_fileio_t *this)
{
    return fileno(this->fp);
}

METHOD(fileio_t, get_filename, char *, private_fileio_t *this)
{
    return this->filename;
}

METHOD(fileio_t, get_file_size, int, private_fileio_t *this)
{
    int total_size = 0;
    int before_size = 0;

    before_size = ftell(this->fp);
    fseek(this->fp, 0, SEEK_END);
    total_size = ftell(this->fp);
    fseek(this->fp, before_size, SEEK_SET);

    return total_size;
}

METHOD(fileio_t, get_before_size, int, private_fileio_t *this)
{
    return ftell(this->fp);
}

METHOD(fileio_t, get_rest_size, int, private_fileio_t *this)
{
    int total_size = 0;
    int before_size = 0;

    before_size = ftell(this->fp);
    fseek(this->fp, 0, SEEK_END);
    total_size = ftell(this->fp);
    fseek(this->fp, before_size, SEEK_SET);

    return (total_size - before_size);
}

METHOD(fileio_t, get_read_buf_size, int, private_fileio_t *this)
{
    return this->read_buf_size;
}

METHOD(fileio_t, get_write_buf_size, int, private_fileio_t *this)
{
    return this->write_buf_size;
}

METHOD(fileio_t, set_read_buf_size, char *, private_fileio_t *this, unsigned int size)
{
    char *ptr = NULL;

    if (!this->read_buffer) {
        this->read_buffer = malloc(size);
        return this->read_buffer;
    }

    if (this->read_buf_size != size) {
        ptr = realloc(this->read_buffer, size);
        if (ptr != NULL) {
            this->read_buffer = ptr;
            this->read_buf_size = size;
        }
    }

    return this->read_buffer;
}

METHOD(fileio_t, set_write_buf_size, char *, private_fileio_t *this, unsigned int size)
{
    char *ptr = NULL;

    if (!this->write_buffer) {
        this->write_buffer = malloc(size);
        return this->write_buffer;
    }

    if (this->write_buf_size != size) {
        ptr = realloc(this->write_buffer, size);
        if (ptr != NULL) {
            this->write_buffer = ptr;
            this->write_buf_size = size;
        }
    }

    return this->write_buffer;
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
            .open     = _open_,
            .ropen    = _ropen,
            .read     = _read_,
            .readn    = _readn_,
            .readrl   = _readrl_,
            .readrn   = _readrn_,
            .write    = _write_,
            .vwrite   = _vwrite_,
            .seek     = _seek_,
            .truncate = _truncate_,
            .close    = _close_,
            .destroy  = _destroy_,

            .get_file_handle    = _get_file_handle,
            .get_file_no        = _get_file_no,
            .get_filename       = _get_filename,
            .get_file_size      = _get_file_size,
            .get_rest_size      = _get_rest_size,
            .get_before_size    = _get_before_size,
            .get_read_buf_size  = _get_read_buf_size,
            .get_write_buf_size = _get_write_buf_size,
            .get_read_buffer    = _get_read_buffer,
            .get_write_buffer   = _get_write_buffer,

            .set_read_buf_size  = _set_read_buf_size,
            .set_write_buf_size = _set_write_buf_size,
        },
        .fp = NULL,
        .filename     = NULL,
        .read_buffer  = NULL,
        .write_buffer = NULL,
        .read_buf_size  = DEFAULT_FILE_READ_BUFFER_SIZE,
        .write_buf_size = DEFAULT_FILE_WRITE_BUFFER_SIZE,
    );

    if (filename != NULL && mode != NULL) {
        this->fp = fopen(filename, mode);
        this->filename = strdup(filename);
    }

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

typedef struct private_cfg_t private_cfg_t;
struct private_cfg_t {
    /**
     * @brief public interface
     */
    cfg_t public;

    /**
     * deal with file, like open, read and wirte
     */
    fileio_t *file;
};

METHOD(cfg_t, get_value, char *, private_cfg_t *this, const char *keyname, const char *split)
{
    char *read_ptr = NULL;
    char *ret_ptr  = NULL;
    char *save_ptr = NULL;

    if (!keyname || !split) return NULL;
    while ((read_ptr = this->file->read(this->file)) != NULL) {
        ret_ptr = strtok_r(read_ptr, split, &save_ptr);
        if (!ret_ptr) continue;
        if (!strcmp(ret_ptr, keyname)) break;
    }

    ret_ptr = strtok_r(NULL, split, &save_ptr);
    if (!ret_ptr) return NULL;
    ret_ptr = strtok_r(ret_ptr, "\n", &save_ptr);
    
    return ret_ptr;
}

METHOD(cfg_t, set_value, void, private_cfg_t *this, const char *keyname, const char *split, const char *value)
{
    char *read_ptr  = NULL;
    char *ret_ptr   = NULL;
    char *save_ptr  = NULL;
    char *write_ptr = NULL;
    char *value_ptr = NULL;
    int  write_pos  = 0;

    if (!keyname || !split) return;
    if (!this->file->ropen(this->file, "r+")) return;

    this->file->set_write_buf_size(this->file, this->file->get_file_size(this->file) + (value != NULL ? strlen(value) : 0));
    write_ptr = this->file->get_write_buffer(this->file);
    if (!write_ptr) return;
    
    this->file->seek(this->file, 0, SEEK_SET);
    while ((read_ptr = this->file->read(this->file)) != NULL) {
        sprintf(write_ptr, "%s%c", read_ptr, '\0');
        ret_ptr = strtok_r(read_ptr, split, &save_ptr);
        if (!ret_ptr) continue;
        if (!strcmp(ret_ptr, keyname)) break;
    }
    
    ret_ptr = strtok_r(NULL, split, &save_ptr);
    if (!ret_ptr) return;
    ret_ptr = strtok_r(ret_ptr, "\n", &save_ptr);
    if (value != NULL && strlen(ret_ptr) == strlen(value) && !strcmp(ret_ptr, value)) return;

    write_pos = this->file->get_before_size(this->file) - strlen(write_ptr);
    value_ptr = strstr(write_ptr, ret_ptr);
    if (value != NULL) {
        strcpy(value_ptr, value);
        strcat(write_ptr, "\n");
    } else {
        memset(write_ptr, 0, this->file->get_write_buf_size(this->file));
    }

    this->file->readrn(this->file, write_ptr + strlen(write_ptr), this->file->get_rest_size(this->file));
    strcat(write_ptr, "\0");
    
    this->file->truncate(this->file, write_pos);
    this->file->seek(this->file, write_pos, SEEK_SET);
    this->file->write(this->file, write_ptr);
}

METHOD(cfg_t, cfg_destroy, void, private_cfg_t *this)
{
    if (this->file != NULL) this->file->close(this->file);
    free(this);
    this = NULL;
}

/**
 * @brief create config file dealing instance 
 *
 * @param filename   config file name
 */
cfg_t *create_cfg(const char *filename)
{
    private_cfg_t *this;

    if (filename == NULL && access(filename, F_OK) != 0) return NULL;
    INIT(this,
        .public = {
            .get_value = _get_value,
            .set_value = _set_value,
            .destroy = _cfg_destroy,
        },
        .file = create_fileio(filename, "r"),
    );
    if (!this->file) {
        free(this);
        return NULL;
    }

    return &this->public;
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

/**
 * @brief recover filename by file handle
 *
 * @param fp   file handle
 * @return     file name
 */
static char local_file_name[255];
char *recover_filename(FILE *fp)
{
    char fd_path[255] = {0};
    
    if (!fp) return NULL;
    sprintf(fd_path, "/proc/self/fd/%d", fileno(fp));
    if (readlink(fd_path, local_file_name, sizeof(local_file_name)) < 0) return NULL;
    return local_file_name;
}
