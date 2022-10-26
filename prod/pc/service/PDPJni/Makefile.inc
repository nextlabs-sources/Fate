
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = pdpjni

SRC = src/PDPJni.cpp 

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/jinclude         \
  $(NLBUILDROOT)/prod/common/brain/include     \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pc/marshal/include \
  $(NLBUILDROOT)/prod/pc/kif/include \
  $(NLBUILDROOT)/prod/pc/transport/include \
  $(NLBUILDROOT)/prod/pc/service/generic/include \
  $(NLBUILDROOT)/prod/pc/service/PDPJni/generated_code \
  $(JDKDIR)/include \
  $(JDKDIR)/include/win32

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR) \
  $(NLBUILDROOT)/prod/pc/kif/$(BINDIR) \
  $(NLBUILDROOT)/prod/pc/service/generic/$(BINDIR) \
  $(BUILDOUTPUTDIR)/$(LIBDIR)  \
  $(JDKDIR)/lib 	

LIBS += \
  jvm \
  cebrain$(SUBTARGET_SUFFIX) \
  cesdk$(SUBTARGET_SUFFIX) \
  cekif$(SUBTARGET_SUFFIX) \
  cepdpgeneric$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX)

#ifneq ($(OS), Linux)
#LIBS+= \
#  cekif \
#  cepdpgeneric 
#endif
