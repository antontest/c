#include "bsem.h"

/******************************************************
 *********** Binary Semaphore Function ****************
 ******************************************************/
/**
 * @brief init semaphore to 1 or 0
 *
 * @param bsem_p [in] binary semaphore
 * @param value [in] take only 1 or 0
 */
void bsem_init(bsem *bsem_p, int value)
{
    if (value < 0 || value > 1) {
        fprintf(stderr, "bsem_init(): Binary semaphore can take only values 1 or 0");
        exit(1);
    }

    pthread_mutex_init(&bsem_p->mutex, NULL);
    pthread_cond_init(&bsem_p->cond, NULL);
    bsem_p->v = value;
}

/**
 * @brief reset semaphore to 0
 *
 * @param bsem_p [in] semaphore
 */
void bsem_reset(bsem *bsem_p)
{
    bsem_init(bsem_p, 0);
}

/**
 * @brief post to at least one thread
 *
 * @param bsem_p
 */
void bsem_post(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_signal(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

/**
 * @brief post all threads
 *
 * @param bsem_p
 */
void bsem_post_all(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_broadcast(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

/**
 * @brief wait on semaphore until semaphore has value 0
 *
 * @param bsem_p
 */
void bsem_wait(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    while (bsem_p->v != 1) {
        pthread_cond_wait(&bsem_p->cond, &bsem_p->mutex);
    }
    pthread_mutex_unlock(&bsem_p->mutex);
}

/**
 * @brief destroy binary semaphore
 *
 * @param bsem_p
 */
void bsem_destroy(bsem *bsem_p)
{
    pthread_mutex_destroy(&bsem_p->mutex);
    pthread_cond_destroy(&bsem_p->cond);
    bsem_p->v = 0;
}
