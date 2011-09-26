#################################################################
# Main makefile
# Type "make help" for usage
#
# To compile (optimized) and install static and shared library
# with each avaible compilers, type:
# for c in pgi intel gcc 'sun studio12'; do make c $c optimized static shared install; done
#################################################################

# Project options
LIB             := inputoutput
BIN              = $(LIB)_test
SRCDIRS          = src
SRCEXT           = cpp
HEADEXT          = hpp
HEADERS          = $(wildcard $(addsuffix *.$(HEADEXT),$(addsuffix /, $(SRCDIRS)) ) )
LANGUAGE         = CPP

# Include the generic rules
include makefiles/Makefile.rules

#################################################################
# Project specific options

LIB_OBJ          = $(OBJ)

# Compression. Uncomment to enable compression. Requires libz
CFLAGS          += -DCOMPRESS_OUTPUT

### Following is needed for compression. Don't touch!
ifneq (,$(findstring -DCOMPRESS_OUTPUT,$(CFLAGS)))
libz_loc = ../libz.git/src
CFLAGS          += -I$(libz_loc)/
LIBZ_OBJ = adler32.o compress.o crc32.o deflate.o gzclose.o gzlib.o gzread.o gzwrite.o infback.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o
LIBZ_OBJ_build = $(addprefix $(build_dir)/libz/, $(notdir $(LIBZ_OBJ)) )

LIB_OBJ += $(LIBZ_OBJ_build)

$(build_dir)/libz :
	mkdir -p $(build_dir)/libz
$(build_dir)/libz/%.o : ../libz.git/src/%.o $(build_dir)/libz
	cp $< $@
endif
### End of compression block

# Project is a library. Include the makefile for build and install.
include makefiles/Makefile.library

# TinyXML might report failures under valgrind when compiled with
# Intel's compiler (optimized or not). To fix this, tell TinyXML
# to use STL's strings internally.
CFLAGS          += -DTIXML_USE_STL

### Include NetCDF support
# The order of the libraries is important!!!
CFLAGS          += -DNETCDF


$(eval $(call Flags_template,stdcout,StdCout.hpp,ssh://optimusprime.selfip.net/git/nicolas/stdcout.git))
$(eval $(call CFlags_template,assert,Assert.hpp,ssh://optimusprime.selfip.net/git/nicolas/assert.git))


############ End of file ########################################
