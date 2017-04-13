
OUT_FILE := bin
SOURCE := $(PWD)/src/*.c
INCLUDE := $(PWD)/include

OUT_FILE: main.o 96boards_gpio.o
	gcc -pthread main.o 96boards_gpio.o -o ${OUT_FILE}

main.o: main.c
	gcc -Wall -g3 -I${INCLUDE} -c main.c

96boards_gpio.o: src/96boards_gpio.c
	gcc -Wall -g3 -I${INCLUDE} -c src/96boards_gpio.c

clean:
	rm -rf ./*.o ${OUT_FILE}
