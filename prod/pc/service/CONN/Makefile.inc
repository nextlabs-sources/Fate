
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = cepdpconn

SRC = src/PDPConn.cpp 

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include            \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include   \
  $(NLBUILDROOT)/prod/common/celog/include           \
  $(JDKDIR)/include \
  $(JDKDIR)/include/win32

LIBPATH += \
  $(NLENFORCERSDIR)/sdk/client/stub/SingleDLL/include            \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)             \
  $(BUILDOUTPUTDIR)/$(LIBDIR)  \

LIBS += \
  cebrain$(SUBTARGET_SUFFIX) \
  cesdk$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX)

DFLAGS +=

