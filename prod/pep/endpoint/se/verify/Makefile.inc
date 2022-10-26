
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER = VS2008
PLATFORMSDK=7.0

TARGET = fs_verify

BOOST =  $(NLEXTERNALDIR)/boost/boost_1_45_0
WINDDK = C:/WinDDK/7600.16385.1

SRC = \
      src/verify.cpp

INCPATH += \
  include \
  $(NLENFORCERSDIR)/platforms/common/include \
  $(NLENFORCERSDIR)/platforms/win32/modules/eframework/eframework.R2.1 \
  ../nl_SysEncryption/user/include \
  ../nl_sysenc_lib/include \
  ../include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH = \
  $(MSVC_ROOT)/VC/lib \
  $(BINDIR) \
  ../../../../build/main/dependencies/main/lib   \
  ../../../../build/main/installer/lib    \
  $(WINDDK)/lib/wnet/i386 \
  $(WINDDK)/lib/crt/i386 \
  ../nl_sysenc_lib/$(BINDIR) \
  $(BOOST)/bin.v2/libs/filesystem/build/msvc-9.0/$(BUILDTYPE)/link-static/runtime-link-static/threading-multi \
  $(BOOST)/bin.v2/libs/system/build/msvc-9.0/$(BUILDTYPE)/link-static/runtime-link-static/threading-multi \
  $(BOOST)/bin.v2/libs/regex/build/msvc-9.0/$(BUILDTYPE)/link-static/runtime-link-static/threading-multi

LIBS +=

LOCAL_DEFINES += \
  -DVERSION_MAJOR=$(VERSION_MAJOR) \
  -DVERSION_MINOR=$(VERSION_MINOR)

CFLAGS   += $(LOCAL_DEFINES)
CXXFLAGS += $(LOCAL_DEFINES)
