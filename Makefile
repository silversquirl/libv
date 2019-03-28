.PHONY: all clean

include config.mk
override CFLAGS += -pedantic -Wall -Werror

# If no version is explicitly set, we need to detect it
VERSION ?= $(patsubst v%,%,$(shell git describe))

# Used by clean
GENERATED_HEADERS := $(patsubst %,v%.h,$(HEADER_COMPONENTS))

OBJECTS := $(patsubst %,v%.o,$(BINARY_COMPONENTS))
HEADERS := $(GENERATED_HEADERS) $(patsubst %,v%.h,$(BINARY_COMPONENTS))

all: libv.a libv.pc

clean:
	rm -f $(OBJECTS)
	rm -f $(GENERATED_HEADERS)
	rm -f libv.a libv.pc

# To people (probably me) trying to decipher this: glhf
# (Also this might help: https://www.gnu.org/software/make/manual/html_node/Secondary-Expansion.html)
.SECONDEXPANSION:
$(GENERATED_HEADERS): $$(patsubst %.h,%.scriptc.h,$$@) $$(wildcard scripts/$$(patsubst %.h,%,$$@)/*) $(SCRIPTC)
	$(SCRIPTC) $< $@

# We need the headers as a dep because some of them are preprocessed by scriptc
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

libv.a: $(OBJECTS) vmisc.o
	$(AR) rcs $@ $^

# This mess of Makefile generates a sed expression that substitutes the specified Make variables' names for their values
sed_subst = s/\b$1\b/$(subst ','\'',$(subst /,\/,$(subst \,\\,$2)))/g
space :=
space +=
sed_subvars = $(subst $(space),;,$(foreach a,$1,$(call sed_subst,$a,$($a))))

libv.pc: libv_template.pc
	$(SED) -E '$(call sed_subvars,INCLUDE_DIR LIB_DIR VERSION)' < $< > $@
