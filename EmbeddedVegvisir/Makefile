# IF A COMPILE COMMAND FAILS YOU MIGHT NEED TO CREATE THE "bin/" AND "bin/obj" DIRECTORIES MANUALLY
# The "bin/" directory should be in the same place as this Makefile

# ======================= Usage ===========================
#
# Use the following rules to build the project. Rules that
# begin with 'arm' will 'source' their environmant automatically.
#
#---> Build or rebuild the project:
#   make arm
#   make arm-rebuild
#
#   The 'rebuild' command will recompile all .c files.
#
#---> Send the stream executeable to one of the devices:
#   make arm-send
#
#---> You're probably going to use something like this:
#   make arm-rebuild && make arm-send
#
#=========================================================

# ARM_IP_BASE=10.220.27.
ARM_IP_BASE=192.168.0.
ARM_BDIR=""

ARM_SDK_ENV=/opt/fsl-imx-wayland/5.4-zeus/environment-setup-aarch64-poky-linux

# Required for correct behaviour of the make environment
SHELL := /bin/bash

# ====== These default settings are used for non-arm building ==========
DEFAULT-CC=gcc
DEFAULT-CFLAGS=-g -pedantic -pipe -Wall 

DATACHANNEL_CXXFLAGS= -DRTC_ENABLE_MEDIA=1 -DRTC_ENABLE_WEBSOCKET=1 -DRTC_SYSTEM_JUICE=0 -DRTC_SYSTEM_SRTP=0 -DUSE_GNUTLS=1 -DUSE_NICE=0 -Ddatachannel_EXPORTS

# Use all sorts of warning flags to find as many possile errors and bugs as possible
# Feel free to remove any or all of these warnings
DEFAULT-EXTRA-FLAGS:=-D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wall -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wextra -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -ftrapv -Wunreachable-code  -Winit-self -Werror-implicit-function-declaration

# Used by 'all' and 'all-rebuild'
DEFAULT-OPTIONS:=CC=$(DEFAULT-CC) CFLAGS='$(DEFAULT-CFLAGS) $(DEFAULT-EXTRA-FLAGS)'
#======================================================================

# Directory definitions (relevant for all build methods)
SDIR=src
HDIR=$(SDIR)/header

# .o file output directoy
ODIR=bin/obj

# Executable file output directoy
BDIR=bin

BFILE=vegvisir
BPATH=$(BDIR)/$(BFILE) # Final output file (full path with filename)

INC=-I$(HDIR)

LIB:=lib
LFLAGS=-L$(LIB) -lm -lglfw -lGLESv2
LIBDIRS:=$(shell find $(LIB) -type d)
LIBS:= $(patsubst %,-L%, $(LIBDIRS:%/=%))

VPATH=$(SDIR):$(OBJDIR):

vpath %.h $(HDIR)
# vpath %.o $(ODIR)

INCLUDE=$(INC)

# List of all .o files
OBJECTS := $(wildcard $(ODIR)/*.o)

#=================== .c and .o files automation ===================
SRC=$(wildcard $(SDIR)/*.c)
SRC_NAMES=$(subst $(SDIR)/,,$(SRC))
OBJ_INTERMEDIATE=$(subst $(SDIR),$(ODIR),$(SRC))
OBJ=$(OBJ_INTERMEDIATE:.c=.o)
OBJ_NAMES=$(SRC_NAMES:.c=.o)

# Final rule to build the executable
target: $(OBJ_NAMES)
	$(CC) $(CFLAGS) $(OBJ) $(LFLAGS) $(LIBS) -o $(BPATH) 
	@echo "Building done"

# Automated rules for building .o files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -c -o $(ODIR)/$@

# ===== Helper methods =====
.PHONY: clean				# Delete all .o files
.PHONY: v						# Run the compiler with -v flag
.PHONY: rebuild			# Run clean then build (DON'T run this manually)
									 	#  -> Used by actual building methods

# ===== Use these methods to build =====
.PHONY: all					# Build using default options
.PHONY: all-rebuild # Clean then build using default options

.PHONY: arm					# Run source for arm environment then build
.PHONY: arm-rebuild # Clean then ^^
.PHONY: arm-send

all:
	@$(DEFAULT-OPTIONS) make -j6

all-rebuild:
	@$(DEFAULT-OPTIONS) make -j6 rebuild

arm:
	@source $(ARM_SDK_ENV) && make -j6

arm-rebuild:
	@source $(ARM_SDK_ENV) && make -j6 rebuild

arm-send:
ifndef ip
	$(error "ip" variable not set. Use "make arm-send ip=XXX" to specify which device to send to.)
endif
	@echo "Copying ./$(BPATH) to root@$(ARM_IP_BASE)$(ip):~/$(ARM_BDIR) ..."
	@scp $(BPATH) root@$(ARM_IP_BASE)$(ip):~/$(ARM_BDIR)

clean:
	@for obj in $(OBJECTS) ; do \
    echo "Removing" $$obj ; \
		rm $$obj ; \
    done
	@echo "Cleanup done"

v:
	$(CC) -v

rebuild:
	@echo "Removing existing obj files..."
	@make clean
	@echo "Building the project..."
	@make -j6