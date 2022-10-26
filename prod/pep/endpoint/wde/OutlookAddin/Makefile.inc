
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = OutlookAddin

SRC = \
	src/dllmain.cpp \
	src/Hook.cpp \
	src/OutlookAddin.cpp \
	src/outlookImpl.cpp \
	src/OutlookAddin_i.c \
	src/stdafx.cpp

RCSRC =	src/OutlookAddin.rc 

# .idl file must be placed in SRC directory.
IDL_SRC = src/OutlookAddin.idl

INCPATH += \
  ./include \
  ./idlgen \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLBUILDROOT)/prod/pep/endpoint/se/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 


LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR)  \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR)
  
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


LIBS += fltlib  cebrain_static$(SUBTARGET_SUFFIX) CELog2$(SUBTARGET_SUFFIX) nl_sysenc_lib_static$(SUBTARGET_SUFFIX)


ifeq ($(TARGETENVARCH), x64)
	DFLAGS += -D_M_X64 -D_ATL_STATIC_REGISTRY
else
	DFLAGS += -D_M_IX86 -D_ATL_STATIC_REGISTRY
endif
