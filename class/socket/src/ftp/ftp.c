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
     * @brief ftp message buffer 
     */
    char *msg_buffer;
};

METHOD(ftp_t, login, int, private_ftp_t *this, char *server, unsigned int port, const char *user, const char *passwd)
{
    if (!user || !passwd || !server) return -1;
    
    this->sck = create_socket();
    if (this->sck->connect(this->sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, server, port) <= 0) return -1;

    this->msg_buffer= (char *)malloc(FTP_MSG_BUFF_SIZE);
    if (!this->msg_buffer) return -1;
    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    if (this->code != 220) return -1;
    
    sprintf(this->msg_buffer, "USER %s\r\n", user); 
    this->sck->send(this->sck, this->msg_buffer, strlen(this->msg_buffer));

    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    if (this->code != 331) return -1;

    sprintf(this->msg_buffer, "PASS %s\r\n", passwd); 
    this->sck->send(this->sck, this->msg_buffer, strlen(this->msg_buffer));

    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    if (this->code != 230) return -1;
    this->logged = 1;
    this->server = strdup(server);

    return 0;
}

METHOD(ftp_t, get_data_port, int, private_ftp_t *this)
{
    int p1 = 0, p2 = 0, p3_secute = 0;
    char *p = NULL;

    if (this->logged != 1) return -1;

    sprintf(this->msg_buffer, "PASV\r\n");
    this->sck->send(this->sck, this->msg_buffer, strlen(this->msg_buffer));
    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    if (this->code != 227) return -1;

    strtok(this->msg_buffer, "(");
    p = strtok(NULL, "(");
    if (!p) return -1;
    sscanf(p, "%*d,%*d,%*d,%d,%d,%d)", &p3_secute, &p1, &p2);
    this->data_port = p1 * 256 + p2;
    printf("p1: %d, p2: %d, data_port: %d\n", p1, p2, this->data_port);
    
    return 0;
}

METHOD(ftp_t, list, char *, private_ftp_t *this, const char *path, char *buf, int size)
{
    _get_data_port(this);
    printf("data_port: %d\n", this->data_port);

    //this->data_sck = create_socket();
    //this->data_sck->connect(this->data_sck, AF_INET, SOCK_STREAM, IPPROTO_TCP, this->server, this->data_port);

    sprintf(this->msg_buffer, "LIST %s\r\n",path);
    this->sck->send(this->sck, this->msg_buffer, strlen(this->msg_buffer));
    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    printf("list: %s", this->msg_buffer);
    sscanf(this->msg_buffer, "%d", &this->code);
    printf("code: %d\n", this->code);
    
    sprintf(this->msg_buffer, "LIST %s\r\n", path);
    this->sck->send(this->sck, this->msg_buffer, strlen(this->msg_buffer));
    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    printf("list: %s", this->msg_buffer);
    //if (this->code != 150) return NULL;


    return buf;
}

METHOD(ftp_t, pwd, char *, private_ftp_t *this, char *buf, int size)
{
    char *path = NULL;

    sprintf(this->msg_buffer, "PWD\r\n");
    this->sck->send(this->sck, this->msg_buffer, strlen(this->msg_buffer));
    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    if (this->code != 257) return NULL;

    path = strstr(this->msg_buffer, "\"");
    sscanf(path, "\"%s\" %[^\n]s", path, this->msg_buffer);
    strtok(path, "\"");
    printf("path: %s\n", path);
    
    return buf;
}

METHOD(ftp_t, close_, void, private_ftp_t *this)
{
    if (this->logged != 1) return;
    sprintf(this->msg_buffer, "QUIT\r\n");
    this->sck->send(this->sck, this->msg_buffer, strlen(this->msg_buffer));
    this->sck->receive(this->sck, this->msg_buffer, FTP_MSG_BUFF_SIZE, 0);
    sscanf(this->msg_buffer, "%d", &this->code);
    if (this->code != 221) this->logged = -1;
    
    this->logged = 0;
    this->sck->destroy(this->sck);
}

METHOD(ftp_t, destroy_, void, private_ftp_t *this)
{
    if (this->sck != NULL) close_(this);
    if (this->msg_buffer != NULL) free(this->msg_buffer);

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

    return &this->public;
}
