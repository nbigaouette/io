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
# Seems to cause a lot of overhead in saving the screen log (std_cout)
#CFLAGS          += -DCOMPRESS_OUTPUT

### Following is needed for compression. Don't touch!
ifneq (,$(findstring -DCOMPRESS_OUTPUT,$(CFLAGS)))
LDFLAGS         += -lz
endif
### End of compression block

# Project is a library. Include the makefile for build and install.
include makefiles/Makefile.library

# TinyXML might report failures under valgrind when compiled with
# Intel's compiler (optimized or not). To fix this, tell TinyXML
# to use STL's strings internally.
CFLAGS          += -DTIXML_USE_STL

NETCDF_LOCAL_INSTALLATION_MACHINES = supermicro3 cosmos
ifneq (,$(filter $(host), $(NETCDF_LOCAL_INSTALLATION_MACHINES) ))
# On these machines, netcdf is installed locally.
CFLAGS          += -I${HOME}/usr/$(host)/include
endif
### Include NetCDF support
CFLAGS          += -DNETCDF


$(eval $(call Flags_template,stdcout,StdCout.hpp,ssh://optimusprime.selfip.net/git/nicolas/stdcout.git))
$(eval $(call CFlags_template,assert,Assert.hpp,ssh://optimusprime.selfip.net/git/nicolas/assert.git))


############ End of file ########################################
