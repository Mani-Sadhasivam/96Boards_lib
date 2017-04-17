
CC := gcc
CFLAGS := -Wall -pthread

all: sample

%.o:%.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

sample: main.o
	@$(CC) $(CFLAGS) $^ -l96gpio -o $@

clean:
	@rm -f sample *.o

.PHONY: clean sample all
