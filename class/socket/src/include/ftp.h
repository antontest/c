#ifndef __FTP_H__
#define __FTP_H__

typedef struct ftp_t ftp_t;
struct ftp_t  {
    /**
     * @brief login ftp server
     *
     * @param server  ftp server ip address
     * @param port    ftp server port listening on
     * @param user    user name
     * @param passwd  password
     *
     * @return        0, if login successfully; -1, if failed
     * 
     */
    int (*login) (ftp_t *this, char *server, unsigned int port, const char *user, const char *passwd, int timeout_ms);

    /**
     * @brief  ftp server file listing 
     *
     * @param buf   return results buffer
     * @param size  buffer size
     *
     * @return      pointer to buffer
     */
    void (*list) (ftp_t *this, const char *path);

    /**
     * @brief  ftp server file listing 
     *
     * @param buf   return results buffer
     * @param size  buffer size
     *
     * @return      pointer to buffer
     */
    char *(*pwd) (ftp_t *this);

    /**
     * @brief get file size on ftp server
     * @param path  file path
     * @return      file size
     */
    int (*size) (ftp_t *this, const char *path);

    /**
     * @brief change directroy
     * @param path  file path
     * @return 0, if succ; -1, if failed
     */
    int (*cd) (ftp_t *this, const char *path);

    /**
     * @brief make directroy on ftp server
     * @param ftp_path path on ftp server
     * @param path new directroy
     * @return 0, if succ; -1, if failed
     */
    int (*mkdir) (ftp_t *this, const char *ftp_path, const char *dir_path);

    /**
     * @brief make directroy on ftp server
     * @param ftp_path path on ftp server
     * @param path new directroy
     * @return 0, if succ; -1, if failed
     */
    int (*rmdir) (ftp_t *this, const char *ftp_path, const char *dir_path);

    /**
     * @brief download one file from ftp server
     * @param path  file path
     * @return      file size
     */
    int (*download) (ftp_t *this, const char *path);

    /**
     * @brief upload one file from ftp server
     * @param path  file path
     * @return      file size
     */
    int (*upload) (ftp_t *this, const char *ftp_path, const char *file_path);

    /**
     * @brief close ftp instance
     */
    void (*close) (ftp_t *this);

    /**
     * @brief free ftp instance 
     */
    void (*destroy) (ftp_t *this);

    /**
     * @brief get ftp data transport port
     */
    int (*get_data_port) (ftp_t *this);

};

/**
 * @brief create ftp client instance 
 */
ftp_t *create_ftp();

#endif /* __FTP_H__ */
