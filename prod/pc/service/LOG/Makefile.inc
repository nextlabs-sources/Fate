
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = cepdplog

SRC = src/PDPLog.cpp 

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/common/celog/include         \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(JDKDIR)/include \
  $(JDKDIR)/include/win32

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR)      \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)      \
  $(BUILDOUTPUTDIR)/$(LIBDIR)  \
  $(JDKDIR)/lib 	

LIBS += \
  cesdk$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX) \
  jvm

DFLAGS +=

# Define the test target
TESTTARGET   = 

TESTSRC      = 

TESTLIBPATH += 

TESTLIBS    += 
