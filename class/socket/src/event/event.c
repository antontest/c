#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <thread/thread.h>
#include <utils/utils.h>
#include <utils/linked_list.h>

#define DFT_EPOLL_EVTS_MAX_SIZE (10)
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

typedef struct callback_t callback_t;
struct callback_t {
    void (*handler) (void *arg);
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
            fd_set rfds;

            /**
             * @brief fd sets listening on
             */
            fd_set wfds;
        } s;
#define select_rfds   fd.s.rfds
#define select_wfds   fd.s.wfds
#define select_maxfd  fd.s.max_fd

        struct {
            /**
             * @brief handle of epoll
             */
            int epfd;
        } e;
#define epoll_fd fd.e.epfd
    } fd;

    /**
     * @brief select or epoll timeout
     */
    unsigned int timeout;

    /**
     * @brief stand for add or delete, flag 1 when
     *        add event, flag -1, when delete event
     */
    int flag;

    /**
     * @brief select or epoll timeout handle 
     */
    callback_t timeout_handler;

    /**
     * @brief select or epoll error handle 
     */
    callback_t error_handler;
};
static private_event_t *local_free_pointer = NULL;

static void set_fds(private_event_t *this)
{
    int evt_cnt = 0;
    int max_fd  = 0;
    event_pkg_t *pkg = NULL;

    evt_cnt = this->event_list->get_count(this->event_list);
    if (evt_cnt < 1) return;

    FD_ZERO(&this->select_rfds);
    this->event_list->reset_current(this->event_list);
    while (evt_cnt-- > 0) {
        this->event_list->get_next(this->event_list, (void **)&pkg);
        if (pkg->type == EVENT_ON_CONNECT) FD_SET(pkg->fd, &this->select_wfds);
        else FD_SET(pkg->fd, &this->select_rfds);

        if (pkg->fd > max_fd) max_fd = pkg->fd;
    }
    this->select_maxfd = max_fd + 1;
}

static volatile int thread_onoff = 1;
void *select_events_handler(private_event_t *this)
{
    int    ready_fds_cnt;
    int    deal_fds_cnt;
    int    evt_cnt = 0;
    int    can_read_bytes = 0;
    fd_set rfds, wfds;
    struct timeval tv = {0};
    struct event_pkg_t *pkg = NULL;

    while (thread_onoff) {
        /**
         * set event fds
         */
        if (this->flag) {
            set_fds(this);
            this->flag = 0;
        }
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        rfds = this->select_rfds;
        wfds = this->select_wfds;

        /**
         * wait time
         */
        tv.tv_sec  = this->timeout / 1000;
        tv.tv_usec = this->timeout % 1000 * 1000;
    
        /**
         * wait socket event
         */
        deal_fds_cnt = 0;
        ready_fds_cnt = select(this->select_maxfd, &rfds, &wfds, NULL, &tv);
        switch (ready_fds_cnt) {
            case 0:
                if (this->timeout_handler.handler != NULL) this->timeout_handler.handler(this->timeout_handler.arg);
                break;
            case -1:
                if (this->error_handler.handler != NULL) this->error_handler.handler(this->error_handler.arg);
                thread_onoff = 0;
                break;
            default:
                evt_cnt = this->event_list->get_count(this->event_list);
                this->event_list->reset_current(this->event_list);
                while (evt_cnt-- > 0) {
                     this->event_list->get_next(this->event_list, (void **)&pkg); 
                    if (FD_ISSET(pkg->fd, &rfds)) {
                        ioctl(pkg->fd, FIONREAD, &can_read_bytes);
                        if (can_read_bytes <= 0 && pkg->type != EVENT_ON_CLOSE && pkg->type != EVENT_ON_ACCEPT)
                            continue;
                        if (pkg->event_handler != NULL) pkg->event_handler(pkg->fd, pkg->arg);
                        if (++deal_fds_cnt >= ready_fds_cnt) break;
                    } else if (FD_ISSET(pkg->fd, &wfds)) {
                        if (pkg->type != EVENT_ON_CONNECT) continue;
                        if (pkg->event_handler != NULL) pkg->event_handler(pkg->fd, pkg->arg);
                        if (++deal_fds_cnt >= ready_fds_cnt) break;
                    }
                }
                break;
        }
    }

    return NULL;
}

static void *epoll_events_handler(private_event_t *this)
{
    struct epoll_event evts[DFT_EPOLL_EVTS_MAX_SIZE];
    int    ready_fds_cnt;
    int    deal_fds_cnt;
    int    evt_cnt = 0;
    struct event_pkg_t *pkg = NULL;

    while (thread_onoff) {
        ready_fds_cnt = epoll_wait(this->epoll_fd, evts, sizeof(evts) / sizeof(struct epoll_event), this->timeout);    
        switch (ready_fds_cnt) {
            case -1:
                if (this->error_handler.handler != NULL) this->error_handler.handler(this->error_handler.arg);
                break;
            case 0:
                if (this->timeout_handler.handler != NULL) this->timeout_handler.handler(this->timeout_handler.arg);
                break;
            default:
                for (deal_fds_cnt = 0; deal_fds_cnt < ready_fds_cnt; deal_fds_cnt++) {
                    evt_cnt = this->event_list->get_count(this->event_list);
                    this->event_list->reset_current(this->event_list);
                    while (evt_cnt-- > 0) {
                        this->event_list->get_next(this->event_list, (void **)&pkg); 
                        if (pkg->fd != evts[deal_fds_cnt].data.fd) continue;
                        if (evts[deal_fds_cnt].events & EPOLLRDHUP) {
                            if (pkg->type != EVENT_ON_CLOSE) continue;
                            else
                                epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, evts[deal_fds_cnt].data.fd, &evts[deal_fds_cnt]);
                        }
                            
                        if ((pkg->type == EVENT_ON_ACCEPT) ||
                            (evts[deal_fds_cnt].events & EPOLLIN) || 
                            (evts[deal_fds_cnt].events & EPOLLRDHUP) || 
                            (evts[deal_fds_cnt].events & EPOLLOUT)) {
                            if (pkg->event_handler != NULL) {
                                pkg->event_handler(pkg->fd, pkg->arg);
                            }
                            break;
                        }
                    }
                }
                break;
        }
    }

    return NULL;
}

