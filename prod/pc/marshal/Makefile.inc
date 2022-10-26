# Makefile inc file for the brain module
# The source files are OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = cemarshal50
SUBTARGET_STATIC=yes

# Define the objects of the target module
SRC = src/marshal.cpp

# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include    \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include

# Define the custom library path
LIBPATH += \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR)      \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEM/$(BINDIR)

# Define the custom library to link
# eg. mylib myother_lib
LIBS += \
  cecem$(SUBTARGET_SUFFIX) \
  cebrain$(SUBTARGET_SUFFIX)

DFLAGS +=

