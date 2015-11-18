#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <thread/thread.h>
#include <utils/utils.h>
#include <utils/linked_list.h>

typedef struct event_pkg_t event_pkg_t;
struct event_pkg_t {
    /**
     * @brief socket descriptor listening on
     */
    int fd;

    /**
     * @brief event type
     */
    event_type_t type;

    /**
     * @brief socket event handler callback function 
     */
    void (*event_handler) (int fd, void *arg);

    /**
     * @brief callback function parameter
     */
    void *arg;
};

typedef struct private_event_t private_event_t;
struct private_event_t {
    /**
     * @brief public interface
     */
    event_t public;

    /**
     * @brief socket event mode, like select or epool
     */
    event_mode_t mode;

    /**
     * @brief list of event listening on
     */
    linked_list_t *event_list;

    /**
     * @brief event listening on thread
     */
    thread_t *thread;

    /**
     * @brief event handle information
     */
    union {
        struct {
            /**
             * @brief max fd of listening on
             */
            int max_fd;

            /**
             * @brief fd sets listening on
             */
            fd_set fds;
        } s;
#define select_fds   fd.s.fds
#define select_maxfd fd.s.max_fd

        struct {
            /**
             * @brief handle of epoll
             */
            int epfd;
        } e;
#define epoll_fd fd.e.epfd
    } fd;

    /**
     * @brief stand for add or delete, flag 1 when
     *        add event, flag -1, when delete event
     */
    int flag;
};

static void set_fds(private_event_t *this)
{
    int evt_cnt = 0;
    int max_fd  = 0;
    event_pkg_t *pkg = NULL;

    evt_cnt = this->event_list->get_count(this->event_list);
    if (evt_cnt < 1) return;

    FD_ZERO(&this->select_fds);
    this->event_list->reset_current(this->event_list);
    while (evt_cnt-- > 0) {
        this->event_list->get_next(this->event_list, (void **)&pkg);
        FD_SET(pkg->fd, &this->select_fds);

        if (pkg->fd > max_fd) max_fd = pkg->fd;
    }
    this->select_maxfd = max_fd + 1;
}

static volatile int thread_onoff = 1;
void *select_events_handler(private_event_t *this)
{
    int    ready_fds_cnt;
    int    deal_fds_cnd;
    int    evt_cnt = 0;
    int    can_read_bytes = 0;
    fd_set fds;
    struct timeval tv = {0};
    struct event_pkg_t *pkg = NULL;

    while (thread_onoff) {
        /**
         * set event fds
         */
        FD_ZERO(&fds);
        if (this->flag) {
            set_fds(this);
            this->flag = 0;
        }
        fds = this->select_fds;

        /**
         * wait time
         */
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
    
        /**
         * wait socket event
         */
        deal_fds_cnd = 0;
        ready_fds_cnt = select(this->select_maxfd, &fds, NULL, NULL, &tv);
        switch (ready_fds_cnt) {
            case 0:
                break;
            case -1:
                thread_onoff = 0;
                break;
            default:
                evt_cnt = this->event_list->get_count(this->event_list);
                this->event_list->reset_current(this->event_list);
                while (evt_cnt-- > 0) {
                     this->event_list->get_next(this->event_list, (void **)&pkg); 
                    if (FD_ISSET(pkg->fd, &fds)) {
                        ioctl(pkg->fd, FIONREAD, &can_read_bytes);
                        if (can_read_bytes <= 0 && pkg->type != EVENT_ON_CLOSE && pkg->type != EVENT_ON_ACCEPT)
                            continue;
                        if (pkg->event_handler != NULL) pkg->event_handler(pkg->fd, pkg->arg);
                        if (++deal_fds_cnd >= ready_fds_cnt) break;
                    }
                }
                break;
        }
    }

    return NULL;
}

static void *epoll_events_handler(private_event_t *this)
{

    return NULL;
}

