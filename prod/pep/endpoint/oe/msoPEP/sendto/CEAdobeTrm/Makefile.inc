VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 

SUBTARGET = CE_AdobePEPTrm

SRC = \
  src/dllmain.cpp \
  src/ApiHook.cpp \

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/pa/paf/include
     
LIBPATH += \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  
LIBS += Shlwapi GDIPlus\
  
