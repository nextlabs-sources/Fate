# Makefile inc file for the ceInjection module
# The source files are win32 platform only

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

# Define the compiler to be used. At present it can be VS2008/VS2008
COMPILER = VS2008
PLATFORMSDK=7.0

# Define the target
TARGET=Decrypt
MANIFEST = src/Decrypt.exe.manifest

# Define the objects of the target module
SRC = src/Decrypt.cpp           \
     src/stdafx.cpp

INCPATH += \
  include  \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include/win32 

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \

# Define the custom library to link
# eg. mylib myother_lib
LIBS +=         \

ifeq ($(OS), CYGWIN_NT-5.1)
	LDFLAGS += -MANIFEST
endif


