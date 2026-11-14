CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2

.PHONY: all clean
all: dir_stats/dir_stats pipe_exec/pipe_exec producer_consumer/producer_consumer

dir_stats/dir_stats: dir_stats/dir_stats.c
	$(CC) $(CFLAGS) $< -o $@
pipe_exec/pipe_exec: pipe_exec/pipe_exec.c
	$(CC) $(CFLAGS) $< -o $@
producer_consumer/producer_consumer: producer_consumer/producer_consumer.c
	$(CC) $(CFLAGS) -pthread $< -o $@
clean:
	rm -f dir_stats/dir_stats pipe_exec/pipe_exec producer_consumer/producer_consumer
