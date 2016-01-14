#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <utils/utils.h>
#include <utils/enum.h>
#include <thread/mutex.h>
#include <timer.h>

ENUM(timer_state_type_name, TIMER_STARTING, TIMER_DESTROYED,
    "starting",
    "running",
    "paused",
    "destroyed",
    );

static volatile unsigned int next_id = 1;
typedef struct private_timer_t private_timer_t;
struct private_timer_t {
    /**
     * public interface
     */
    timer public;

    /**
     * interval that timer need to wait
     */
    unsigned int timer_interval;

    /**
     * timer run times
     */
    unsigned int run_times;

    /**
     * Human-readable ID of this timer
     */
    unsigned int id;

    /**
     * ID of the underlying timer
     */
    pthread_t thread_id;

    /**
     * State of timer
     */
    timer_state_t state;

    /**
     * Main function of this timer
     */
    timer_main_t main;

    /**
     * Argument for the main function
     */
    void *arg;

    /**
     * Mutex to make modifying timer properties safe
     */
    mutex_t *mutex;

    /**
     * Semaphore used to sync the create/start of the timer
     */
    sem_t created;

    /**
     * control timer run or pause
     */
    bool run_or_pause;

    /**
     * control timer delete/destroy
     */
    bool terminate;
} ;

/**
 * Destroy an internal timer object
 */
static void timer_cleanup(private_timer_t *this)
{
    this->mutex->unlock(this->mutex);
    this->mutex->destroy(this->mutex);
    sem_destroy(&this->created);
    free(this);
}

/**
 * @brief Main function wrapper for timer
 */
void *timer_main(void *arg)
{
    private_timer_t *this = (private_timer_t *)arg;
    struct timeval tv = {0};
    unsigned int time_wait = 0;
    unsigned int time_interval = 1000 * 100;

    sem_wait(&this->created);
    while (!this->terminate) {
        if (!this->run_or_pause) continue;
        if (this->terminate) break;

        tv.tv_sec = 0;
        tv.tv_usec = time_interval;
        select(0, NULL, NULL, NULL, &tv);
        time_wait += time_interval / 1000;
        if (time_wait < this->timer_interval) continue;

        if (this->main) {
            this->main(this->arg);

            time_wait = 0;
            this->mutex->lock(this->mutex);
            this->run_times++;
            this->mutex->unlock(this->mutex);
        }
    }

    timer_cleanup(this);
    return NULL;
}

METHOD(timer, start, void, private_timer_t *this)
{
    this->mutex->lock(this->mutex);
    this->run_or_pause = 1;
    this->state = TIMER_RUNNING;
    this->mutex->unlock(this->mutex);
}

METHOD(timer, pause_, void, private_timer_t *this)
{
    this->mutex->lock(this->mutex);
    this->run_or_pause = 0;
    this->state = TIMER_PAUSED;
    this->mutex->unlock(this->mutex);
}

METHOD(timer, resume_, void, private_timer_t *this)
{
    this->mutex->lock(this->mutex);
    this->run_or_pause = 1;
    this->state = TIMER_RUNNING;
    this->mutex->unlock(this->mutex);
}

METHOD(timer, destroy, void, private_timer_t *this)
{
    this->mutex->lock(this->mutex);
    this->terminate = 1;
    this->state = TIMER_DESTROYED;
    this->mutex->unlock(this->mutex);
}

METHOD(timer, get_state, int, private_timer_t *this)
{
    return this->state;
}

METHOD(timer, get_state_str, char *, private_timer_t *this)
{
    return enum_to_name(timer_state_type_name, this->state);
}

METHOD(timer, get_runtimes, unsigned int, private_timer_t *this)
{
    return this->run_times;
}

METHOD(timer, set_interval, void, private_timer_t *this, unsigned int interval)
{
    this->mutex->lock(this->mutex);
    this->timer_interval = interval;
    this->mutex->unlock(this->mutex);
}

static private_timer_t *timer_create_internal()
{
    private_timer_t *this;

    INIT(this,
        .public = {
            .start = _start,
            .pause = _pause_,
            .resume = _resume_,
            .destroy = _destroy,
            .get_state = _get_state,
            .get_state_str = _get_state_str,
            .get_runtimes = _get_runtimes,
            .set_interval = _set_interval,
        },
        .state = TIMER_STARTING,
        .mutex = mutex_create(),
        .run_or_pause = 1,
        .terminate = 0,
    );
    sem_init(&this->created, 0, 0);

    return this;
}

/**
 * @brief create a new timer to run cyclely
 *
 * @param main           [in] timer main function
 * @param arg            [in] argument provided to the main function
 * @param timer_interval [in] timer waiting time, ms
 *
 * @return timer instance
 */
timer *timer_start(timer_main_t main, void *arg, unsigned int timer_interval)
{
    private_timer_t *this = timer_create_internal();
    this->main = main;
    this->arg = arg;
    if (pthread_create(&this->thread_id, NULL, timer_main, this) != 0)
    {
        fprintf(stderr, "failed to create timer\n");
        this->mutex->lock(this->mutex);
        timer_cleanup(this);

        return NULL;
    }
    this->timer_interval = timer_interval;
    this->id = next_id++;
    this->run_or_pause = 0;
    this->state = TIMER_STARTING;
    sem_post(&this->created);

    return &this->public;
}

