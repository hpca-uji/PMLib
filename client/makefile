# ------------------
# Usage example
# ------------------
# make PREFIX=/home/dolz/pmlib install

# ------------------
# Makefile variables
# ------------------

# Installation and search directories
PREFIX = /usr/local
LIBDIR = $(abspath $(PREFIX)/lib)
INCDIR = $(abspath $(PREFIX)/include)
BINDIR = $(abspath $(PREFIX)/bin)

# Compiler and compilation options
CC=gcc
CFLAGS +=-O3 -Wall -fPIC
OBJ= pm_set_server.o \
     pm_start_counter.o \
     pm_stop_counter.o \
     pm_get_counter_data.o \
     pm_finalize_counter.o \
     pm_continue_counter.o \
     pm_create_counter.o \
     pm_get_device_info.o \
     pm_get_devices.o \
     pm_set_lines.o \
     pm_print_data_text.o \
     pm_print_data_csv.o \
     pm_print_data_paraver.o \
     pm_print_data_stdout.o \
     pm_print_data_paraver_cstates.o \
     pm_print_data_paraver_pstates.o \
     pm_get_and_print_data_paraver.o \
     pm_calculate_energy.o \
     pm_fprint_counter_info.o 
#     pm_print_data_otf.o \

LIB = libpmlib.so
PLUGIN_LIB = libPmlibPlugin.so
VT_INSTALL_DIR=/home/dolz/Install/VampirTrace_gcc_install

# LIB = pmlib.a
# OTF_INC=-I/home/dolz/otf_install/include/open-trace-format/


# --------------
# Makefile rules
# --------------

.PHONY: lib test all install clean

lib: $(LIB)	 # default rule ('all' rule is not the default one)

plugin_lib: $(PLUGIN_LIB)

all: lib plugin_lib

$(LIB): $(OBJ)
	$(CC) -shared $(OBJ) -o $(LIB)
#	ar -cr $(LIB) $(OTF_LIB) $(OBJ)
#	ranlib $(LIB)

$(PLUGIN_LIB): $(LIB) vt_pmlib_plugin.c
	$(CC) -c -fPIC vt_pmlib_plugin.c -o libPmlibPlugin.o -I$(VT_INSTALL_DIR)/include -I$(INSTALL) `pkg-config --cflags --libs glib-2.0`
	$(CC) -shared -Wl,-soname,$(PLUGIN_LIB) -o $(PLUGIN_LIB) libPmlibPlugin.o $(LIB) `pkg-config --libs glib-2.0`

.c.o:
	$(CC) $(CFLAGS) $(OTF_INC) -D_LARGEFILE64_SOURCE -c $<

install: lib
	mkdir -p ${LIBDIR}
	install -m 644 $(LIB) $(LIBDIR)
	mkdir -p ${INCDIR}
	install -m 644 line.h pmlib.h $(INCDIR)

clean:
	rm -rf $(OBJ) $(LIB) $(PLUGIN_LIB) libPmlibPlugin.o