static void signal_handler(int sig, siginfo_t *siginfo, void *context)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            local_free_pointer->public.destroy(&local_free_pointer->public);
            exit(1);
            break;
        default:
            break;
    }
}

static int epoll_event_add(private_event_t *this, int fd, event_type_t type)
{
    struct epoll_event evt  = {0};
    struct event_pkg_t *pkg = NULL;
    int epctl   = EPOLL_CTL_ADD;
    int evt_cnt = 0;

    evt_cnt = this->event_list->get_count(this->event_list);
    this->event_list->reset_current(this->event_list);
    while (evt_cnt-- > 0) {
        this->event_list->get_next(this->event_list, (void **)&pkg);
        if (pkg->fd == fd) {
            epctl = EPOLL_CTL_MOD;
            break;
        }
    }

    switch (type) {
        case EVENT_ON_ACCEPT:
            evt.events |= EPOLLIN | EPOLLHUP | EPOLLRDHUP;
            break;
        case EVENT_ON_RECV:
            evt.events |= EPOLLIN;
            break;
        case EVENT_ON_CONNECT:
            evt.events |= EPOLLOUT;
            break;
        case EVENT_ON_CLOSE:
            evt.events |= EPOLLIN | EPOLLRDHUP | EPOLLHUP;
            break;
        default:
            break;
    }
    evt.data.fd = fd;
    evt.events |= EPOLLET;

    return epoll_ctl(this->epoll_fd, epctl, fd, &evt);
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
        case EVENT_MODE_EPOLL:
            epoll_event_add(this, fd, type);
            break;
        default:
            break;
    }

    this->event_list->insert_last(this->event_list, (void *)pkg);
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
        case EVENT_MODE_EPOLL:
            break;
        default:
            break;  
    }

    this->event_list->reset_current(this->event_list);
    while (evt_cnt-- > 0) {
        this->event_list->get_next(this->event_list, (void **)&del_evt_pkg);
        if (del_evt_pkg->fd == fd && (del_evt_pkg->type == type || type == EVENT_ON_ALL)) {
            if (this->mode == EVENT_MODE_EPOLL) {
                struct epoll_event del_evt = {0};
                del_evt.data.fd = fd;
                epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, &del_evt);
            }
            this->event_list->remove(this->event_list, del_evt_pkg, NULL);
            del_evt_pkg = NULL;
            this->flag = -1;
        }
    }
    if (evt_cnt >=0 && del_evt_pkg == NULL) return 0;

    return -1;
}

METHOD(event_t, destroy_, void, private_event_t *this)
{
    if (this->thread != NULL) {
        thread_onoff = 0;
        usleep(100);
        this->thread->cancel(this->thread);
    }

    if (this->event_list != NULL) {
        this->event_list->destroy(this->event_list);
    }
    
    free(this);
}

METHOD(event_t, exception_handle_, void, private_event_t *this, exception_type_t type, void (*handler) (void *), void *arg)
{
    switch (type) {
        case EXCEPTION_TIMEOUT:
            this->timeout_handler.handler = handler;
            this->timeout_handler.arg = arg;
            break;
        case EXCEPTION_ERROR:
            this->error_handler.handler = handler;
            this->error_handler.arg = arg;
            break;
        default:
            break;
    }
}

static int start_event_capture(private_event_t *this, event_mode_t mode)
{
    void *handler = NULL;
    struct sigaction act;
    
    /**
     * act signal
     */
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &signal_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    /**
     * act socket event
     */
    switch (mode) {
        case EVENT_MODE_SELECT:
            handler = (void *)select_events_handler;
            break;
        case EVENT_MODE_EPOLL:
            this->epoll_fd = epoll_create(DFT_EPOLL_EVTS_MAX_SIZE); 
            handler = (void *)epoll_events_handler;
            break;
        default:
            break;
    }
    this->thread = thread_create(handler, this);
    if (!this->thread) return -1;

    return 0;
}

event_t *create_event(event_mode_t mode, int timeout)
{
    private_event_t *this;

    if (mode < EVENT_MODE_SELECT || mode > EVENT_MODE_EPOLL) return NULL;

    INIT(this,
        .public = {
            .add     = _add_,
            .delete  = _delete_,
            .destroy = _destroy_,
            .exception_handle = _exception_handle_,
        },
        .event_list = linked_list_create(),
        .thread     = NULL,
        .mode       = mode,
        .flag       = 0,
        .timeout    = timeout < 0 ? 0 : timeout,
    );

    threads_init();
    if (start_event_capture(this, mode) < 0 || !this->event_list) {
        _destroy_(this);
        return NULL;
    }

    local_free_pointer = this;
    return &this->public;
}
