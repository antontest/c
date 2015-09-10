#ifndef __SOCKET_MSG_H__
#define __SOCKET_MSG_H__

#ifndef __SOCKET_BASE_H__
#include "socket_base.h"
#endif

#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>

/*************************************************************
*************************  macro  ****************************
**************************************************************/
#define MOD_MAX 32
#define MSG_MAX_LEN (1024 * 32)
#define MSG_SOCKET_DIR "/var/msg/"

/*************************************************************
***********************  typedef  ****************************
**************************************************************/
typedef unsigned int    mod_id_t;
typedef unsigned int    msg_flag_t;


/*************************************************************
*************************  enum  *****************************
**************************************************************/
typedef enum msg_rst {
    MSG_RST_SUCCESS = 0,
    MSG_RST_FAILURE,
    MSG_RST_INVALID_CFG,
    MSG_RST_INVALID_MOD_ID,
    MSG_RST_SOCKET_ERROR,
    MSG_RST_THREAD_ERROR,
    MSG_RST_QUEUE_FULL
} msg_rst_t;

enum {
    MOD_ID_UNKNOW   = 0,
    MOD_ID_TMR      = 1
};

/*************************************************************
*************************  struct  ***************************
**************************************************************/
typedef struct msg {
    unsigned char   endian;
    mod_id_t        src_id;
    mod_id_t        dst_id;
    mod_id_t        mod;            /* which mod this message belong to */
    unsigned int    msg_id;         /* msg id, module releated */
    unsigned int    len;            /* message body length */
    unsigned int    hold_cnt;
    
    void (*destructor)(void *);
    void            *free_ptr;
    struct msg      *next;
    
    /**
     * Dummy message body
     */
     char body[0];
} msg_t;

#define MSG(p)              ((struct msg *)(p))
#define MSG_MOD(p)          ( MSG(p)->mod    )
#define MSG_MSGID(p)        ( MSG(p)->msg_id )
#define MSG_LEN(p)          ( MSG(p)->len    )
#define MSG_DATA(p)         ( MSG(p)->body   )
#define MSG_SRC(p)          ( MSG(p)->src_id )

typedef void            ( *msg_handler_t )(struct msg *msg);
typedef msg_flag_t      ( *msg_ident )(struct msg *msg);

/**
 * \brief Register a module for communication
 *
 *  Any message send to the module will fail until one of message 
 *  in ininitize the module.
 */
typedef struct msg_mod_cfg {
    char            *name;          /* module name */
    mod_id_t        mod_id;         /* module id */  
    pthread_t       thread_id;      /* thread id */
    msg_handler_t   handler;        /* message handler */
} msg_mod_cfg_t;


typedef struct local_msg_entry {
    /**
     *  Basic Information
     */
    mod_id_t        mod_id;
    
    /**
     *  Message Queue
     */
    int             pos;
    sem_t           sem;
    pthread_mutex_t mtx;
    struct msg      *handling_msg;
    
    /**
     * Message Handler
     */
    msg_ident       ident;
    msg_handler_t   handler;
    pthread_t       proc_thread;
    int             proc_stop;
    
    /**
     * Statistics information
     */
    unsigned int    msg_hdled_total;
    unsigned int    mod_reg_sys_uptime;
} local_entry_t;

typedef struct shm_entry {
    mod_id_t            mod_id;
    int                 flag;
    struct sockaddr_in  in_addr;
    struct sockaddr_un  un_addr;
} shm_entry_t;

typedef struct msg_ctl {
    /**
     *  Information in msg
     */
    struct loc_ctl {
        pthread_rwlock_t        rwlock;
        struct local_msg_entry  reg_tbl[ MOD_MAX ];
    } *p_loc_ctl;
    
    /**
     *  Information in shared memory
     */
    struct shm_ctl {
        pthread_rwlock_t    rwlock;
        struct shm_entry    reg_tbl[ MOD_MAX ];
    } *p_shm_ctl;
    
    /**
     *  For inter-process-communication
     */
    int         ipc_fd;
    pthread_t   ipc_thread;
    int         ipc_stop;
} msg_ctl_t;

struct msg_ctl *p_msg_ctl = NULL;

#endif
