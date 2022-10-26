
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER = VS2008
PLATFORMSDK=7.0

ifeq ($(SE_LD), )
  SE_LD = CONSOLE
endif

ifeq ($(SE_LD),CONSOLE)
	TARGET = nlSysEncryption
else
	TARGET = nlSysEncryptionObligation
        LDFLAGS  += -ENTRY:wmainCRTStartup -SUBSYSTEM:WINDOWS
endif

WINDDK = C:/WinDDK/7600.16385.1

SRC = \
      src/nlsysenc.cpp \
      src/nlsysenc_efilter.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_SysEncryption/user/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

LIBPATH = \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR) \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

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
    CELog2$(SUBTARGET_SUFFIX) \
    nl_sysenc_lib_static$(SUBTARGET_SUFFIX) \
    fltlib	\
	Shlwapi

LOCAL_DEFINES += \
  -DVERSION_MAJOR=$(VERSION_MAJOR) \
  -DVERSION_MINOR=$(VERSION_MINOR)

CFLAGS   += $(LOCAL_DEFINES)
CXXFLAGS += $(LOCAL_DEFINES)
