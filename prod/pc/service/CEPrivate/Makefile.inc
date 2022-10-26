
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = cepdpprivate

SRC = src/PDPPrivate.cpp 

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/pc/service/CONN/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(JDKDIR)/include \
  $(JDKDIR)/include/win32

LIBPATH += \
  $(NLENFORCERSDIR)/sdk/client/stub/SingleDLL/$(BINDIR)	\
  $(BUILDOUTPUTDIR)/$(LIBDIR)  \
  $(NLENFORCERSDIR)/pdpd/service/CONN/$(BINDIR)	\
  $(JDKDIR)/lib 	

LIBS += \
  cesdk$(SUBTARGET_SUFFIX)			\
  cepdpconn$(SUBTARGET_SUFFIX)			\
  jvm

