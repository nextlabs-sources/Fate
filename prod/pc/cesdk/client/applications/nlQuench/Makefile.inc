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
TARGET = nlQuench

# Define the objects of the target module
SRC = \
  src/nlQuench

# Define the custom header include path
INCPATH += \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include  \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEPrivate/include \
  $(NLBUILDROOT)/prod/pc/quenchhash/include

# Define the custom library path
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

# Define the custom library to link
# eg. mylib myother_lib
LIBS += \
  Psapi \
  cequenchhash_static$(SUBTARGET_SUFFIX) \
  cebrain$(SUBTARGET_SUFFIX) \
  Advapi32 \
  iphlpapi
