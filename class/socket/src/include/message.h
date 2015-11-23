#ifndef __MESSAGE_H__
#define __MESSAGE_H__

typedef enum mod_id_t mod_id_t;
enum mod_id_t {
    MOD_ID_UNKNOW = -1,
    MOD_ID_CTL
};

typedef unsigned int msg_id_t;

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

#endif /* __MESSAGE_H__ */
