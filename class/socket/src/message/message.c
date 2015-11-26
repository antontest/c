#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
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

typedef struct local_share_t local_share_t;
struct local_share_t {
    int fd;
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

static void on_msg_recv(int fd, private_msg_mod_t *this)
{
    int ret = 0;
    char *msg = (char *)malloc(1024);
    if (!msg) return;

    ret = this->lsck->recv(this->lsck, msg, 1024, 0);
    if (ret < sizeof(msg_t)) return;

    this->msg_lock->lock(this->msg_lock);
    this->msg_list->insert_last(this->msg_list, msg);
    this->msg_lock->unlock(this->msg_lock);
    this->has_msg->post(this->has_msg);
}

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

METHOD(msg_mod_t, register_mod, int, private_msg_mod_t *this, mod_cfg_t *cfg)
{
    local_share_t lshm;
    struct sockaddr *addr = NULL;

    if (!cfg || !cfg->handler || cfg->id == MOD_ID_UNKNOW) return -1;
    memcpy(this->msg_cfg, cfg, sizeof(mod_cfg_t));
    if (this->shm->mkshm(this->shm, 0x1234, sizeof(local_share_t)) < 0)
        goto free;

    if (this->lsck->socket(this->lsck, SOCK_DGRAM) <=0 ) goto free;
    if (!(addr = this->lsck->init_addr(this->lsck, "./s1"))) goto free;
    if (this->lsck->bind(this->lsck) < 0) goto free;
    if (this->evt->add(this->evt, this->lsck->get_fd(this->lsck), EVENT_ON_RECV, (void *)on_msg_recv, this) < 0) goto free;

    lshm.fd = this->lsck->get_fd(this->lsck);
    memcpy(&lshm.addr, addr, sizeof(lshm.addr));
    if (this->shm->write(this->shm, &lshm, sizeof(lshm)) < 0) goto free;

    this->work = thread_create((void *)msg_handler, this);
    if (!this->work) goto free;

    return 0;

free:
    return -1;
}

msg_mod_t *create_msg_mod()
{
    private_msg_mod_t *this;
    
    threads_init();
    INIT(this, 
        .public = {
            .register_mod = _register_mod,
            .destroy = _destroy_,
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

int send_msg(msg_t *msg)
{
    local_share_t lshm;
    local_socket_t *lsck = create_local_socket();
    ipc_t *shm = create_ipc();
    int ret = 0;

    ret = shm->mkshm(shm, 0x1234, sizeof(local_share_t));
    if (ret < 0) return -1;
    ret = shm->read(shm, &lshm, sizeof(local_share_t));
    shm->destroy(shm);
    if (ret < 0) return -1;

    lsck->socket(lsck, SOCK_DGRAM);
    lsck->init_addr(lsck, lshm.addr.sun_path);
    if (lsck->connect(lsck) < 0) printf("connect failed\n");
    perror("connect\n");
    ret = lsck->send(lsck, msg, sizeof(msg_t), 0);

    lsck->destroy(lsck);
    return 0;
}
