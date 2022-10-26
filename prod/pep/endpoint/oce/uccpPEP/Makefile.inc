
VERSION_PRODUCT		= $(VERSION_PRODUCT_OCE)
VERSION_MAJOR		= $(VERSION_MAJOR_OCE)
VERSION_MINOR		= $(VERSION_MINOR_OCE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OCE)
VERSION_PATCH		= $(VERSION_PATCH_OCE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = uccpPEP

SRC = \
  src/stdafx.cpp \
  src/Blocks.cpp \
  src/Helper.cpp \
  src/Platform.cpp \
  src/uccpPEP.cpp \
  src/LiveSessionWnd.cpp \
  src/EvalCache.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(BUILDTYPE)_win_$(TARGETENVARCH)



ifeq ($(BUILDTYPE), debug)
LIBPATH += \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/checked
else
LIBPATH += \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free
endif


LIBS += Ws2_32 mpr psapi version comctl32 netapi32 secur32
