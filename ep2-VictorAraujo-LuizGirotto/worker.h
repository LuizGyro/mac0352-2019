#ifndef __WORKER_H__
#define __WORKER_H__

#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "file.h"

typedef struct _work_args {
    pthread_mutex_t *work_done_mutex;
    bool *work_done;
    int work_number;
} work_args;

void *work(void *args);

#endif