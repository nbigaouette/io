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

# TinyXML might report failures under valgrind when compiled with
# Intel's compiler (optimized or not). To fix this, tell TinyXML
# to use STL's strings internally.
CFLAGS          += -DTIXML_USE_STL


$(eval $(call Flags_template,stdcout,StdCout.hpp,ssh://optimusprime.selfip.net/git/nicolas/stdcout.git))
$(eval $(call CFlags_template,assert,Assert.hpp,ssh://optimusprime.selfip.net/git/nicolas/assert.git))

### Just build "full" and install
.PHONY: f
f:
	$(MAKE) shared static install $(filter-out f, $(MAKECMDGOALS) )

.PHONY: shared lib_shared
shared: lib_shared
lib_shared: $(build_dir)/lib$(LIB).so
$(build_dir)/lib$(LIB).so: $(LIB_OBJ)
	############################################################
	######## Building shared library... ########################
	#
	$(LD) $(LIB_OPTIONS) -o $(build_dir)/lib$(LIB).so $(LIB_OBJ)
	######## Done ##############################################

.PHONY: static lib_static
static: lib_static
lib_static: $(build_dir)/lib$(LIB).a
$(build_dir)/lib$(LIB).a: $(LIB_OBJ)
	############################################################
	######## Building static library... ########################
	#
	ar rc $(build_dir)/lib$(LIB).a $(LIB_OBJ)
	######## Done ##############################################
#   ranlib $(build_dir)/lib$(LIB).a


### Install #####################################################
INSTALL          = $(GNU)install -m644 -D
INSTALL_EXEC     = $(GNU)install -m755 -D

DEFAULT_DESTDIR := $(HOME)/usr
ifeq ($(DESTDIR),)
DESTDIR         := $(DEFAULT_DESTDIR)
endif

### If installing to "/home/...", we don't need sudo
ifneq ($(findstring $(HOME),$(DESTDIR) ),)
    SUDO        :=
else
    SUDO        := sudo
endif

# Find which cluster we run on
# For this, we iterate through the machines list: for each one of the list,
# we check if it is part of the $(host). If any match is found, then the
# corresponding possible_host_*_match is not empty.
possible_host_HPCVL_match    := $(strip $(foreach possible_host, $(HPCVL_MACHINES),    $(findstring $(possible_host), $(host) ) ) )
# Do specific stuff for different clusters
ifneq ($(possible_host_HPCVL_match), )
INSTALL := $(subst -D,,$(INSTALL) )
INSTALL_EXEC := $(subst -D,,$(INSTALL_EXEC) )
endif

### Add compiler to library directory, but only if not GCC
DESTDIR_INC      = $(DESTDIR)/include/$(LIB)
ifeq ($(DESTDIRCOMPILER),gcc)
DESTDIR_LIB     := $(DESTDIR)/lib
else
DESTDIR_LIB     := $(DESTDIR)/lib/$(DESTDIRCOMPILER)
endif

HEADERS_NOTESTING=$(filter-out $(wildcard testing/*.$(HEADEXT)), $(HEADERS) )
HEADERS_NOTESTING_NOSRC=$(subst src/,,$(HEADERS_NOTESTING) )
HEADERS_NOTESTING_NOSRC_NOMEM=$(filter-out Constants.hpp, $(HEADERS_NOTESTING_NOSRC) )
INSTALLED_HEADERS=$(addprefix $(DESTDIR)/include/$(LIB)/, $(HEADERS_NOTESTING_NOSRC_NOMEM) )
###############################################################


### Install only the build (static,shared) stated as target
TO_INSTALL       = install_headers
ifeq ($(filter shared, $(MAKECMDGOALS)), shared)
TO_INSTALL      += install_shared
endif
ifeq ($(filter static, $(MAKECMDGOALS)), static)
TO_INSTALL      += install_static
endif
.PHONY: install
install: $(TO_INSTALL)

.PHONY: install_static
install_static: $(DESTDIR_LIB)/lib$(LIB).a install_create_folders
$(DESTDIR_LIB)/lib$(LIB).a: $(build_dir)/lib$(LIB).a
	############################################################
	######## Installing static library to $(DESTDIR_LIB)... ####
	$(SUDO) $(INSTALL) $< $@
	############################################################
	######## Done ##############################################

.PHONY: install_shared
install_shared: $(DESTDIR_LIB)/lib$(LIB).so install_create_folders
$(DESTDIR_LIB)/lib$(LIB).so: $(build_dir)/lib$(LIB).so
	############################################################
	######## Installing shared library to $(DESTDIR_LIB)... ####
	$(SUDO) $(INSTALL_EXEC) $< $@
	############################################################
	######## Done ##############################################

.PHONY: install_headers install_headers_print install_headers_print_done
ifeq ($(wildcard $(DESTDIR)/include/$(LIB)/*),)
install_headers_print: install_create_folders
	############################################################
	######## Installing library headers to $(DESTDIR_INC)... ###
install_headers_print_done:
	############################################################
	######## Done ##############################################
else
install_headers_print:
install_headers_print_done:
endif
install_headers: install_headers_print $(INSTALLED_HEADERS) install_headers_print_done
$(DESTDIR)/include/$(LIB)/%.$(HEADEXT): src/%.$(HEADEXT)
	$(SUDO) $(INSTALL) $< $@

install_create_folders:
ifneq (,$(filter $(host), $(HPCVL_MACHINES) ))
	$(SUDO) mkdir -p $(DESTDIR_LIB)
endif

.PHONY: uninstall
uninstall: force
	############################################################
	######## Removing library from $(DESTDIR_LIB)... ###
	$(SUDO) $(RM) $(DESTDIR_LIB)/lib$(LIB)*
	######## Removing library headers from $(DESTDIR_INC)... ###
	$(SUDO) $(RM) -r $(DESTDIR_INC)

test:
	# INSTALLED_HEADERS = $(INSTALLED_HEADERS)
	# HEADERS = $(HEADERS)
	# SOURCES = $(SOURCES)
	# OBJ = $(OBJ)
	# LIB = $(LIB)
	# DESTDIR = $(DESTDIR)
	# DESTDIR_INC = $(DESTDIR_INC)

############ End of file ########################################
