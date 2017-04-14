
CC := gcc
CC_FLAGS := -Wall -g3 -I./include -pthread 
INST_PATH:= /usr/local

all: dirs lib96gpio

dirs:
	@mkdir -p lib obj

obj/%.o: src/%.c
	@echo "Compiling $<"
	@$(CC) $(CC_FLAGS) -o "$@" -c "$<"

lib96gpio: obj/lib96gpio.o
	@ar rcs lib/lib96gpio.a $^

clean:
	@rm -rf obj/* bin/* lib/*

install: glem
	@mkdir -p ${INST_PATH}/include
	@mkdir -p ${INST_PATH}/lib
	@cp include/* ${INST_PATH}/include
	@cp lib/* ${INST_PATH}/lib

uninstall:
	@rm -rf ${INST_PATH}/lib/lib96gpio.a
	@rm -rf ${INST_PATH}/include/96gpio.h

.PHONY: clean 96gpio dirs all install uninstall
