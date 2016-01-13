#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <ftp.h>
#include <utils.h>
#include <fileio.h>
#include <socket.h>
#include <stdarg.h>

#define FTP_PORT "21"
#define FTP_SERV "localhost"
#define FTP_MSG_BUFF_SIZE (1024)
#define FTP_CODE_BUFF_SIZE (128)
typedef struct private_ftp_t private_ftp_t;
struct private_ftp_t {
    /**
     * @brief public interface
     */
    ftp_t public;

    /**
     * @brief ftp server ip address
     */
    char *server;

    /**
     * @brief ftp server port listening on
     */
    unsigned int port;

    /**
     * @brief user name
     */
    char *user;

    /**
     * @brief password
     */
    char *passwd;
    
    /**
     * @brief login flag
     */
    int logged;

    /**
     * @brief ftp return code
     * Like 220 Welcome info
     *      230 login successfully
     *      530 login incorrect
     *      331 User name okay, need password
     *      221 Goodbye!
     *      227 Entering Passive Mode (172,21,34,18,128,233)
     */
    int code;

    /**
     * @brief ftp data transport
     */
    int data_port;

    /**
     * @brief deal with file
     */
    FILE *fp;

    /**
     * @brief ftp socket
     */
    socket_t *sck;

    /**
     * @brief socket used to transport data
     */
    socket_t *data_sck;

    /**
     * @brief code buffer
     */
    char *code_buffer;

    /**
     * @brief ftp message buffer 
     */
    char *msg_buffer;

    /**
     * @brief file dealing
     */
    fileio_t *file;
};

static int ftp_cmd_send(private_ftp_t *this, const char *code, const char *fmt, ...)
{
    va_list arg;
    char extern_opt[48] = {0};

    va_start(arg, fmt);
    vsnprintf(extern_opt, sizeof(extern_opt), fmt, arg);
    va_end(arg);

    memset(this->code_buffer, 0, FTP_CODE_BUFF_SIZE);
    if (strlen(extern_opt))
        snprintf(this->code_buffer, FTP_CODE_BUFF_SIZE, "%s %s\r\n", code, extern_opt);
    else
        snprintf(this->code_buffer, FTP_CODE_BUFF_SIZE, "%s\r\n", code);

    return this->sck->send(this->sck, this->code_buffer, FTP_CODE_BUFF_SIZE);
}