static int start_event_capture(private_event_t *this, event_mode_t mode)
{
    void *handler = NULL;
    switch (mode) {
        case EVENT_MODE_SELECT:
            handler = (void *)select_events_handler;
            break;
        case EVENT_MODE_EPOLL:
            this->epoll_fd = epoll_create(10); 
            handler = (void *)epoll_events_handler;
            break;
        default:
            break;
    }
    this->thread = thread_create(handler, this);
    if (!this->thread) return -1;

    return 0;
}

static int epoll_addfd(private_event_t *this, int fd, event_type_t type)
{
    struct epoll_event evt = {0};
    evt.data.fd = fd;
    evt.events |= EPOLLIN | EPOLLRDHUP | EPOLLET;
    return epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &evt);
}

METHOD(event_t, add_, int, private_event_t *this, int fd, event_type_t type, void (*handler) (int fd, void *arg), void *arg)
{
    struct event_pkg_t *pkg = NULL;
    int evt_cnt = 0;

    if (!handler || fd < 1) return -1;
    
    /**
     * avoid same fd and same event
     */
    evt_cnt = this->event_list->get_count(this->event_list);
    while (evt_cnt-- > 0) {
        this->event_list->get_next(this->event_list, (void **)&pkg);
        if (pkg->fd == fd && pkg->type == type) {
            return 0;
        }
    }

    /**
     * malloc memory for event package
     */
    if ((pkg = (event_pkg_t *)malloc(sizeof(event_pkg_t))) == NULL) return -1;

    /**
     * event package init
     */
    pkg->fd   = fd;
    pkg->type = type;
    pkg->arg  = arg;
    pkg->event_handler = handler;

    /**
     * add socket event
     */
    switch (this->mode) {
        case EVENT_MODE_SELECT:
            this->event_list->insert_last(this->event_list, (void *)pkg);
        case EVENT_MODE_EPOLL:
            epoll_addfd(this, fd, type);
            break;
        default:
            break;
    }

    this->flag = 1;
    return 0;
}

METHOD(event_t, delete_, int, private_event_t *this, int fd, event_type_t type)
{
    event_pkg_t *del_evt_pkg = NULL;
    int evt_cnt = 0;

    evt_cnt = this->event_list->get_count(this->event_list);
    if (evt_cnt < 1) return 0;

    /**
     * delete socket event
     */
    switch (this->mode) {
        case EVENT_MODE_SELECT:
            this->event_list->reset_current(this->event_list);
            while (evt_cnt-- > 0) {
                this->event_list->get_next(this->event_list, (void **)&del_evt_pkg);
                if (del_evt_pkg->fd == fd && (del_evt_pkg->type == type || type == EVENT_ON_ALL)) {
                    this->event_list->remove(this->event_list, del_evt_pkg, NULL);
                    del_evt_pkg = NULL;
                    this->flag = -1;
                }
            }
            if (evt_cnt >=0 && del_evt_pkg == NULL) return 0;
            break;
        case EVENT_MODE_EPOLL:
            break;
        default:
            break;  
    }

    return -1;
}

METHOD(event_t, destroy_, void, private_event_t *this)
{
    if (this->event_list != NULL) {
        void *p = NULL;

        this->event_list->reset_current(this->event_list);
        while (this->event_list->remove_first(this->event_list, &p) != NOT_FOUND) free(p);
        this->event_list->destroy(this->event_list);
    }
    
    if (this->thread != NULL) {
        thread_onoff = 0;
        sleep(1);
        this->thread->cancel(this->thread);
        threads_deinit();
    }
    free(this);
}

event_t *create_event(event_mode_t mode)
{
    private_event_t *this;

    if (mode < EVENT_MODE_SELECT || mode > EVENT_MODE_EPOLL) return NULL;

    INIT(this,
        .public = {
            .add           = _add_,
            .delete        = _delete_,
            .destroy       = _destroy_,
        },
        .event_list = linked_list_create(),
        .thread     = NULL,
        .mode       = mode,
        .flag       = 0,
    );

    threads_init();
    if (start_event_capture(this, mode) < 0 || !this->event_list) {
        _destroy_(this);
        return NULL;
    }

    return &this->public;
}
