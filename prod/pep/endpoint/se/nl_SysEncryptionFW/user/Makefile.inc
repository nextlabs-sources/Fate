
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

SUBDIRS = 

WINDDK = C:/WinDDK/7600.16385.1

COMPILER = VS2008
PLATFORMSDK=7.0

SUBTARGET = nlsefw_plugin

SRC = src/NLSEPlugin.cpp		\
      src/NLSESDKWrapper.cpp	\
      src/NLSELib.cpp

INCPATH =																\
  $(WINDDK)/inc/ddk														\
  include																\
  ../common/include 													\
  ../../include 														\
  $(NLBUILDROOT)/prod/pc/cesdk/include									\
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include				\
  $(NLBUILDROOT)/prod/common/include									\
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/common/celog2/include								\
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0				\
  $(NLBUILDROOT)/prod/pc/key_management/KeyManagementConsumer/include	\
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(NLBUILDROOT)/prod/common/celog2/$(BINDIR)

# MSVC libs must be placed in front of DDK libs to avoid link error.
ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib \
		$(WINDDK)/lib/wnet/i386
else
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib/amd64 \
		$(WINDDK)/lib/wnet/amd64
endif

LIBS += ole32 psapi libcpmt fltlib CELog2$(SUBTARGET_SUFFIX)


# temporary workarounnd for build issue.  Can remove once link to Enforcer directory is removed
BUILDOUTPUTDIR=../../../../build/main/build_native
