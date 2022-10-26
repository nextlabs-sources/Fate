
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

SUBTARGET = nl_tamper_plugin

SRC = src/nl_tamper_plugin.cpp \
      src/nl_tamper_lib.cpp

DFLAGS += -D PSAPI_VERSION=1

INCPATH += \
  $(MSVC_ROOT)/VC/include \
  c:/winddk/7600.16385.1/inc/api \
  c:/winddk/7600.16385.1/inc/ddk \
  $(NLBUILDROOT)/prod/pc/tamper/fs/user/include	\
  $(NLBUILDROOT)/prod/pc/tamper/fs/inc \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pc/tamper/TamperproofConfig/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/celog/include
  
LIBPATH += \
  $(NLBUILDROOT)/common/brain/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR) \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR) \
  $(NLBUILDROOT)/prod/pc/tamper/TamperproofConfig/$(BINDIR) 

# MSVC libs must be placed in front of DDK libs to avoid link error.
ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib \
  c:/winddk/7600.16385.1/lib/wnet/i386 \
  c:/winddk/7600.16385.1/lib/crt/i386
else
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib/amd64 \
  c:/winddk/7600.16385.1/lib/wnet/amd64 \
  c:/winddk/7600.16385.1/lib/crt/amd64
endif

LIBS += ole32 psapi cesdk$(SUBTARGET_SUFFIX) libcpmt fltlib libcpmtd \
    celog$(SUBTARGET_SUFFIX) \
    nlTamperproofConfig_static$(SUBTARGET_SUFFIX)
