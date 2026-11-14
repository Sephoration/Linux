CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
PROGRAMS = tcp_echo_server tcp_echo_client thread_counter signal_wait child_status poll_stdin timerfd_demo
.PHONY: all clean
all: $(PROGRAMS)
tcp_echo_server: tcp_echo_server.c
	$(CC) $(CFLAGS) $< -o $@
tcp_echo_client: tcp_echo_client.c
	$(CC) $(CFLAGS) $< -o $@
thread_counter: thread_counter.c
	$(CC) $(CFLAGS) -pthread $< -o $@
signal_wait: signal_wait.c
	$(CC) $(CFLAGS) $< -o $@
child_status: child_status.c
	$(CC) $(CFLAGS) $< -o $@
poll_stdin: poll_stdin.c
	$(CC) $(CFLAGS) $< -o $@
timerfd_demo: timerfd_demo.c
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -f $(PROGRAMS)
