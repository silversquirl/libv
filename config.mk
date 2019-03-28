# Components of libv to be built
BINARY_COMPONENTS := fs io misc time
HEADER_COMPONENTS := hacks

# Current version of libv
VERSION := 0.1.0

### Autoconfiguration ###
# Return the first command in the list that exists on the system
ac_choose = $(or $(firstword $(foreach a,$1,$(if $(shell which $a 2>&-),$a))),$(error Could not find any of $1))

# C compiler to use
CC := $(call ac_choose,clang gcc) -std=c99
# C compiler flags to use
CFLAGS :=
# Linker flags to use
LDFLAGS :=

# Static library archiver to use
AR := ar

# Stream editor to use
SED := sed

# Script preprocessor to use
SCRIPTC := ./scripts/scriptc.sh

# Installation directories
PREFIX := /usr/local
EXEC_PREFIX := $(PREFIX)
INCLUDE_DIR := $(PREFIX)/include
LIB_DIR := $(EXEC_PREFIX)/lib
