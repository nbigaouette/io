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

# Project is a library. Include the makefile for build and install.
include makefiles/Makefile.library

# TinyXML might report failures under valgrind when compiled with
# Intel's compiler (optimized or not). To fix this, tell TinyXML
# to use STL's strings internally.
CFLAGS          += -DTIXML_USE_STL


$(eval $(call Flags_template,stdcout,StdCout.hpp,ssh://optimusprime.selfip.net/git/nicolas/stdcout.git))
$(eval $(call CFlags_template,assert,Assert.hpp,ssh://optimusprime.selfip.net/git/nicolas/assert.git))



############ End of file ########################################
