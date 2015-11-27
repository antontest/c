#ifndef __TASK_MANAGER_H__
#define __TASK_MANAGER_H__

typedef enum task_type_t task_type_t;
enum task_type_t {
    TASK_TYPE_COMMAND = 0,
    TASK_TYPE_SYSN_TIME
};

typedef struct task_msg_t task_msg_t;
struct task_msg_t {
    /**
     * @brief task type
     */
    task_type_t type;

    /**
     * @brief message length
     */
    int len;

    /**
     * @brief task message data 
     */
    char data[0];
};

#endif /* __TASK_MANAGER_H__ */
