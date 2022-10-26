
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
TARGET = nlcontextmgr

SRC = \
  src/CacheMgr.cpp \
  src/CriticalSections.cpp \
  src/nlstorage_service.cpp \
  src/Services.cpp \
  src/stdafx.cpp \
  src/WindowsService.cpp \

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/common/nlcommonlib/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

ifeq ($(TARGETENVARCH),x86)
	LIBPATH += $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib
else
	LIBPATH += $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib
endif

LIBS += 
