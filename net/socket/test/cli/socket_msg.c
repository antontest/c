#include "socket_msg.h"

static msg_rst_t shm_init(struct msg_ctl *p_ctl)
{
    key_t   key = -1;
    int     shm_exist = 0;
    int     shmid = -1;
    void    *shmaddr = NULL;
    pthread_rwlockattr_t attr;
    
    if ((key = ftok("/tmp", 0x0369)) == -1) return MSG_RST_FAILURE;
    if ((shmid = shmget(key, sizeof(struct shm_ctl), IPC_CREAT | IPC_EXCL | 0x600)) == -1)
    {
        if (errno == EEXIST)
        {
            shm_exist = 1;
            shmid = shmget(key, sizeof(struct shm_ctl), 0x600);
            if (shmid == -1) return MSG_RST_FAILURE;
        }
        else return MSG_RST_FAILURE;
    }
    
    shmaddr = shmat(shmid, NULL, 0);
    if ((void *)-1 == shmaddr)
    {
        if (!shm_exist) shmctl(shmid, IPC_RMID, 0);
        return MSG_RST_FAILURE;
    }
    
    p_msg_ctl->p_shm_ctl = shmaddr;
    if (!shm_exist)
    {
        memset(p_msg_ctl->p_shm_ctl, 0, sizeof(struct shm_ctl));
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_rwlock_init(&(p_msg_ctl->p_shm_ctl->rwlock), &attr);
        pthread_rwlockattr_destroy(&attr);
    }
    
    return MSG_RST_SUCCESS;
}

static int open_local_socket(const char *path)
{
    int                 fd   = -1;
    int                 ret  = -1;
    struct sockaddr_un  addr = {0};
    
    fd = socket_create(PF_UNIX, SOCK_DGRAM);
    if (fd == -1) return ret;
    
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path));
    addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
    unlink(addr.sun_path) ;
    
    ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) 
    {
        close(fd);
        return ret;
    }
    
    return fd;
}

static void ipc_addr_init(struct sockaddr_un *ipc_addr)
{
    memset(ipc_addr, 0, sizeof(struct sockaddr_un));
    ipc_addr->sun_family = AF_UNIX;
    snprintf(ipc_addr->sun_path, sizeof(ipc_addr->sun_path), "%s%d", MSG_SOCKET_DIR, getpid());
    ipc_addr->sun_path[sizeof(ipc_addr->sun_path) - 1] = '\0';
    
    return;
}

void *ipc_func(void *arg)
{
    struct msg *msg = NULL;
    int len = 0;
    
    while (p_msg_ctl->ipc_stop)
    {
        if (NULL == (msg = malloc(MSG_MAX_LEN)))
        {
            close(p_msg_ctl->ipc_fd);
            break;
        }
        
        len = read(p_msg_ctl->ipc_fd, msg, MSG_MAX_LEN);
        if (len < sizeof(struct msg))
        {
            free(msg);
            continue;
        }
        
        msg->destructor = NULL;
        msg->free_ptr = NULL;
        msg->hold_cnt = 1;
    }
    
    return NULL;
}

static msg_rst_t ipc_init(struct msg_ctl *p_ctl)
{
    /**
     * Create unix socket for send/recv inter-process-message
     */
    struct sockaddr_un ipc_addr = {0};
    
    ipc_addr_init(&ipc_addr);
    p_msg_ctl->ipc_fd = open_local_socket(ipc_addr.sun_path);
    if (p_msg_ctl->ipc_fd == -1) return MSG_RST_FAILURE;
    
    if (pthread_create(&p_msg_ctl->ipc_thread, NULL, ipc_func, NULL) != 0)
    {
        close(p_msg_ctl->ipc_fd);
        return MSG_RST_FAILURE;
    }
    
    return MSG_RST_SUCCESS;
}

msg_rst_t msg_init()
{
    if (p_msg_ctl != NULL) return MSG_RST_SUCCESS;
    
    do {
        p_msg_ctl = (struct msg_ctl *)malloc(sizeof(struct msg_ctl));
        if (p_msg_ctl == NULL) break;
        memset(p_msg_ctl, 0, sizeof(struct msg_ctl));
        
        p_msg_ctl->p_loc_ctl = (struct loc_ctl *)malloc(sizeof(struct loc_ctl));
        if (p_msg_ctl->p_loc_ctl == NULL) break;
        memset(p_msg_ctl->p_loc_ctl, 0, sizeof(struct loc_ctl));
        
        if (shm_init(p_msg_ctl) != MSG_RST_SUCCESS) return MSG_RST_FAILURE;
        if (ipc_init(p_msg_ctl) != MSG_RST_SUCCESS) return MSG_RST_FAILURE;
        
        
        return MSG_RST_SUCCESS;
    } while (0);
    
    if (p_msg_ctl->p_loc_ctl != NULL) free(p_msg_ctl->p_loc_ctl);
    if (p_msg_ctl != NULL) free(p_msg_ctl);
    
    return MSG_RST_FAILURE;
}

msg_rst_t msg_deinit()
{
    if (p_msg_ctl == NULL) return MSG_RST_SUCCESS;
    
    if (p_msg_ctl->p_loc_ctl)
    {
        pthread_rwlock_destroy(&p_msg_ctl->p_loc_ctl->rwlock);
        free(p_msg_ctl->p_loc_ctl);
    }
    
    free(p_msg_ctl);
    p_msg_ctl = NULL;
    
    return MSG_RST_SUCCESS;
}

msg_rst_t msg_module_reg(struct msg_mod_cfg *cfg)
{
    int i = 0;
    struct loc_ctl *p_loc_ctl = p_msg_ctl->p_loc_ctl;
    struct local_msg_entry *p_entry = NULL;
    
    if (cfg == NULL || cfg->handler == NULL || p_msg_ctl == NULL) 
        return MSG_RST_FAILURE;
    
    pthread_rwlock_wrlock(&p_loc_ctl->rwlock);
    for (i = 0; i < 32; i++)
    {
        if (p_loc_ctl->reg_tbl[i].mod_id == cfg->mod_id)
        {
            p_entry = &p_loc_ctl->reg_tbl[i];
            break;
        }
        else if (!p_entry && p_loc_ctl->reg_tbl[i].mod_id == MOD_ID_UNKNOW)
            p_entry = &(p_loc_ctl->reg_tbl[i]);
            break;
    }
    
    if (p_entry == NULL)
    {
        pthread_rwlock_unlock(&p_loc_ctl->rwlock);
        return MSG_RST_FAILURE;
    }
    
    if (p_entry->mod_id == cfg->mod_id)
    {
        pthread_rwlock_unlock(&p_loc_ctl->rwlock);
        return MSG_RST_SUCCESS;
    }
    
    /* do register now */
    
    
    return MSG_RST_SUCCESS;
}
