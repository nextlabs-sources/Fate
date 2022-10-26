# Makefile inc file for the ceInjection module
# The source files are win32 platform only

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET=ceinjection

SRC += src/CEAdapterConfig.cpp  \
	src/CEAdapter.cpp \
     src/ceBootstrapInjection.cpp \
     src/ModuleScope.cpp \
     src/Detours.cpp     

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include  \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEM/$(BINDIR)  \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1/lib/free \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)         

LDFLAGS += /DELAYLOAD:celog$(SUBTARGET_SUFFIX).dll

ifeq ($(TARGETENVARCH),x86)
LIBS +=         \
  madCHook32 \
  oleaut32 \
  celog$(SUBTARGET_SUFFIX) \
  delayimp
else
LIBS +=         \
  madCHook64 \
  oleaut32 \
  delayimp \
  celog$(SUBTARGET_SUFFIX)
endif
