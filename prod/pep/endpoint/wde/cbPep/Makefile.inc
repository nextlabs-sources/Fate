
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = cbpep

WINDDK = c:/Winddk/7600.16385.1

SRC = \
	  src/cbPep_i.c \
      src/cbPep.cpp \
      src/dllmain.cpp \
      src/FileOperationHooking.cpp \
      src/Hooking.cpp \
      src/NxtMgr.cpp \
      src/NxtReceiver.cpp \
      src/NxtShellExtension.cpp \
      src/stdafx.cpp 
      
      

RCSRC =	src/cbPep.rc 

# .idl file must be placed in SRC directory.
IDL_SRC = src/cbPep.idl

INCPATH += \
  ./ \
  ./include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 \
  $(NLBUILDROOT)/prod/common/nlcommonlib/include	\
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0/eframework/policy \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0/eframework/platform \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/common/celog/include 

ifeq ($(TARGETENVARCH), x86)
LIBPATH += \
  $(MSVC_ROOT)/VC/atlmfc/lib \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib
else
LIBPATH += \
	$(MSVC_ROOT)/VC/atlmfc/lib/amd64 \
	$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib
endif

LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR) \
	$(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free \
	$(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR) \
	

# We need to specify the full file path of fltlib in LIBS below.
# Otherwise, if we specify $(WINDDK)/lib/wnet/xxx in LIBPATH above,
# the linker complains about unresolved external symbols for
# _IID_IFileOperation, _CLSID_FileOperation and _IID_IShellItemArray
# for some reason.
ifeq ($(TARGETENVARCH), x86)
LIBS +=         \
  madCHook32 \
  $(WINDDK)/lib/wnet/i386/fltlib 
else
LIBS +=         \
  madCHook64 \
  $(WINDDK)/lib/wnet/amd64/fltlib
endif

LIBS += \
  mpr \
  Shlwapi \
  nl_sysenc_lib_static$(SUBTARGET_SUFFIX) \
  cebrain_static$(SUBTARGET_SUFFIX) \
  CELog2$(SUBTARGET_SUFFIX) \
  nlcc_ulib_static$(SUBTARGET_SUFFIX)
  
DFLAGS += -DENHANCEMENT_EXPORTS 