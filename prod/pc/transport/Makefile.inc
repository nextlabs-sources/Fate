
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

SUBTARGET = cetransport
SUBTARGET_STATIC=yes

SRC = src/transport.cpp           

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include

LIBPATH +=  \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR)  \
  $(NLBUILDROOT)/bin/$(BINDIR)    

LIBS += \
  cebrain$(SUBTARGET_SUFFIX)
