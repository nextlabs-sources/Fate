
VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = adaptermanager

SRC = \
      src/adaptermanager.cpp \
      src/stdafx.cpp

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/pep/endpoint/oe/msoPEP/adaptercomm/include \
  $(NLBUILDROOT)/prod/common/celog/include
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)
  
LIBS += ole32 oleaut32 

DFLAGS += -DADAPTERMANAGER_EXPORTS
