CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
PROGRAMS := copy_file locked_append mmap_count list_tree positional_io inotify_watch atomic_write

.PHONY: all clean
all: $(PROGRAMS)
%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@
clean:
	rm -f $(PROGRAMS)
