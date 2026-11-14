#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct shared { long counter; long iterations; pthread_mutex_t mutex; };
static void *increment(void *arg) {
    struct shared *s = arg;
    for (long i = 0; i < s->iterations; ++i) {
        pthread_mutex_lock(&s->mutex);
        ++s->counter;
        pthread_mutex_unlock(&s->mutex);
    }
    return NULL;
}
int main(int argc, char **argv) {
    if (argc > 2) { fprintf(stderr, "用法: %s [每线程次数]\n", argv[0]); return 2; }
    long iterations = 100000; char *end = NULL;
    if (argc == 2) { errno = 0; iterations = strtol(argv[1], &end, 10); if (errno || !*argv[1] || *end || iterations < 0 || iterations > 10000000) { fputs("次数范围为 0 到 10000000\n", stderr); return 2; } }
    struct shared s = { .counter = 0, .iterations = iterations, .mutex = PTHREAD_MUTEX_INITIALIZER };
    pthread_t threads[4]; int started = 0, failed = 0;
    for (int i = 0; i < 4; ++i) {
        int error = pthread_create(&threads[i], NULL, increment, &s);
        if (error) { fprintf(stderr, "pthread_create: %s\n", strerror(error)); failed = 1; break; }
        ++started;
    }
    for (int i = 0; i < started; ++i) pthread_join(threads[i], NULL);
    printf("线程数: %d, 期望: %ld, 实际: %ld\n", started, started * iterations, s.counter);
    pthread_mutex_destroy(&s.mutex);
    return failed || s.counter != started * iterations;
}
