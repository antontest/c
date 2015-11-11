#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftp.h>
#include <utils/utils.h>
#include <fileio.h>
#include <socket.h>

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
    if (!this->msg_buffer) this->msg_buffer= (char *)malloc(FTP_MSG_BUFF_SIZE);
    if (!this->msg_buffer) return -1;
    sock->recv(sock, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    
    return this->code;
}

METHOD(ftp_t, login, int, private_ftp_t *this, char *server, unsigned int port, const char *user, const char *passwd)
{
    if (!user || !passwd || !server) return -1;
    if (this->sck != NULL) return 0;
    this->server = strdup(server);
    this->user   = strdup(user);
    this->passwd = strdup(passwd);
    this->port   = port;
    
    this->sck = create_socket();
    if (this->sck->connect(this->sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, this->server, this->port) <= 0) return -1;
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
    char *list_buffer = NULL;
    int list_bytes = 0;

    _get_data_port(this);

    this->data_sck = create_socket();
    this->data_sck->connect(this->data_sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, this->server, this->data_port);
    if (this->data_sck->get_state(this->data_sck) != SOCKET_CONNECTED) return;

    ftp_cmd_send(this, "LIST", path);
    list_bytes = this->data_sck->get_can_read_bytes(this->data_sck);
    list_buffer = (char *)malloc(list_bytes);
    if (!list_buffer) return;
    this->data_sck->recv(this->data_sck, list_buffer, list_bytes, 0);
    printf("%s", list_buffer);
    free(list_buffer);
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
            .get_data_port = _get_data_port,
            .destroy       = _destroy_,
        },
        .fp         = NULL,
        .sck        = NULL,
        .msg_buffer = NULL,
        .logged     = 0,
        .data_port  = -1,
    );
    this->code_buffer = (char *)malloc(FTP_CODE_BUFF_SIZE);
    if (!this->code_buffer) {
        free(this);
        return NULL;
    }

    return &this->public;
}
