// from Shadow countdown latch

#ifndef SHD_COUNT_DOWN_LATCH_H_
#define SHD_COUNT_DOWN_LATCH_H_

typedef struct _CountDownLatch CountDownLatch;

CountDownLatch* countdownlatch_new(guint count);
void countdownlatch_free(CountDownLatch* latch);

void countdownlatch_await(CountDownLatch* latch);
void countdownlatch_countDown(CountDownLatch* latch);
void countdownlatch_countDownAwait(CountDownLatch* latch);
void countdownlatch_reset(CountDownLatch* latch);

#endif /* SHD_COUNT_DOWN_LATCH_H_ */
