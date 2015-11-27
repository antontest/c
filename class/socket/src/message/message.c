#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <proc.h>
#include <utils/utils.h>
#include <data/linked_list.h>
#include <local.h>
#include <event.h>
#include <thread/thread.h>
#include <thread/bsem.h>
#include <thread/mutex.h>

#define MAX_MSG_LEN (1024)
#define COMM_KEY    (0x1111)
#define DFT_LOCAL_PATH "/home/anton/var/run/msg/"
typedef struct local_share_t local_share_t;
struct local_share_t {
    int mod_id;
    struct sockaddr_un addr;
};

typedef struct private_msg_mod_t private_msg_mod_t;
struct private_msg_mod_t {
    /**
     * @brief public interface
     */
    msg_mod_t public;

    /**
     * @brief public interface
     */
    msg_t *msg;

    /**
     * @brief local socket 
     */
    local_socket_t *lsck;

    /**
     * @brief message list
     */
    linked_list_t *msg_list;

    /**
     * @brief share socket fd and address packet
     */
    ipc_t *shm;

    /**
     * @brief message module config 
     */
    mod_cfg_t *msg_cfg;

    /**
     * @brief handle message recv
     */
    event_t *evt;

    /**
     * @brief message lock
     */
    mutex_t *msg_lock;

    /**
     * @brief if has message, then tell do it
     */
    bsem_t *has_msg;

    /**
     * @brief deal with msg
     */
    thread_t *work;

    /**
     * @brief stop flag
     */
    int stop;
};

private_msg_mod_t *global_this = NULL;

/**
 * @brief message handler when module recving
 */
static void on_msg_recv(int fd, private_msg_mod_t *this)
{
    int ret = 0;
    char *msg = (char *)malloc(MAX_MSG_LEN);
    if (!msg) return;

    ret = this->lsck->recv(this->lsck, msg, MAX_MSG_LEN, 0);
    if (ret < sizeof(msg_t)) return;

    this->msg_lock->lock(this->msg_lock);
    this->msg_list->insert_last(this->msg_list, msg);
    this->msg_lock->unlock(this->msg_lock);
    this->has_msg->post(this->has_msg);
}

/**
 * @brief handle message
 */
static void msg_handler(private_msg_mod_t *this)
{
    msg_t *msg = NULL;
    int ret = 0;

    while (!this->stop) {
        this->has_msg->wait(this->has_msg);
        if (this->stop) break;

        this->msg_lock->lock(this->msg_lock);
        ret = this->msg_list->remove_first(this->msg_list, (void **)&msg);
        this->msg_lock->unlock(this->msg_lock);
        if (ret == NOT_FOUND) continue;
        if (!msg) continue;
        if (this->msg_cfg && this->msg_cfg->handler) this->msg_cfg->handler(msg);
        free(msg);
        msg = NULL;
    }    
}

METHOD(msg_mod_t, act_, int, private_msg_mod_t *this, mod_cfg_t *cfg)
{
    local_share_t lshm;
    struct sockaddr *addr = NULL;
    char local_path[128]  = {0};

    if (!cfg || !cfg->handler || cfg->id == MOD_ID_UNKNOW) return -1;

    /**
     * create shared memory
     */
    memcpy(this->msg_cfg, cfg, sizeof(mod_cfg_t));
    if (this->shm->mkshm(this->shm, cfg->id + COMM_KEY, sizeof(local_share_t)) < 0)
        goto free;

    /**
     * create local socket
     */
    if (this->lsck->socket(this->lsck, SOCK_DGRAM) <=0 ) goto free;
    if (access(DFT_LOCAL_PATH, R_OK)) mkdir(DFT_LOCAL_PATH, 0777);
    sprintf(local_path, "%s%d", DFT_LOCAL_PATH, cfg->id + COMM_KEY);
    if (!(addr = this->lsck->init_addr(this->lsck, local_path))) goto free;
    if (this->lsck->bind(this->lsck) < 0) goto free;
    if (this->evt->add(this->evt, this->lsck->get_fd(this->lsck), EVENT_ON_RECV, (void *)on_msg_recv, this) < 0) goto free;

    /**
     * write shared memory
     */
    lshm.mod_id = cfg->id;
    memcpy(&lshm.addr, addr, sizeof(lshm.addr));
    if (this->shm->write(this->shm, &lshm, sizeof(lshm)) < 0) goto free;

    /**
     * start msg handler thread
     */
    this->work = thread_create((void *)msg_handler, this);
    if (!this->work) goto free;

    return 0;

free:
    perror("message module act failed");
    return -1;
}

