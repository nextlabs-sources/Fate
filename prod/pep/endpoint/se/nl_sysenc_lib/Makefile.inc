
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

SUBDIRS = 

WINDDK = C:/WinDDK/7600.16385.1

COMPILER = VS2008
PLATFORMSDK=7.0

SUBTARGET        = nl_sysenc_lib
SUBTARGET_STATIC = yes

SRC = \
      src/NLSESDKWrapper.cpp \
      src/NLSELib.cpp \
      src/NLSELibFW.cpp \
      src/se_client.cpp \
      src/AltDataStream.cpp

INCPATH = \
  $(WINSDK_ROOT)/include \
  $(MSVC_ROOT)/VC/include \
  $(MSVC_ROOT)/VC/atlmfc/Include \
  $(WINDDK)/inc/api \
  $(WINDDK)/inc/ddk \
  include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/include \
  $(NLBUILDROOT)/prod/common/NextLabsTaggingLib/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_SysEncryption/common/include	\
  $(NLBUILDROOT)/prod/pep/endpoint/se/pcs_rpc/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include	     \
  $(NLBUILDROOT)/prod/pc/key_management/KeyManagementConsumer/include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include         \
  $(NLBUILDROOT)/prod/common/celog2/include         \
  $(NLBUILDROOT)/prod/common/brain/include         \
  $(NLBUILDROOT)/prod/common/include         \
	$(NLBUILDROOT)/prod/common/feature_manager         \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR) \
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

LIBS += fltlib shlwapi cebrain_static$(SUBTARGET_SUFFIX) \
	CELog2$(SUBTARGET_SUFFIX)

LOCAL_DEFINES += \
  -DVERSION_MAJOR=$(VERSION_MAJOR) \
  -DVERSION_MINOR=$(VERSION_MINOR) \
  -DVERSION_MAINTENANCE=$(VERSION_MAINTENANCE) \
  -DVERSION_PATCH=$(VERSION_PATCH)

CFLAGS   += $(LOCAL_DEFINES)
CXXFLAGS += $(LOCAL_DEFINES)
