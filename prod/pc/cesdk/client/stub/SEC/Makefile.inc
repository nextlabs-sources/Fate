
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = cesec
SUBTARGET_STATIC=yes

SRC = src/cesec.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include     \
  $(NLBUILDROOT)/prod/pc/marshal/include  \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/PEPMan/include 

LIBPATH += \
  $(NLPLATFORMSDIR)/common/brain/$(BINDIR)      \
  $(NLENFORCERSDIR)/sdk/client/stub/CEM/$(BINDIR) \
  $(NLENFORCERSDIR)/framework/marshal/$(BINDIR) \
  $(NLENFORCERSDIR)/framework/transport/$(BINDIR) \
  $(NLENFORCERSDIR)/sdk/client/stub/PEPMan/$(BINDIR) \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

# Define the custom library to link
# eg. mylib myother_lib
LIBS += \
  cebrain$(SUBTARGET_SUFFIX)			\
  cecem$(SUBTARGET_SUFFIX) \
  cemarshal50$(SUBTARGET_SUFFIX) \
  cepepman$(SUBTARGET_SUFFIX) \
  cetransport$(SUBTARGET_SUFFIX)

DFLAGS +=

# Define the test target
TESTTARGET   = cesectest

TESTSRC      = test/test_cesec.cpp

TESTLIBPATH += \
 $(NLPLATFORMSDIR)/common/brain/$(BINDIR)         \
 $(NLENFORCERSDIR)/sdk/client/stub/CEM/$(BINDIR)  \
 $(NLENFORCERSDIR)/framework/marshal/$(BINDIR)    \
 $(NLENFORCERSDIR)/framework/transport/$(BINDIR)    \
 $(NLENFORCERSDIR)/sdk/client/stub/CONN/$(BINDIR)  \
 $(NLENFORCERSDIR)/sdk/client/stub/PEPMan/$(BINDIR) \
 $(NLENFORCERSDIR)/sdk/client/stub/SEC/$(BINDIR) 

TESTLIBS    += \
  cebrain$(SUBTARGET_SUFFIX)                                       \
  cecem$(SUBTARGET_SUFFIX)                                       \
  ceconn$(SUBTARGET_SUFFIX) \
  cesec$(SUBTARGET_SUFFIX)
