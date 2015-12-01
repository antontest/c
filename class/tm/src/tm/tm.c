#include <tm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct tm *real_sys_time()
{
    static time_t now;
    static struct tm *nowtime = NULL;

    time(&now);
    nowtime = localtime(&now);

    return nowtime;
}

static struct timespec start_time;
static struct timespec run_time;
static struct timespec end_time;
void run_time_start()
{
    clock_gettime(CLOCK_REALTIME, &start_time);
}

struct timespec *run_time_end()
{
    clock_gettime(CLOCK_REALTIME, &end_time);
    run_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
    run_time.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
    
    return &run_time;
}
