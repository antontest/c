#ifndef __BSEM_H__
#define __BSEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <semaphore.h>

/******************************************************
 ********************* Struct *************************
 ******************************************************/
/**
 * @brief Binary semaphore
 */
typedef struct bsem {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int v;
} bsem;

/******************************************************
 *********** Binary Semaphore Function ****************
 ******************************************************/
/**
 * @brief init semaphore to 1 or 0
 *
 * @param bsem_p [in] binary semaphore
 * @param value [in] take only 1 or 0
 */
void bsem_init(bsem *bsem_p, int value);

/**
 * @brief reset semaphore to 0
 *
 * @param bsem_p [in] semaphore
 */
void bsem_reset(bsem *bsem_p);

/**
 * @brief post to at least one thread
 *
 * @param bsem_p
 */
void bsem_post(bsem *bsem_p);

/**
 * @brief post all threads
 *
 * @param bsem_p
 */
void bsem_post_all(bsem *bsem_p);

/**
 * @brief wait on semaphore until semaphore has value 0
 *
 * @param bsem_p
 */
void bsem_wait(bsem *bsem_p);

/**
 * @brief destroy binary semaphore
 *
 * @param bsem_p
 */
void bsem_destroy(bsem *bsem_p);

#endif
