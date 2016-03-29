#ifndef __TMR_H__
#define __TMR_H__

typedef struct tmr_arg_t tmr_arg_t;
struct tmr_arg_t {
    unsigned short wait;   /* timer interval */
    unsigned short cnt;    /* running times */
    short event;           /* timer event */
    void (*cb)(void *arg); /* timer callback */
    void *arg;             /* parameter of callback */
};

typedef struct tmr_t tmr_t;
struct tmr_t {
    /**
     * @brief start timer
     */
    int (*start) (tmr_t *this, tmr_arg_t *arg);

    /**
     * @brief pause timer by timer event
     */
    void (*pause) (tmr_t *this, short event);

    /**
     * @brief stop timer by event
     */
    void (*stop) (tmr_t *this, short event);

    /**
     * @brief destroy timer and free memory
     */
    void (*destroy) (tmr_t *this);
};

tmr_t *tmr_create();

#endif /* __TMR_H__ */
