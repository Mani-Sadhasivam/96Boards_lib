
CC := gcc
CC_FLAGS := -Wall -I../include -L../lib

all: sample

%.o:%.c
	@echo "Compiling $<"
	$(CC) $(CC_FLAGS) -o "$@" -c "$<"

sample: main.o
	$(CC) $(CC_FLAGS) $^ -l96gpio -o $@

clean:
	@rm -f sample *.o