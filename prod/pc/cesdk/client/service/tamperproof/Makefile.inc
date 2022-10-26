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
SUBTARGET=ceTamperproof

# Define the objects of the target module
SRC = src/policy.cpp           \
     src/tamper${OS}.cpp


# Define the custom header include path
INCPATH += \
  ./include  \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include

ifeq ($(OS), CYGWIN_NT-5.1)
  INCPATH += $(NLBUILDROOT)/prod/pc/ceInjection/include
endif

# Define the custom library path
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

# Define the custom library to link
# eg. mylib myother_lib
LIBS += cesdk$(SUBTARGET_SUFFIX) cebrain$(SUBTARGET_SUFFIX)

ifeq ($(OS), CYGWIN_NT-5.1)
       LIBS += ceInjection$(SUBTARGET_SUFFIX)
endif

DFLAGS +=
