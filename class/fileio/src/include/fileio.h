#ifndef __CFG_H__
#define __CFG_H__

typedef struct filelock_t filelock_t;
struct filelock_t {
    /**
     * @brief used to describe the locking register action taking place 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*lock_register) (filelock_t *this, int cmd, int type, off_t offset, int whence, off_t len);

    /**
     * @brief used to describe the read locking action taking place 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*read_lock) (filelock_t *this, off_t offset,int whence,off_t len);

    /**
     * @brief used to describe the read locking action taking place 
     */
    int (*read_lock_all) (filelock_t *this);

    /**
     * @brief used to describe the read locking action taking place 
     */
    int (*read_lock_rest) (filelock_t *this);

    /**
     * @brief used to describe the locking read action taking place 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*read_lock_try) (filelock_t *this, off_t offset,int whence,off_t len);

    /**
     * @brief used to describe the locking write action taking place 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*write_lock) (filelock_t *this, off_t offset,int whence,off_t len);

    /**
     * @brief used to describe the write locking action taking place 
     */
    int (*write_lock_all) (filelock_t *this);

    /**
     * @brief used to describe the locking write action taking place 
     */
    int (*write_lock_rest) (filelock_t *this);

    /**
     * @brief used to describe the locking write action taking place 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*write_lock_try) (filelock_t *this, off_t offset,int whence,off_t len);

    /**
     * @brief used to unlock the locking write action taking place 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*unlock) (filelock_t *this, off_t offset, int whence,off_t len);

    /**
     * @brief set file handle
     *
     * @param fd  file handler
     */
    void (*set_file_handle) (filelock_t *this, FILE *fp);

    /**
     * @brief free file lock instance 
     */
    void (*destroy) (filelock_t *this);

    /**
     * @brief checked whether has readable permission 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*is_read_lockable) (filelock_t *this, off_t offset, int whence, off_t len);

    /**
     * @brief checked whether has writeable permission 
     *
     * @param offset  offset from l_whence
     * @param whence  SEEK_SET, SEEK_CUR, SEEK_END 
     * @param len     length, 0 = to EOF  
     */
    int (*is_write_lockable) (filelock_t *this, off_t offset, int whence, off_t len);
};

/**
 * @brief create_filelock instance
 */
filelock_t *create_filelock(FILE *fp);


typedef struct fileio_t fileio_t;
struct fileio_t {
    /**
     * @brief  Open a file 
     *
     * @param path   path of file 
     * @param mode   r, w, r+, w+ and so on
     */
    FILE *(*open) (fileio_t *this, const char *filename, const char *mode);
    
    /**
     * @brief opens the file whose name is the string pointed to by path and associates the stream  pointed  to  by  stream with it.
     *
     * @param mode  r, w, r+, w+ and so on
     */
    FILE *(*ropen) (fileio_t *this, const char *mode);
    /**
     * @brief   Read a line string from file 
     * @return  pointer to read buffer, if succ; NULL, if failed;
     */
    char *(*read) (fileio_t *this);

    /**
     * @brief Read the specified number context from file  
     *
     * @param   size   the specified number
     * @return  pointer to read buffer, if succ; NULL, if failed;
     */
    char *(*readn) (fileio_t *this, int size);

    /**
     * @brief read one line, save to buffer
     *
     * @param buffer read buffer
     * @param size   read buffer size
     * @return       pointer to read buffer 
     */
    char *(*readrl) (fileio_t *this, char *buffer, int size);

    /**
     * @brief read specified number string, save to buffer
     *
     * @param buffer read buffer
     * @param size   read buffer size
     * @return       number of reading 
     */
    int (*readrn) (fileio_t *this, char *buffer, int size);

    /**
     * @brief Write string to file
     *
     * @param buf   string writing
     * @return      writed number of string, if succ; -1, if failed;
     */
    int (*write) (fileio_t *this, const char *buf);

    /**
     * @brief Write string with format
     *
     * @param fmt   Write string format
     * @param ...   parameter
     * @return      writed number of string, if succ;
     */
    int (*vwrite) (fileio_t *this, const char *fmt, ...);

    /**
     * @brief sets the file position indicator for the stream pointed to by stream
     *
     * @param offset  offset bytes
     * @param whence  whence is set to SEEK_SET, SEEK_CUR, or SEEK_END
     */
    int (*seek) (fileio_t *this, int offset, int whence);

    /**
     * @briefe a file to a specified length
     *
     * @param length  file length
     */
    void (*truncate) (fileio_t *this, unsigned int length);

    /**
     * @brief close file 
     */
    void (*close) (fileio_t *this);

    /**
     * @brief free fileio instance 
     */
    void (*destroy) (fileio_t *this);

    /**
     * @brief return read buffer pointer  
     */
    char *(*get_read_buffer) (fileio_t *this);