METHOD(msg_mod_t, destroy_, void, private_msg_mod_t *this)
{
    int msg_cnt = 0;
    void *msg = NULL;

    if (this->evt) this->evt->destroy(this->evt);
    if (this->lsck) this->lsck->destroy(this->lsck);
    if (this->work) {
        this->stop = 1;
        if (this->has_msg) this->has_msg->post(this->has_msg);
        usleep(10);

        this->work->destroy(this->work);
    }
    if (this->msg_lock) {
        this->msg_lock->unlock(this->msg_lock);
        this->msg_lock->destroy(this->msg_lock);
    }
    if (this->has_msg) {
        this->has_msg->post(this->has_msg);
        this->has_msg->destroy(this->has_msg);
    }
    if (this->shm) this->shm->destroy(this->shm);
    if (this->msg_list) {
        this->msg_list->reset_current(this->msg_list);
        msg_cnt = this->msg_list->get_count(this->msg_list);
        while (msg_cnt-- > 0) {
            this->msg_list->get_next(this->msg_list, &msg);
            free(msg);
        }
    }
    if (this->msg_cfg) free(this->msg_cfg);
    if (this->msg) free(this->msg);
}

msg_mod_t *create_msg_mod()
{
    private_msg_mod_t *this;
    
    threads_init();
    INIT(this, 
        .public = {
            .act = _act_,
            .destroy  = _destroy_,
        },
        .msg      = NULL,
        .lsck     = create_local_socket(),
        .msg_list = linked_list_create(),
        .shm      = create_ipc(),
        .evt      = create_event(EVENT_MODE_EPOLL, 1000),
        .msg_cfg  = malloc(sizeof(mod_cfg_t)),
        .msg_lock = mutex_create(),
        .has_msg  = bsem_create(0),
    );
    global_this = this;

    return &this->public;
}


typedef struct private_msg_handler_t private_msg_handler_t;
struct private_msg_handler_t {
    /**
     * @brief public interface
     */
    msg_handler_t public;

    /**
     * @brief ipc memory share
     */
    local_share_t *share;

    /**
     * @brief ipc
     */
    ipc_t *shm;

    /**
     * @brief local socket
     */
    local_socket_t *sck;

    /**
     * @brief is connected
     */
    int connected;
};

METHOD(msg_handler_t, send_, int, private_msg_handler_t *this, msg_t *msg)
{
    int ret = 0;

    if (!this->share) {
        ret = this->shm->mkshm(this->shm, msg->dst_mod + COMM_KEY, sizeof(local_share_t));
        if (ret < 0) return -1;
        
        this->share = (local_share_t *)malloc(sizeof(local_share_t));
        if (!this->share) return -1;
        ret = this->shm->read(this->shm, this->share, sizeof(local_share_t));
        if (ret < 0) return -1;
        this->connected = 0;
    }
    
    if (this->share->mod_id != msg->dst_mod) {
        ret = this->shm->read(this->shm, this->share, sizeof(local_share_t));
        if (ret < 0) return -1;
        this->connected = 0;
    }
    
    if (!this->connected) {
        if (this->sck->connect_addr(this->sck, (struct sockaddr *)&this->share->addr) < 0) {
            perror("local socket connec()");
            return -1;
        }
        this->connected = 1;   
    }
    ret = this->sck->send(this->sck, msg, sizeof(msg_t) + msg->msg_len, 0);

    return ret;
}

METHOD(msg_handler_t, destroy_msg_hdl_, void, private_msg_handler_t *this)
{
    if (this->sck) this->sck->destroy(this->sck);
    if (this->shm) this->shm->destroy(this->shm);
    if (this->share) free(this->share);
    free(this);
}

msg_handler_t *create_msg_handler()
{
    private_msg_handler_t *this;

    INIT(this, 
        .public = {
            .send    = _send_,
            .destroy = _destroy_msg_hdl_,
        },
        .share = NULL,
        .shm = create_ipc(),
        .sck = create_local_socket(),
        .connected = 0,
    );

    this->sck->socket(this->sck, SOCK_DGRAM);
    return &this->public;
}

msg_t *create_new_msg(mod_id_t src_mod, mod_id_t dst_mod, msg_id_t msg_id, void *data, int size)
{
    msg_t *msg = NULL;

    if ((data && size <=0) || dst_mod == MOD_ID_UNKNOW) return NULL;
    msg = (msg_t *)malloc(sizeof(msg_t) + size);
    if (!msg) return NULL;

    msg->src_mod = src_mod;
    msg->dst_mod = dst_mod;
    msg->msg_id  = msg_id;
    msg->msg_len = size;
    memcpy(msg->data, data, size);

    return msg;
}
