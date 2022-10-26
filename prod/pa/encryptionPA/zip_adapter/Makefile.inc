# Makefile inc file for the brain module
# The source files are OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

# Define the compiler to be used. At present it can be VS2008/VS2008
COMPILER = VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = zip_adapter
MANIFEST = $(BINDIR)/zip_adapter$(SUBTARGET_SUFFIX).dll.manifest

# On Windows, add MD
# Fix Bug 11440 by sharing dynamic linked CRT with pa_encrypt.dll
DYNAMICCRT=yes

DFLAGS+=-D_USRDLL -DADAPTER_BASE_EXPORTS 

# Define the objects of the target module
SRC +=  \
	src/stdafx.cpp \
	src/log.cpp \
	src/zip_adapter.cpp 

RCSRC = src/zip_adapter.rc
  
# Define the custom header include path
INCPATH += \
  include  \
  ../include \
  $(NLEXTERNALDIR)/zip_utils/zip_utils2005
 
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  lib/release 

  
ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
    $(NLEXTERNALDIR)/zip_utils/zip_utils2005/$(BUILDTYPE)
else
  LIBPATH += \
  	$(NLEXTERNALDIR)/zip_utils/zip_utils2005/x64-$(BUILDTYPE)
endif

LIBS += zip_utils

# Define the custom library to link
# eg. mylib myother_lib

DFLAGS +=

