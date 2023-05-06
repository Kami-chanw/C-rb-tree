CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -I./include
LDFLAGS_STATIC = -L./lib -lrb_tree
LDFLAGS_SHARED = -L./lib -Wl,-rpath=./lib -lrb_tree

SRCDIR = ./src
OBJDIR = ./examples
LIBDIR = ./lib
EXEDIR = ./examples

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

.PHONY: all static dynamic test clean

all: static dynamic test

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

static: $(LIBDIR)/librb_tree.a

$(LIBDIR)/librb_tree.a: $(OBJECTS)
	mkdir -p $(LIBDIR)
	ar rcs $@ $^

dynamic: $(LIBDIR)/librb_tree.so

$(LIBDIR)/librb_tree.so: $(OBJECTS)
	mkdir -p $(LIBDIR)
	$(CC) -shared -fPIC $(LDFLAGS_SHARED) $^ -o $@

test: $(EXEDIR)/test

$(EXEDIR)/test: $(EXEDIR)/test.o
	mkdir -p $(EXEDIR)
	$(CC) $(LDFLAGS_STATIC) $^ -o $@

$(EXEDIR)/%.o: $(EXEDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*.o $(LIBDIR)/*.a $(LIBDIR)/*.so $(EXEDIR)/*.o $(EXEDIR)/test
