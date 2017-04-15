# lib96Boards
-------------

### Overview

lib96Boards aims at providing an unified interface for accessing the peripherals in 96Boards - CE family. This library works independently and has the ability to detect the board type without any user input.

The following boards are currently supported,
  1. HiKey Board
  2. Dragon Board

### Installation

From the top level, make and install library like so,

``` shell
$ make
$ make install  # run as root
```

### Usage

In your application code include `#include <lib96gpio.h>` and link to lib96boards with `-l96gpio` linker flag.  Have a look at `sample/Makefile` for more details.

The following functions are exposed by the library.
``` c
int gpio_set_mode(int gpio, enum gpio_dir);
int gpio_write(int gpio, int val);
int gpio_read(int gpio);
int gpio_set_edge(int gpio, enum gpio_edge edge);
int gpio_wait_for_edge(int gpio, enum gpio_edge edge, unsigned long timeout_ms);
int gpio_register_event_cb(int, enum gpio_edge, void (*handler)(void *), void *);
```

Also go through `sample/main.c` for use case example.

### Contributions

This repository is under development, patches and contributions are welcome. Fork this repo and raise a pull requests.
