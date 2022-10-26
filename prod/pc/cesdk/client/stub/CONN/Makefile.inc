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
SUBTARGET = ceconn
SUBTARGET_STATIC = yes

# Define the objects of the target module
SRC = src/conn.cpp

# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/pc/marshal/include  \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/PEPMan/include 

# Define the custom library path
LIBPATH += \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR)      \
  $(NLENFORCERSDIR)/sdk/client/stub/CEM/$(BINDIR) \
  $(NLBUILDROOT)/prod/pc/framework/marshal/$(BINDIR) \
  $(NLENFORCERSDIR)/framework/transport/$(BINDIR) \
  $(NLENFORCERSDIR)/sdk/client/stub/PEPMan/$(BINDIR) \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

# Define the custom library to link
# eg. mylib myother_lib
LIBS += \
  cebrain$(SUBTARGET_SUFFIX) \
  cecem$(SUBTARGET_SUFFIX) \
  cemarshal50$(SUBTARGET_SUFFIX) \
  cepepman$(SUBTARGET_SUFFIX) \
  cetransport$(SUBTARGET_SUFFIX)

DFLAGS += -DUSE_LOOPBACK_FOR_LOCALHOST_ADDR

# Define the test target
TESTTARGET   = ceconntest

TESTSRC      = test/test_conn.cpp

TESTLIBPATH += \
 $(NLPLATFORMSDIR)/common/brain/$(BINDIR)         \
 $(NLENFORCERSDIR)/sdk/client/stub/CEM/$(BINDIR)  \
 $(NLENFORCERSDIR)/framework/marshal/$(BINDIR)    \
 $(NLENFORCERSDIR)/framework/transport/$(BINDIR)    \
 $(NLENFORCERSDIR)/sdk/client/stub/CONN/$(BINDIR)  \
 $(NLENFORCERSDIR)/sdk/client/stub/PEPMan/$(BINDIR) 

TESTLIBS    += \
  cebrain$(SUBTARGET_SUFFIX) \
  cecem$(SUBTARGET_SUFFIX) \
  ceconn$(SUBTARGET_SUFFIX)
