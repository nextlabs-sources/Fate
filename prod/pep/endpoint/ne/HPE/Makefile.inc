
VERSION_PRODUCT		= $(VERSION_PRODUCT_NE) 
VERSION_MAJOR		= $(VERSION_MAJOR_NE)
VERSION_MINOR		= $(VERSION_MINOR_NE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NE)
VERSION_PATCH		= $(VERSION_PATCH_NE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = hpe

SRC = \
  src/APIHook.cpp \
  src/configure.cpp \
  src/dllmain.cpp \
  src/Eval.cpp \
  src/HPE.cpp \
  src/log.cpp \
  src/MapperMgr.cpp \
  src/SPIInstaller.cpp \
  src/stdafx.cpp \
  src/Utilities.cpp \
  src/criticalMngr.cpp \
  src/ModulVer.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(NLBUILDROOT)/prod/common/nlcc/ulib/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free

LIBS += ole32 oleaut32 \
  nlcc_ulib_static$(SUBTARGET_SUFFIX)

CFLAGS += \
  -DPEP_USE_LEGACY_IGNORE_CHECK

CXXFLAGS += \
  -DPEP_USE_LEGACY_IGNORE_CHECK