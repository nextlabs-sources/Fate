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
SUBTARGET = cepdpeval

# Define the objects of the target module
SRC = src/PDPEval.cpp 

# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/pc/transport/include \
  $(NLBUILDROOT)/prod/pc/TransCtrl/include \
  $(NLBUILDROOT)/prod/pc/marshal/include \
  $(NLBUILDROOT)/prod/pc/kif/include \
  $(NLBUILDROOT)/prod/pc/tamper/TamperproofConfig/include    \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(JDKDIR)/include	\
  $(JDKDIR)/include/win32

# Define the custom library path
LIBPATH += \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR)      \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)      \
  $(NLBUILDROOT)/prod/pc/tamper/TamperproofConfig/$(BINDIR)      \
  $(NLBUILDROOT)/prod/pc/cesdk/client/SingleDLL/$(BINDIR)      \
  $(BUILDOUTPUTDIR)/$(LIBDIR)  \
  $(JDKDIR)/lib 	

LIBS += \
  cebrain_static$(SUBTARGET_SUFFIX) \
  cesdk$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX) \
  nlTamperproofConfig$(SUBTARGET_SUFFIX) \
  mpr

ifeq ($(TARGETENVOS),linux)
LIBS += \
  cebrain \
  celog \
else
LIBS += \
  cebrain_static$(SUBTARGET_SUFFIX) \
  cesdk$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX) \
  nlTamperproofConfig$(SUBTARGET_SUFFIX)
endif

LIBS += \
  psapi

DFLAGS +=

