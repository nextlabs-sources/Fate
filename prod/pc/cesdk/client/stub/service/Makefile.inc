
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = ceservice
SUBTARGET_STATIC=yes

SRC = src/service.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include     \
  $(NLBUILDROOT)/prod/common/celog/include     \
  $(NLBUILDROOT)/prod/pc/marshal/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/PEPMan/include 

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += \
  cebrain$(SUBTARGET_SUFFIX)			\
  cecem$(SUBTARGET_SUFFIX) \
  cemarshal50$(SUBTARGET_SUFFIX) \
  cepepman$(SUBTARGET_SUFFIX) \
  cetransport$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX)
