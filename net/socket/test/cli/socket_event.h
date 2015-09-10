#ifndef __SOCKET_EVENT_H__
#define __SOCKET_EVENT_H__

#ifndef __SOCKET_BASE_H__
#include <socket_base.h>
#endif

/*************************************************************
************************  typedef  ***************************
**************************************************************/
typedef void (*event_cb)(void *arg);

/*************************************************************
*************************  enum  *****************************
**************************************************************/
typedef enum event_type 
{
    SOCKET_ON_ACCEPT  = 1,
    SOCKET_ON_CONNECT = 2, 
    SOCKET_ON_RECV    = 4,
    SOCKET_ON_SEND    = 8,
    SOCKET_ON_CLOSE   = 16
} event_type_t;

/*************************************************************
*************************  struct  ***************************
**************************************************************/
/**
 * @brief callback of socket event
 *
 * 1. evt_cb    -- function poiner of callback
 * 2. arg       -- parameter of function
 *
 */
typedef struct call_back
{
    /**
     * function poiner of callback 
     */
    event_cb evt_cb;   
    
    /**
     * parameter of function 
     */
    void *arg;        
} call_back_t;

/**
 * @brief event of socket
 *
 * 1. evt       -- type of socket event
 * 2. callback  -- callback of socket event
 *
 */
typedef struct event_loop
{   
    /**
     * callback of socket event
     */         
    
    /**
     * happen when accept socket
     */        
    struct call_back on_accept;
    
    /**
     * happen when connected server
     */   
    struct call_back on_connect;
    
    /**
     * happen when socket sending data
     */   
    struct call_back on_send;
    
    /**
     * happen when socket recving data
     */   
    struct call_back on_recv;
    
    /**
     * happen when socket breaking off
     */   
    struct call_back on_close;
} event_loop_t;

/*************************************************************
*********  Function Declaration Of Socket Event  *************
**************************************************************/
/**
 * @brief socket event init 
 *
 * @param evl [in] event loop
 */
void socket_event_init(event_loop_t *evl);

/**
 * @brief add socket event
 *
 * @param evl  [in] event loop
 * @param evt  [in] event type
 * @param cb   [in] event callback
 * @param arg  [in] parameter of callback
 */
void socket_event_add(event_loop_t *evl, event_type_t evt, event_cb cb, void *arg);

/**
 * @brief delete a socket event
 *
 * @param evl [in] event loop
 * @param evt [in] event type
 */
void socket_event_delete(event_loop_t *evl, event_type_t evt);

/**
 * @brief clear all socket events
 *
 * @param evl [in] event loop
 */
void socket_event_clearall(event_loop_t *evl);

/**
 * @brief process socket event
 *
 * @param fd [in] socket fd
 * @param cb [in] event callback
 */
void socket_event_process(struct call_back cb);

#endif