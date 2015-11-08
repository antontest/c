#ifndef __CFG_H__
#define __CFG_H__

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
#endif
