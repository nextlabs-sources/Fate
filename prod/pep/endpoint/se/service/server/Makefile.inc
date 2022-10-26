
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

SUBDIRS = 

COMPILER = VS2008
PLATFORMSDK=7.0

SUBTARGET = pcs_server

WINDDK = C:/WinDDK/7600.16385.1

SRC = \
  src/se_server.cpp \
  src/se_server_implementation.cpp \
  src/drmctrl.cpp \
  src/se_dispatch.cpp

INCPATH = \
  $(WINSDK_ROOT)/include \
  $(MSVC_ROOT)/VC/include \
  $(WINDDK)/inc/api \
  $(WINDDK)/inc/ddk \
  ./include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_SysEncryption/common/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/NextLabsTaggingLib/include \
  ../include \
  ../src \
  $(NLBUILDROOT)/prod/pep/endpoint/se/pcs_rpc/include	\
  $(NLBUILDROOT)/prod/pc/cesdk/include	\
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include/win32

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

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

LIBS += \
  cebrain_static$(SUBTARGET_SUFFIX) \
  fltlib \
  Shlwapi
