
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = BasePEPPlugin

SRC = \
  src/ApplicationScope.cpp \
  src/HookServicePlugin.cpp 

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/Common \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.11/inc 

LIBPATH += \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.11/lib/free

ifeq ($(TARGETENVARCH), x86)
LIBS +=         \
  madCHook32 
else
LIBS +=         \
  madCHook64  
endif


LIBS += \
  Shlwapi
