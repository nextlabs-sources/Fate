
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = iePEP


SRC = \
	src/iePEP.cpp \
	src/IEObj.cpp \
	src/log.cpp \
	src/hook.cpp \
	src/event_handler.cpp \
	src/iePEP_i.c \
	src/NL_base64.cpp \
	src/stdafx.cpp \
	src/ActionHandler.cpp \
	src/utils.cpp

RCSRC =	src/iePEP.rc 

# .idl file must be placed in SRC directory.
IDL_SRC = src/iePEP.idl

ifeq ($(TARGETENVARCH), x64)
	ASMOBJS = asm/OnSubmit.obj
endif

INCPATH += \
  ./include \
  ./idlgen \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/inc \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include \
  $(NLBUILDROOT)/prod/common/celog2/include \

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/lib/free \
  $(NLBUILDROOT)/prod/common/celog2/$(BINDIR)

ifeq ($(TARGETENVARCH), x86)
	LIBS += madCHook32 \
			nlcc_ulib_static$(SUBTARGET_SUFFIX) \
			cebrain_static$(SUBTARGET_SUFFIX) \
			CELog2$(SUBTARGET_SUFFIX) \
			Wininet \
			Gdi32 \
			iepmapi
else
	LIBS += madCHook64 \
			nlcc_ulib_static$(SUBTARGET_SUFFIX) \
			cebrain_static$(SUBTARGET_SUFFIX) \
			CELog2$(SUBTARGET_SUFFIX) \
			Wininet \
			Gdi32 \
			iepmapi
endif

ifeq ($(TARGETENVARCH), x64)
	DFLAGS += -D_M_X64 -D_ATL_STATIC_REGISTRY
else
	DFLAGS += -D_M_IX86 -D_ATL_STATIC_REGISTRY
endif
