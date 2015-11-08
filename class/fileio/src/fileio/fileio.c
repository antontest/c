#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
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

    ignore_result(fread(this->read_buffer, size < this->read_buf_size ? size : this->read_buf_size, 1, this->fp));
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
    if (ftruncate(fileno(this->fp), length) < 0) return;
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
    return (int)ftell(this->fp);
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

METHOD(fileio_t, is_endof, int, private_fileio_t *this)
{
    return feof(this->fp); 
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
            .is_endof           = _is_endof,

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

    /**
     * ini config split
     */
    char split[10];
};

METHOD(cfg_t, get_cfg_value, char *, private_cfg_t *this, const char *keyname)
{
    char *read_ptr = NULL;
    char *ret_ptr  = NULL;
    char *save_ptr = NULL;
    const char *split = this->split;

    if (!keyname || !split) return NULL;
    while ((read_ptr = this->file->read(this->file)) != NULL) {
        ret_ptr = strtok_r(read_ptr, split, &save_ptr);
        if (!ret_ptr) continue;
        if (!strcmp(ret_ptr, keyname)) break;
    }

    if (strlen(ret_ptr) != strlen(keyname)) return NULL;
    ret_ptr = strtok_r(NULL, split, &save_ptr);
    if (!ret_ptr) return NULL;
    ret_ptr = strtok_r(ret_ptr, "\n", &save_ptr);
    
    return ret_ptr;
}

METHOD(cfg_t, set_cfg_value, void, private_cfg_t *this, const char *keyname, const char *value)
{
    char *read_ptr  = NULL;
    char *ret_ptr   = NULL;
    char *save_ptr  = NULL;
    char *write_ptr = NULL;
    char *value_ptr = NULL;
    const char *split = this->split;
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

METHOD(cfg_t, set_cfg_split, void, private_cfg_t *this, const char *split)
{
    if (split != NULL) strcpy(this->split, split);
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
            .get_value = _get_cfg_value,
            .set_value = _set_cfg_value,
            .set_split = _set_cfg_split,
            .destroy = _cfg_destroy,
        },
        .file = create_fileio(filename, "r"),
        .split = ":=",
    );
    if (!this->file) {
        free(this);
        return NULL;
    }

    return &this->public;
}


/**
 * @brief remove all blank from string
 *
 * @param s [in] string
 */
static void a_trim(char *s)
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

typedef struct private_ini_t private_ini_t;
struct private_ini_t {
    /**
     * @brief public interface
     */
    ini_t public;

    /**
     * deal with file, like open, read and wirte
     */
    fileio_t *file;

    /**
     * ini config split
     */
    char split[10];
};

METHOD(ini_t, get_ini_value, char *, private_ini_t *this, const char *appname, const char *keyname)
{
    char *read_ptr = NULL;
    char *ret_ptr  = NULL;
    char *save_ptr = NULL;
    char ini_app_name[48] = {0};
    const char *split = this->split;

    if (!appname || !keyname) return NULL;
    sprintf(ini_app_name, "[%s]", appname);

    while ((read_ptr = this->file->read(this->file)) != NULL) {
        a_trim(read_ptr);
        read_ptr = strtok(read_ptr, "\n");
        if (!strcmp(read_ptr, ini_app_name)) break;
    }
    if (this->file->is_endof(this->file)) return NULL;

    while ((read_ptr = this->file->read(this->file)) != NULL) {
        if (strstr(read_ptr, "[")) return NULL;
        ret_ptr = strtok_r(read_ptr, split, &save_ptr);
        if (!ret_ptr) continue;
        if (strlen(ret_ptr) == strlen(keyname) && !strcmp(ret_ptr, keyname)) break;
    }
    if (strlen(ret_ptr) != strlen(keyname) || strcmp(ret_ptr, keyname)) return NULL;

    ret_ptr = strtok_r(NULL, split, &save_ptr);
    if (!ret_ptr) return NULL;
    ret_ptr = strtok_r(ret_ptr, "\n", &save_ptr);
    
    return ret_ptr;
}

METHOD(ini_t, set_ini_value, void, private_ini_t *this, const char *appname, const char *keyname, const char *value)
{
    char *read_ptr  = NULL;
    char *ret_ptr   = NULL;
    char *save_ptr  = NULL;
    char *write_ptr = NULL;
    char *value_ptr = NULL;
    char ini_app_name[48] = {0};
    const char *split = this->split;
    int  write_pos  = 0;

    if (!appname) return;
    if (!this->file->ropen(this->file, "r+")) return;
    sprintf(ini_app_name, "[%s]", appname);

    this->file->set_write_buf_size(this->file, this->file->get_file_size(this->file) + strlen(appname) + 3 + (keyname != NULL ? strlen(keyname) : 0) + 1 + (value != NULL ? strlen(value) : 0) + 1);
    write_ptr = this->file->get_write_buffer(this->file);
    if (!write_ptr) return;
    
    this->file->seek(this->file, 0, SEEK_SET);
    while ((read_ptr = this->file->read(this->file)) != NULL) {
        a_trim(read_ptr);
        read_ptr = strtok(read_ptr, "\n");
        if (!strcmp(read_ptr, ini_app_name)) break;
    }

    if (this->file->is_endof(this->file)) {
        if (!value) return;
        sprintf(write_ptr, "[%s]\n%s=%s\n%c", appname, keyname, value, '\0');
        write_pos = this->file->get_before_size(this->file);
        goto rest;
    }

    if (!keyname) {
        if (value != NULL) return;

        write_pos = this->file->get_before_size(this->file) - strlen(read_ptr) - 1;
        while ((read_ptr = this->file->read(this->file)) != NULL) {
            if (strstr(read_ptr, "[")) {
                strcpy(write_ptr, read_ptr);
                goto rest;
            }
        }
        if (this->file->is_endof(this->file)) goto rest; 
    }

    while ((read_ptr = this->file->read(this->file)) != NULL) {
        if (strstr(read_ptr, "[")) {
            sprintf(write_ptr, "%s=%s\n%s%c", keyname, value, read_ptr, '\0');
            write_pos = this->file->get_before_size(this->file) - strlen(read_ptr);
            goto rest;
        }

        sprintf(write_ptr, "%s%c", read_ptr, '\0');
        ret_ptr = strtok_r(read_ptr, split, &save_ptr);
        if (!ret_ptr) continue;
        if (!strcmp(ret_ptr, keyname)) break;
    }
    if (!read_ptr) {
        sprintf(write_ptr, "%s=%s\n%c", keyname, value, '\0');
        write_pos = this->file->get_before_size(this->file);
        goto rest;
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

rest:
    this->file->readrn(this->file, write_ptr + strlen(write_ptr), this->file->get_rest_size(this->file));
    strcat(write_ptr, "\0");
    
    this->file->truncate(this->file, write_pos);
    this->file->seek(this->file, write_pos, SEEK_SET);
    this->file->write(this->file, write_ptr);
}

METHOD(ini_t, set_ini_split, void, private_ini_t *this, const char *split)
{
    if (split != NULL) strcpy(this->split, split);
}

METHOD(ini_t, ini_destroy, void, private_ini_t *this)
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
ini_t *create_ini(const char *filename)
{
    private_ini_t *this;

    if (filename == NULL && access(filename, F_OK) != 0) return NULL;
    INIT(this,
        .public = {
            .get_value = _get_ini_value,
            .set_value = _set_ini_value,
            .set_split = _set_ini_split,
            .destroy = _ini_destroy,
        },
        .file = create_fileio(filename, "r"),
        .split = ":=",
    );
    if (!this->file) {
        free(this);
        return NULL;
    }

    return &this->public;
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
