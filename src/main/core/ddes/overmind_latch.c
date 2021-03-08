// modified from Shadow countdownlatch

#include <glib.h>

#include "overmind_latch.h"

struct _CountDownLatch {
    guint initialCount;
    volatile guint count;
    GCond waiters;
    GMutex lock;
};

CountDownLatch* countdownlatch_new(guint count) {
    CountDownLatch* latch = g_new0(CountDownLatch, 1);
    latch->initialCount = count;
    latch->count = count;
    g_mutex_init(&(latch->lock));
    g_cond_init(&(latch->waiters));
    return latch;
}

void countdownlatch_free(CountDownLatch* latch) {
    g_cond_clear(&(latch->waiters));
    g_mutex_clear(&(latch->lock));
    g_free(latch);
}

void countdownlatch_await(CountDownLatch* latch) {
    g_mutex_lock(&(latch->lock));
    while(latch->count > 0) {
        g_cond_wait(&(latch->waiters), &(latch->lock));
    }
    g_mutex_unlock(&(latch->lock));
}

void countdownlatch_countDown(CountDownLatch* latch) {
    g_mutex_lock(&(latch->lock));
    (latch->count)--;
    if(latch->count == 0) {
        g_cond_broadcast(&(latch->waiters));
    }
    g_mutex_unlock(&(latch->lock));
}

void countdownlatch_countDownAwait(CountDownLatch* latch) {
    g_mutex_lock(&(latch->lock));
    (latch->count)--;
    if(latch->count == 0) {
        g_cond_broadcast(&(latch->waiters));
    } else {
        g_cond_wait(&(latch->waiters), &(latch->lock));
    }
    g_mutex_unlock(&(latch->lock));
}

void countdownlatch_reset(CountDownLatch* latch) {
    g_mutex_lock(&(latch->lock));
    latch->count = latch->initialCount;
    g_mutex_unlock(&(latch->lock));
}
