# Simple Makefile for microvm_net
# For more advanced builds, use CMake

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -fPIC
LDFLAGS = -lpthread

# Directories
SRCDIR = src
INCDIR = include
EXAMPLEDIR = examples
BUILDDIR = build

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Library
LIBNAME = libmicrovm_net
SHARED_LIB = $(LIBNAME).so.1.0.0
STATIC_LIB = $(LIBNAME).a

# Examples
EXAMPLES = hello_world echo_server echo_client benchmark
EXAMPLE_BINS = $(EXAMPLES:%=$(BUILDDIR)/%)

.PHONY: all clean install examples

all: $(BUILDDIR)/$(SHARED_LIB) $(BUILDDIR)/$(STATIC_LIB)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/$(SHARED_LIB): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(LIBNAME).so.1 -o $@ $^ $(LDFLAGS)
	cd $(BUILDDIR) && ln -sf $(SHARED_LIB) $(LIBNAME).so.1
	cd $(BUILDDIR) && ln -sf $(SHARED_LIB) $(LIBNAME).so

$(BUILDDIR)/$(STATIC_LIB): $(OBJECTS)
	ar rcs $@ $^

examples: $(EXAMPLE_BINS)

$(BUILDDIR)/%: $(EXAMPLEDIR)/%.c $(BUILDDIR)/$(STATIC_LIB)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< $(BUILDDIR)/$(STATIC_LIB) $(LDFLAGS)

install: all
	install -d $(DESTDIR)/usr/local/lib
	install -d $(DESTDIR)/usr/local/include
	install -m 644 $(BUILDDIR)/$(SHARED_LIB) $(DESTDIR)/usr/local/lib/
	install -m 644 $(BUILDDIR)/$(STATIC_LIB) $(DESTDIR)/usr/local/lib/
	cp -r $(INCDIR)/* $(DESTDIR)/usr/local/include/
	ldconfig

clean:
	rm -rf $(BUILDDIR)

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build shared and static libraries"
	@echo "  examples  - Build example programs"
	@echo "  install   - Install libraries and headers"
	@echo "  clean     - Remove build files"
	@echo "  help      - Show this help"
