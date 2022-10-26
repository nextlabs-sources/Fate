
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

SUBDIRS = 

WINDDK = C:/WinDDK/7600.16385.1

COMPILER = VS2008
PLATFORMSDK=7.0

SUBTARGET = nlse_plugin

SRC = src/NLSEPlugin.cpp \
      src/NLSESDKWrapper.cpp \
      src/NLSELib.cpp

INCPATH = \
  $(WINSDK_ROOT)/include \
  $(MSVC_ROOT)/VC/include \
  $(MSVC_ROOT)/VC/atlmfc/Include \
  $(WINDDK)/inc/api		       \
  $(WINDDK)/inc/ddk		       \
  include \
  ../common/include \
  ../../include \
  $(NLBUILDROOT)/prod/pc/cesdk/include		     \
  $(NLBUILDROOT)/prod/pc/key_management/KeyManagementConsumer/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include         \
  $(NLBUILDROOT)/prod/common/celog2/include \
  $(NLBUILDROOT)/prod/common/NextLabsTaggingLib/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(NLBUILDROOT)/prod/common/celog2/$(BINDIR)

# MSVC libs must be placed in front of DDK libs to avoid link error.
ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib \
  c:/winddk/7600.16385.1/lib/wnet/i386 \
  c:/winddk/7600.16385.1/lib/crt/i386
else
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib/amd64 \
  c:/winddk/7600.16385.1/lib/wnet/amd64	      \
  c:/winddk/7600.16385.1/lib/crt/amd64 
endif

LIBS += ole32 psapi libcpmt fltlib CELog2$(SUBTARGET_SUFFIX)
