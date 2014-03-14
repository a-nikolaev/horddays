SHELL = /bin/sh
CC = gcc

.PHONY: all clean

SRCS_BASE = common.c grid.c gen.c state.c sense.c item.c sim.c init.c input.c gfx.c output.c path.c
HDRS = $(SRCS_BASE:.c=.h)
SRCS = $(SRCS_BASE) main.c
OBJS = $(SRCS:.c=.o)

EXEC = horddays
 
CFLAGS += $(shell sdl-config --cflags) -Wall -lm 
LDLIBS += $(shell sdl-config --libs)

# Build
all: $(EXEC)

clean:
	-rm -f $(OBJS) $(EXEC)

%.o: %.c $(HDRS)  
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

$(EXEC): $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(EXEC)
