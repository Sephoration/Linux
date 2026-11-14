#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define CAPACITY 5
#define ITEMS 20
struct queue { int data[CAPACITY], head, tail, count, done; pthread_mutex_t mutex; pthread_cond_t not_empty, not_full; };
static void *produce(void *arg) {
    struct queue *q = arg;
    for (int i = 1; i <= ITEMS; ++i) {
        pthread_mutex_lock(&q->mutex);
        while (q->count == CAPACITY) pthread_cond_wait(&q->not_full, &q->mutex);
        q->data[q->tail] = i; q->tail = (q->tail + 1) % CAPACITY; ++q->count;
        pthread_cond_signal(&q->not_empty);
        pthread_mutex_unlock(&q->mutex);
    }
    pthread_mutex_lock(&q->mutex); q->done = 1; pthread_cond_broadcast(&q->not_empty); pthread_mutex_unlock(&q->mutex);
    return NULL;
}
static void *consume(void *arg) {
    struct queue *q = arg;
    for (;;) {
        pthread_mutex_lock(&q->mutex);
        while (!q->count && !q->done) pthread_cond_wait(&q->not_empty, &q->mutex);
        if (!q->count && q->done) { pthread_mutex_unlock(&q->mutex); break; }
        int value = q->data[q->head]; q->head = (q->head + 1) % CAPACITY; --q->count;
        pthread_cond_signal(&q->not_full); pthread_mutex_unlock(&q->mutex);
        printf("消费: %d\n", value);
    }
    return NULL;
}
int main(void) {
    struct queue q = { .mutex = PTHREAD_MUTEX_INITIALIZER, .not_empty = PTHREAD_COND_INITIALIZER, .not_full = PTHREAD_COND_INITIALIZER };
    pthread_t producer, consumer;
    if (pthread_create(&consumer, NULL, consume, &q)) { fputs("消费者创建失败\n", stderr); return 1; }
    if (pthread_create(&producer, NULL, produce, &q)) {
        pthread_mutex_lock(&q.mutex); q.done = 1; pthread_cond_signal(&q.not_empty); pthread_mutex_unlock(&q.mutex);
        pthread_join(consumer, NULL); fputs("生产者创建失败\n", stderr); return 1;
    }
    pthread_join(producer, NULL); pthread_join(consumer, NULL);
    pthread_mutex_destroy(&q.mutex); pthread_cond_destroy(&q.not_empty); pthread_cond_destroy(&q.not_full);
    return 0;
}
