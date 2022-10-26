# Makefile inc file for the pepman module
# The source files are not OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = cepdpsec

# Define the objects of the target module
SRC = src/PDPSec.cpp 

# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include            \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include   \
  $(NLBUILDROOT)/prod/common/celog/include           \
  $(JDKDIR)/include \
  $(JDKDIR)/include/win32

# Define the custom library path
LIBPATH += \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR)      \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)             \
  $(BUILDOUTPUTDIR)/$(LIBDIR)  \

LIBS += \
  cesdk$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX)

