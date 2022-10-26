
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = ceeval
SUBTARGET_STATIC=yes

SRC = src/eval.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/celog/include         \
  $(NLBUILDROOT)/prod/common/brain/include     \
  $(NLBUILDROOT)/prod/pc/marshal/include  \
  $(NLBUILDROOT)/prod/pc/transport/include  \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/PEPMan/include 

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += \
  cebrain$(SUBTARGET_SUFFIX)			\
  cecem$(SUBTARGET_SUFFIX) \
  cemarshal50$(SUBTARGET_SUFFIX) \
  cetransport$(SUBTARGET_SUFFIX) \
  cepepman$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX)

DFLAGS +=

# Define the test target
TESTTARGET   = ceevaltest

TESTSRC      = test/eval_test.cpp

TESTLIBPATH += \
 $(NLPLATFORMSDIR)/common/brain/$(BINDIR)         \
 $(NLENFORCERSDIR)/sdk/client/stub/CEM/$(BINDIR)  \
 $(NLENFORCERSDIR)/framework/marshal/$(BINDIR)    \
 $(NLENFORCERSDIR)/framework/transport/$(BINDIR)    \
 $(NLENFORCERSDIR)/sdk/client/stub/PEPMan/$(BINDIR) \
 $(NLENFORCERSDIR)/sdk/client/stub/CONN/$(BINDIR)  \
 $(NLENFORCERSDIR)/sdk/client/stub/EVAL/$(BINDIR)  

TESTLIBS    += \
  cebrain$(SUBTARGET_SUFFIX)     \
  cemarshal50$(SUBTARGET_SUFFIX)    \
  cecem$(SUBTARGET_SUFFIX)       \
  cetransport$(SUBTARGET_SUFFIX)    \
  cepepman$(SUBTARGET_SUFFIX)     \
  ceconn$(SUBTARGET_SUFFIX)      \
  ceeval$(SUBTARGET_SUFFIX)
