#include "socket_event.h"

/*************************************************************
*********  Function Declaration Of Socket Event  *************
**************************************************************/

/**
 * @brief socket event init 
 *
 * @param evl [in] event loop
 */
void socket_event_init(event_loop_t *evl)
{
    if (evl == NULL) return;

    memset(evl, 0, sizeof(event_loop_t));

    return;
}

/**
 * @brief add socket event
 *
 * @param evl  [in] event loop
 * @param evt  [in] event type
 * @param cb   [in] event callback
 * @param arg  [in] parameter of callback
 */
void socket_event_add(event_loop_t *evl, event_type_t evt, event_cb cb, void *arg)
{
    if (evl == NULL) return;

    switch(evt)
    {
        case SOCKET_ON_ACCEPT :
            evl->on_accept.evt_cb = cb;
            evl->on_accept.arg = arg;
            break;
        case SOCKET_ON_CONNECT :
            evl->on_connect.evt_cb = cb;
            evl->on_connect.arg = arg;
            break;
        case SOCKET_ON_SEND :
            evl->on_send.evt_cb = cb;
            evl->on_send.arg = arg;
            break;
        case SOCKET_ON_RECV :
            evl->on_recv.evt_cb = cb;
            evl->on_recv.arg = arg;
            break;
        case SOCKET_ON_CLOSE :
            evl->on_close.evt_cb = cb;
            evl->on_close.arg = arg;
            break;
        default:
            break;
    }
}

/**
 * @brief delete a socket event
 *
 * @param evl [in] event loop
 * @param evt [in] event type
 */
void socket_event_delete(event_loop_t *evl, event_type_t evt)
{
    if (evl == NULL) return;

    switch(evt)
    {
        case SOCKET_ON_ACCEPT :
            evl->on_accept.evt_cb = NULL;
            evl->on_accept.arg = NULL;
            break;
        case SOCKET_ON_CONNECT :
            evl->on_connect.evt_cb = NULL;
            evl->on_connect.arg = NULL;
            break;
        case SOCKET_ON_SEND :
            evl->on_send.evt_cb = NULL;
            evl->on_send.arg = NULL;
            break;
        case SOCKET_ON_RECV :
            evl->on_recv.evt_cb = NULL;
            evl->on_recv.arg = NULL;
            break;
        case SOCKET_ON_CLOSE :
            evl->on_close.evt_cb = NULL;
            evl->on_close.arg = NULL;
            break;
        default:
            break;
    }

    return;
}

/**
 * @brief clear all socket events
 *
 * @param evl [in] event loop
 */
void socket_event_clearall(event_loop_t *evl)
{
    if (evl == NULL) return;

    evl->on_accept.evt_cb = NULL;
    evl->on_accept.arg = NULL;

    evl->on_connect.evt_cb = NULL;
    evl->on_connect.arg = NULL;

    evl->on_send.evt_cb = NULL;
    evl->on_send.arg = NULL;

    evl->on_recv.evt_cb = NULL;
    evl->on_recv.arg = NULL;

    evl->on_close.evt_cb = NULL;
    evl->on_close.arg = NULL;

    return;
}

/**
 * @brief process socket event
 *
 * @param fd [in] socket fd
 * @param cb [in] event callback
 */
void socket_event_process(int fd, struct call_back cb)
{
    if (cb.evt_cb == NULL) return;

    cb.evt_cb(fd, cb.arg);

    return;
}
