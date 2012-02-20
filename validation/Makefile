#################################################################
# Main makefile
# Please edit this makefile to adapt to your project.
# Type "make help" for usage
#################################################################

# To run with a maximum of 500 MiB:
# softlimit -a 500000000 ./mdgit
# softlimit is part of http://cr.yp.to/daemontools.html

# To check memory usage:
# while [ 1 ]; do pmap -d `pidof project_name` | grep writeable | sed "s|K||g" | awk '{print ""$4" KiB    "$4/1024" MiB    "$4/1024/1024" GiB"}' ; sleep 0.1 ;done 2> /dev/null

# Project options
# Intel ICC sets LIB. Reset it here to make sure it's empty. Else code won't compile.
LIB             :=
BIN              = io_testing
SRCDIRS          = src
TESTDIRS         = unit_testing
SRCEXT           = cpp
HEADEXT          = hpp
# LANGUAGE         = C
LANGUAGE         = CPP

# Include the generic rules
include makefiles/Makefile.rules

#################################################################
# Project specific options
CFLAGS          += -DTIXML_USE_STL
LDFLAGS         += -lz

LINK_PREFERED=static

### Include NetCDF support
NETCDF_LOCAL_INSTALLATION_MACHINES = supermicro3 cosmos
ifneq (,$(filter $(host), $(NETCDF_LOCAL_INSTALLATION_MACHINES) ))
# On these machines, netcdf is installed locally.
CFLAGS          += -I${HOME}/usr/$(host)/include
netcdf_LDFLAG   += -L${HOME}/usr/$(host)/lib $(RPATH)${HOME}/usr/$(host)/lib
endif
# The order of the libraries is important!!!
CFLAGS          += -DNETCDF
netcdf_LDFLAG   += -lnetcdf -lhdf5_hl -lhdf5

# To include a library:
$(eval $(call Flags_template,inputoutput,InputOutput.hpp,ssh://optimusprime.selfip.net/git/nicolas/io.git))
$(eval $(call Flags_template,stdcout,StdCout.hpp,ssh://optimusprime.selfip.net/git/nicolas/stdcout.git))
#$(eval $(call CFlags_template,assert,Assert.hpp,ssh://optimusprime.selfip.net/git/nicolas/assert.git))

# Include the library stuff
# include makefiles/Makefile.library

############ End of file ########################################