    /**
     * @brief return write buffer pointer  
     */
    char *(*get_write_buffer) (fileio_t *this);

    /**
     * @brief file handler
     */
    FILE *(*get_file_handle) (fileio_t *this);

    /**
     * @brief Examines the argument stream 
     * @return     its integer descriptor
     */
    int (*get_file_no) (fileio_t *this);

    /**
     * @brief   get current file name  
     * @return  file name
     */
    char *(*get_filename) (fileio_t *this);

    /**
     * @brief  get file size of file pointer 
     * @return file size
     */
    int (*get_file_size) (fileio_t *this);

    /**
     * @brief  get size of current file pointer before 
     * @return file size
     */
    int (*get_before_size) (fileio_t *this);

    /**
     * @brief  get file rest size of file pointer 
     * @return rest size
     */
    int (*get_rest_size) (fileio_t *this);

    /**
     * @brief read buffer size
     */
    int (*get_read_buf_size) (fileio_t *this);

    /**
     * @brief read buffer size
     */
    int (*get_write_buf_size) (fileio_t *this);

    /**
     * @brief  tests the end-of-file indicator for the stream pointed to by stream
     * @return non zero, if end of file; 0, if not; -1, if failed
     */
    int (*is_endof) (fileio_t *this);

    /**
     * @brief set file read buffer size
     *        default value: 1024
     *
     * @param size   buffer size
     */
    char *(*set_read_buf_size) (fileio_t *this, unsigned int size);
    
    /**
     * @brief set file read buffer size
     *        default value: 1024
     *
     * @param size   buffer size
     */
    char *(*set_write_buf_size) (fileio_t *this, unsigned int size);
};

/**
 * @brief create fileio instance
 *
 * @param filename     file name
 */
fileio_t *create_fileio(const char *filename, const char *mode);


typedef struct cfg_t cfg_t;
struct cfg_t {
    /**
     * @brief Get value by name from configure file  
     *
     * @param key_name  config key name
     * @param split     file config with split
     *
     * @return          key value
     */
    char *(*get_value) (cfg_t *this, const char *keyname);

    /**
     * @brief Get value by name from configure file  
     *
     * @param key_name  config key name
     * @param split     file config with split
     * @param value     value
     */
    void (*set_value) (cfg_t *this, const char *keyname, const char *value);

    /**
     * @brief  config split
     */
    void (*set_split) (cfg_t *this, const char *split);

    /**
     * @brief free instance and memory
     */
    void (*destroy) (cfg_t *this);
};

/**
 * @brief create config file dealing instance 
 *
 * @param filename   config file name
 */
cfg_t *create_cfg(const char *filename);


typedef struct ini_t ini_t;
struct ini_t {
    /**
     * @brief Get value by name from configure file  
     *
     * @param key_name  config key name
     * @param split     file config with split
     *
     * @return          key value
     */
    char *(*get_value) (ini_t *this, const char *appname, const char *keyname);

    /**
     * @brief Get value by name from configure file  
     *
     * @param key_name  config key name
     * @param split     file config with split
     * @param value     value
     */
    void (*set_value) (ini_t *this, const char *appname, const char *keyname, const char *value);

    /**
     * @brief  ini config split
     */
    void (*set_split) (ini_t *this, const char *split);

    /**
     * @brief free instance and memory
     */
    void (*destroy) (ini_t *this);
};

/**
 * @brief create config file dealing instance 
 *
 * @param filename   config file name
 */
ini_t *create_ini(const char *filename);


/**
 * @brief recover filename by file handle
 *
 * @param fp   file handle
 * @return     file name
 */
char *recover_filename(FILE *fp);

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

/**
 * @brief remove directory
 *
 * @param pathname [in] path of directory
 *
 * @return 0, if succ; -1, if failed.
 */
int remove_dir(const char *pathname);

/**
 * @brief remove file or dir
 *
 * @param pathname [in] path of file or directoy
 *
 * @return 0, if succ; -1, if failed.
 */
int remove_file(const char *pathname);

/**
 * @brief judge file permission
 *
 * @param path       [in] path
 * @param permission [in] string of permission, like "frwx" 
 *
 * @return 1, if have the permission, 0, if have no; -1, if failed
 */
int detect_permission(const char *path, const char *permission);

/**
 * @brief basename -- get basename of path, like /home/anton/test -- test
 *
 * @param path [in]  path
 * @param name [out] basename of path
 * @param size [in]  buffer size
 *
 * @return basename, if succ; -1, if failed
 */
char *basename(char *path, char *name, int size);

/**
 * @brief dirname -- get dirname of path, like /home/anton/test -- test
 *
 * @param path [in]  path
 * @param name [out] dirname of path
 * @param size [in]  buffer size
 *
 * @return dirname, if succ; -1, if failed
 */
char *dirname(char *path, char *name, int size);

#endif
