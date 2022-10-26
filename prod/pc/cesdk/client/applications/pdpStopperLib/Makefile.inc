# Makefile inc file for the pepman module
# The source files are not OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = PDPStop

SRC = \
  src/agentController.cpp 

INCPATH += \
  include                                                \
  $(NLBUILDROOT)/prod/common/brain/include                 \
  $(NLBUILDROOT)/prod/common/include                       \
  $(NLBUILDROOT)/prod/pc/cesdk/include                          \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEPrivate/include 

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR) \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEPrivate/$(BINDIR)

LIBS += \
  cesdk$(SUBTARGET_SUFFIX) \
  ceprivate$(SUBTARGET_SUFFIX)

DFLAGS +=

LDFLAGS  +=

# Define the test target
TESTTARGET   = test_pdpstop

TESTSRC      = test/test_pdpstop.cpp

TESTLIBPATH += \
 $(NLBUILDROOT)/prod/pc/cesdk/client/applications/pdpStopperLib/$(BINDIR) \
 $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEPrivate/$(BINDIR)

TESTLIBS    += \
  cesdk$(SUBTARGET_SUFFIX)			  \
  ceprivate$(SUBTARGET_SUFFIX)                                       \
  PDPStop$(SUBTARGET_SUFFIX)

ifeq (${OS}, Linux)
CUSTOM += copytest
endif
