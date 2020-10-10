# A simple Makefile for compiling small SDL projects

# set the compiler
CC := gcc

# set the compiler flags
CFLAGS := -std=c99 -Wall -Wextra -Wconversion -Wpedantic `sdl2-config --cflags --libs` -lSDL2_image -lm

# add header files here
HDRS :=

# add source files here
SRCS := Blorpgame.c modules/blit/blit.c modules/input/input.c modules/textures/textures.c modules/update/update.c modules/renderer/renderer.c 
# generate names of object files
OBJS := $(SRCS:.c=.o)

# name of executable
EXEC := opdracht5

# default recipe
all: $(EXEC)

# recipe for building the final executable
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)

# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#	$(CC) -o $@ $(@:.o=.c) -c $(CFLAGS)

# recipe to clean the workspace
clean:
	rm -f $(EXEC) *.o

.PHONY: all clean
