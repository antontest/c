#ifndef __MESSAGE_H__
#define __MESSAGE_H__

typedef enum mod_id_t mod_id_t;
enum mod_id_t {
    MOD_ID_UNKNOW = -1,
    MOD_ID_CTL
};

typedef enum msg_id_t msg_id_t;
enum msg_id_t {
    MSG_ID_UNKNOW = -1,
    MSG_ID_TEST
};

typedef struct msg_t msg_t;
struct msg_t {
    /**
     * @brief this module belong to
     */
    mod_id_t src_mod;

    /**
     * @brief destine module belong to
     */
    mod_id_t dst_mod;

    /**
     * @brief message id
     */
    msg_id_t msg_id;

    /**
     * @brief length of message
     */
    unsigned int msg_len;

    /**
     * @brief content of message
     */
    char data[0];
};

typedef struct mod_cfg_t mod_cfg_t;
struct mod_cfg_t {
    mod_id_t id;
    const char *name;
    void (*handler) (msg_t *msg);
};

typedef struct msg_mod_t msg_mod_t;
struct msg_mod_t {
    /**
     * @brief register module
     */
    int (*register_mod) (msg_mod_t *this, mod_cfg_t *cfg);
    
    /**
     * @brief destroy instance and free memory 
     */
    void (*destroy) (msg_mod_t *this);
};

/**
 * @brief create msg_mod instance 
 */
msg_mod_t *create_msg_mod();

typedef struct msg_handler_t msg_handler_t;
/**
 * @brief create msg_handler instance 
 */
msg_handler_t *create_msg_handler();
int send_msg(msg_t *msg);

#endif /* __MESSAGE_H__ */
