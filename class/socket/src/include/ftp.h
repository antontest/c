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
    int (*login) (ftp_t *this, char *server, unsigned int port, const char *user, const char *passwd);

    /**
     * @brief  ftp server file listing 
     *
     * @param buf   return results buffer
     * @param size  buffer size
     *
     * @return      pointer to buffer
     */
    char *(*list) (ftp_t *this, const char *path, char *buf, int size);

    /**
     * @brief  ftp server file listing 
     *
     * @param buf   return results buffer
     * @param size  buffer size
     *
     * @return      pointer to buffer
     */
    char *(*pwd) (ftp_t *this, char *buf, int size);

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