static int ftp_msg_recv(private_ftp_t *this, socket_t *sock)
{
    if (!this->msg_buffer) return -1;
    sock->recv(sock, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    
    return this->code;
}

METHOD(ftp_t, login, int, private_ftp_t *this, char *server, unsigned int port, const char *user, const char *passwd)
{
    if (!user || !passwd || !server || !this->sck) return -1;
    if (this->logged) return 0;
    this->server = strdup(server);
    this->user   = strdup(user);
    this->passwd = strdup(passwd);
    this->port   = port;
    
    if (this->sck->connect(this->sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, this->server, this->port) <= 0) {
        perror("connect()");
        return -1;
    }
    ftp_msg_recv(this, this->sck);
    if (this->code != 220) return -1;
    
    ftp_cmd_send(this, "USER", this->user);
    ftp_msg_recv(this, this->sck);
    if (this->code != 331) return -1;

    ftp_cmd_send(this, "PASS", this->passwd);
    ftp_msg_recv(this, this->sck);
    if (this->code != 230) return -1;
    this->logged = 1;

    return 0;
}

METHOD(ftp_t, get_data_port, int, private_ftp_t *this)
{
    int p1 = 0, p2 = 0, p3_secute = 0;
    char *p = NULL;

    if (this->logged != 1) return -1;

    ftp_cmd_send(this, "PASV", NULL);
    ftp_msg_recv(this, this->sck);
    if (this->code != 227) return -1;

    strtok(this->msg_buffer, "(");
    p = strtok(NULL, "(");
    if (!p) return -1;
    sscanf(p, "%*d,%*d,%*d,%d,%d,%d)", &p3_secute, &p1, &p2);
    this->data_port = p1 * 256 + p2;
    
    return 0;
}

METHOD(ftp_t, list, void, private_ftp_t *this, const char *path)
{
    _get_data_port(this);

    this->data_sck->connect(this->data_sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, this->server, this->data_port);
    if (this->data_sck->get_state(this->data_sck) != SOCKET_CONNECTED) return;

    ftp_cmd_send(this, "LIST", path);
    while (this->data_sck->recv(this->data_sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0) > 0) {
        printf("%s", this->msg_buffer);
        memset(this->msg_buffer, 0, FTP_MSG_BUFF_SIZE);
    }
    this->data_sck->close(this->data_sck);
}

METHOD(ftp_t, pwd, char *, private_ftp_t *this)
{
    char *path = NULL;

    ftp_cmd_send(this, "PWD", NULL);
    ftp_msg_recv(this, this->sck);
    if (this->code != 257) return NULL;

    path = strstr(this->msg_buffer, "\"");
    sscanf(path, "\"%s\" %[^\n]s", path, this->msg_buffer);
    strtok(path, "\"");
    
    return path;
}

METHOD(ftp_t, size_, int, private_ftp_t *this, const char *path)
{
    int size = -1;

    ftp_cmd_send(this, "SIZE", path);
    ftp_msg_recv(this, this->sck);
    if (this->code != 213) return -1;

    sscanf(this->msg_buffer, "%*d %d", &size);
    return size;
}

METHOD(ftp_t, cd_, int, private_ftp_t *this, const char *path)
{
    if (!path) return -1;

    ftp_cmd_send(this, "CWD", path);
    ftp_msg_recv(this, this->sck);
    if (this->code != 250) return -1;

    return 0;
}

METHOD(ftp_t, mkdir_, int, private_ftp_t *this, const char *ftp_path, const char *dir_path)
{
    if (!ftp_path || !dir_path) return -1;

    if (_cd_(this, ftp_path) < 0) return -1;
    ftp_cmd_send(this, "MKD", dir_path);
    ftp_msg_recv(this, this->sck);
    if (this->code != 257) return -1;

    return 0;
}

METHOD(ftp_t, rmdir_, int, private_ftp_t *this, const char *ftp_path, const char *dir_path)
{
    if (!ftp_path || !dir_path) return -1;

    if (_cd_(this, ftp_path) < 0) return -1;
    ftp_cmd_send(this, "RMD", dir_path);
    ftp_msg_recv(this, this->sck);
    if (this->code == 550) {
        printf("Permission denied.\n");
        return -1;
    }
    printf("%s\n", this->msg_buffer);
    if (this->code != 257) return -1;

    return 0;
}

METHOD(ftp_t, download_, int, private_ftp_t *this, const char *path)
{
    int size          = 0;
    int recv_cnt      = 0;
    int download_size = 0;
    char *filename    = NULL;
    char *ftp_path    = NULL;
    int  ftp_path_len = 0;

    /**
     * 1. get file name by path 
     * 2. check file whether already exist
     */
    if ((filename = strrchr(path, '/')) != NULL) filename += 1;
    else filename = (char *)path;
    if (!access(filename, F_OK)) {
        printf("%s already exist!\n", filename);
        return -1;
    }

    /**
     * get dirname from path
     */
    ftp_path_len = filename - path;
    if (ftp_path_len > 0) {
        ftp_path = malloc(filename - path);
        strncpy(ftp_path, path, filename - path);
        if (_cd_(this, ftp_path) < 0) {
            printf("ftp server does not exist directroy \"%s\"\n", ftp_path);
            free(ftp_path);
            return -1;
        }
        free(ftp_path);
    }

    /**
     * get size of file
     */
    size = _size_(this, filename);
    if (size < 0) {
        printf("ftp server has no file \"%s\"\n", filename);
        return -1;
    }

    ftp_cmd_send(this, "TYPE I", NULL);
    ftp_msg_recv(this, this->sck);
    if (this->code != 200) return -1;

    /**
     * get ftp data port and connect to data_port
     */
    _get_data_port(this);
    ftp_cmd_send(this, "RETR", filename);
    this->data_sck->connect(this->data_sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, this->server, this->data_port);
    if (this->data_sck->get_state(this->data_sck) != SOCKET_CONNECTED) return -1;

    /**
     * download file
     */
    this->file->open(this->file, filename, "wb");
    while ((recv_cnt = this->data_sck->recv(this->data_sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0)) > 0) {
        download_size += recv_cnt;
        this->file->writen(this->file, this->msg_buffer, recv_cnt);
        printf("\rdownload: %.2f%%", ((float)download_size / (float)size) * 100);
        fflush(stdout);
        memset(this->msg_buffer, 0, FTP_MSG_BUFF_SIZE);
    }
    printf("\n");
    printf("file size: %d, download size: %d\n", size, download_size);
    this->data_sck->close(this->data_sck);
    this->file->fflush(this->file);
    this->file->close(this->file);
    
    return 0;
}

METHOD(ftp_t, upload_, int, private_ftp_t *this, const char *ftp_path, const char *file_path)
{
    int size        = 0;
    int read_cnt    = 0;
    int upload_size = 0;
    int left_cnt    = 0;
    char *filename  = NULL;
    struct stat st  = {0};

    if (!file_path) {
        printf("file path can't be null\n");
        return -1;
    }
    /**
     * 1. get file name by path 
     * 2. check file whether already exist
     */
    if ((filename = strrchr(file_path, '/')) != NULL) filename += 1;
    else filename = (char *)file_path;

    if (lstat(file_path, &st) < 0) {
        printf("%s dost not exist!\n", file_path);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        printf("could not up this file\n");
        return -1;
    }
    if (access(file_path, R_OK)) {
        printf("has no read pemission\n");
        return -1;
    }
    
    /**
     * change ftp directroy
     */
    if (_cd_(this, ftp_path) < 0) {
        printf("ftp server does not exist directroy \"%s\"\n", ftp_path);
        return -1;
    }

    ftp_cmd_send(this, "TYPE I", NULL);
    ftp_msg_recv(this, this->sck);
    if (this->code != 200) return -1;

    /**
     * get ftp data port and connect to data_port
     */
    _get_data_port(this);
    ftp_cmd_send(this, "STOR", filename);
    this->data_sck->connect(this->data_sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, this->server, this->data_port);
    if (this->data_sck->get_state(this->data_sck) != SOCKET_CONNECTED) return -1;

    /**
     * upload file
     */
    if (!this->file->open(this->file, file_path, "rb+")) {
        printf("open %s failed\n", file_path);
        return -1;
    }
    left_cnt = size = this->file->get_file_size(this->file);
    while (1) {
        read_cnt = left_cnt < FTP_MSG_BUFF_SIZE ? left_cnt : FTP_MSG_BUFF_SIZE;
        read_cnt = this->file->readrn(this->file, this->msg_buffer, read_cnt);
        upload_size += read_cnt;
        left_cnt -= read_cnt;
        while (this->data_sck->send(this->data_sck, this->msg_buffer, read_cnt) != read_cnt);
        printf("\rupload: %.2f%%", ((float)upload_size / (float)size) * 100);
        fflush(stdout);
        memset(this->msg_buffer, 0, FTP_MSG_BUFF_SIZE);
        if (upload_size >= size) break;
    }
    printf("\n");
    printf("file size: %d, upload_size: %d\n", size, upload_size);
    this->data_sck->close(this->data_sck);
    this->file->close(this->file);
    
    return 0;
}

METHOD(ftp_t, close_, void, private_ftp_t *this)
{
    if (this->logged != 1) return;

    ftp_cmd_send(this, "QUIT", NULL);
    ftp_msg_recv(this, this->sck);
    if (this->code != 221) this->logged = -1;
    
    this->logged = 0;
    this->sck->destroy(this->sck);
}

METHOD(ftp_t, destroy_, void, private_ftp_t *this)
{
    if (this->sck         != NULL) _close_(this);
    if (this->msg_buffer  != NULL) free(this->msg_buffer);
    if (this->code_buffer != NULL) free(this->code_buffer);
    if (this->user        != NULL) free(this->user);
    if (this->passwd      != NULL) free(this->passwd);
    if (this->server      != NULL) free(this->server);
    if (this->file        != NULL) this->file->destroy(this->file);

    free(this);
}

ftp_t *create_ftp()
{
    private_ftp_t *this;

    INIT(this,
        .public = {
            .login         = _login,
            .list          = _list,
            .pwd           = _pwd,
            .size          = _size_,
            .download      = _download_,
            .upload        = _upload_,
            .cd            = _cd_,
            .mkdir         = _mkdir_,
            .rmdir         = _rmdir_,
            .get_data_port = _get_data_port,
            .destroy       = _destroy_,
        },
        .fp         = NULL,
        .sck        = create_socket(),
        .data_sck   = create_socket(),
        .msg_buffer = (char *)malloc(FTP_MSG_BUFF_SIZE),
        .logged     = 0,
        .data_port  = -1,
        .file       = create_fileio(NULL, NULL),
    );
    this->code_buffer = (char *)malloc(FTP_CODE_BUFF_SIZE);
    if (!this->code_buffer) {
        _destroy_(this);
        return NULL;
    }

    return &this->public;
}
