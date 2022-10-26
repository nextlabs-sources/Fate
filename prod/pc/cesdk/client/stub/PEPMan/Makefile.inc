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
SUBTARGET = cepepman
SUBTARGET_STATIC=yes

# Define the objects of the target module
SRC = src/PEPMan.cpp

# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/pc/marshal/include \
  $(NLBUILDROOT)/prod/pc/transport/include 

# Define the custom library path
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

# Define the custom library to link
# eg. mylib myother_lib
LIBS += \
  cecem$(SUBTARGET_SUFFIX) \
  cebrain$(SUBTARGET_SUFFIX) \
  cemarshal50$(SUBTARGET_SUFFIX) \
  cetransport$(SUBTARGET_SUFFIX)

DFLAGS +=

# Define the test target
TESTTARGET   = pepmantest

TESTSRC      = test/pepman_test.cpp

TESTLIBPATH += \
 $(NLPLATFORMSDIR)/common/brain/$(BINDIR)         \
 $(NLENFORCERSDIR)/sdk/client/stub/CEM/$(BINDIR)  \
 $(NLENFORCERSDIR)/framework/marshal/$(BINDIR)    \
 $(NLENFORCERSDIR)/framework/transport/$(BINDIR)    \
 $(NLENFORCERSDIR)/sdk/client/stub/PEPMan/$(BINDIR) 

TESTLIBS    += \
  cebrain$(SUBTARGET_SUFFIX)			  \
  cemarshal50$(SUBTARGET_SUFFIX)                                     \
  cetransport$(SUBTARGET_SUFFIX)                                     \
  cecem$(SUBTARGET_SUFFIX)                                       \
  cepepman$(SUBTARGET_SUFFIX)
