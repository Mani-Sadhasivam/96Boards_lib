
CC := gcc
CFLAGS := -Wall -g3 -I./include
INST_PATH := /usr

all: dirs lib96gpio

dirs:
	@mkdir -p lib obj

obj/%.o: src/%.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

lib96gpio: obj/lib96gpio.o
	@ar rcs lib/lib96gpio.a $^

clean:
	@rm -rf obj lib

install: lib96gpio
	@mkdir -p ${INST_PATH}/include
	@mkdir -p ${INST_PATH}/lib
	@cp include/* ${INST_PATH}/include
	@cp lib/* ${INST_PATH}/lib

uninstall: clean
	@rm -rf ${INST_PATH}/lib/lib96gpio.a
	@rm -rf ${INST_PATH}/include/96gpio.h

.PHONY: clean lib96gpio dirs all install uninstall
