
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = ceprivate

SRC = src/CEPrivate.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/pc/marshal/include  \
  $(NLBUILDROOT)/prod/pc/transport/include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/PEPMan/include 

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR)

LIBS += \
  cesdk$(SUBTARGET_SUFFIX) cebrain$(SUBTARGET_SUFFIX)

# Define the test target
TESTTARGET   = ceprivate_test

TESTSRC      = test/test_private.cpp

TESTLIBPATH += \
 $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR) \
 $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEPrivate/$(BINDIR)  

TESTLIBS    += \
  ceprivate$(SUBTARGET_SUFFIX) \
  cesdk$(SUBTARGET_SUFFIX)
