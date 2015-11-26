#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct private_msg_t private_msg_t;
struct private_msg_t {
    /**
     * @brief public interface
     */
    msg_t public;
};

