
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

# Show library link order
LDFLAGS += /verbose:lib 


SUBTARGET = NLOfficePEP

WINDDK = c:/Winddk/7600.16385.1

SRC = \
	src/stdafx.cpp \
	src/utils.cpp \
	src/NLAction.cpp \
	src/NLObMgr.cpp \
	src/officelistener.cpp \
	src/obligations.cpp \
	src/officepep.cpp \
	src/NLOfficeSink.cpp \
	src/NLHookAPI.cpp \
	src/NLSecondaryThreadForPDFMaker.cpp \
	src/NLProcess.cpp \
	src/NLOfficePEP_Comm.cpp \
	src/NLInsertAndDataAction.cpp \
	src/NLOfficePEP_i.c	\
	src/dllmain.cpp \
	src/TalkWithSCE.cpp \
	src/ScreenCaptureAuxiliary.cpp \
	src/contentstorage.cpp
	
RCSRC =	src/NLOfficePEP.rc 

# .idl file must be placed in SRC directory.
IDL_SRC = src/NLOfficePEP.idl

INCPATH += \
	./include \
	./idlgen \
	$(NLEXTERNALDIR)/boost/boost_1_43_0 \
	$(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
	$(NLBUILDROOT)/prod/common/nlcc/ulib \
	$(NLBUILDROOT)/prod/common/nlcc \
	$(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
	$(NLBUILDROOT)/prod/common/include \
	$(NLBUILDROOT)/prod/common/resattrmgr/include \
	$(NLBUILDROOT)/prod/common/resattrlib/include \
	$(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
	$(NLBUILDROOT)/prod/pa/paf/include \
	$(NLBUILDROOT)/prod/common/lib/mch/3.1.13/inc \
	$(NLBUILDROOT)/prod/common/nlcommonlib/include \
	$(NLBUILDROOT)/prod/common \
	$(NLBUILDROOT)/prod/common/include \
	$(NLBUILDROOT)/prod/common/celog2/include \
	$(NLBUILDROOT)/prod/pep/endpoint/wde/Common \
	$(NLBUILDROOT)/prod/common/feature_manager \
	$(NLBUILDROOT)/prod/pa/NLClassificationOb/include \
	$(NLBUILDROOT)/prod/pa/NLClassificationOb/Common \
	$(NLBUILDROOT)/prod/Common/ResErrHandling/include \
	$(NLBUILDROOT)/prod/pc/cesdk/include \
	$(NLBUILDROOT)/prod/pep/endpoint/rmc/apps/nxrmcsdk/include \
	$(NLBUILDROOT)/prod/pep/endpoint/wde/NLVisualLabelingPA/include
	

LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR) \
    $(NLBUILDROOT)/prod/common/celog2/$(BINDIR)  
ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		$(WINDDK)/lib/wnet/i386 \
		$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib \
		$(MSVC_ROOT)/VC/atlmfc/lib
else
	LIBPATH += \
		$(WINDDK)/lib/wnet/amd64 \
		$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib \
		$(MSVC_ROOT)/VC/atlmfc/lib/amd64
endif

#
#	make to use checked version of MadCHook for compiling debug version
#
ifeq ($(BUILDTYPE), release)
	LIBPATH += $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/lib/free 
else
	LIBPATH += $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/lib/checked
endif

# Add LIBCMT.dll here to force it to be linked before Kernel32.dll.
# http://support.microsoft.com/kb/148652
# http://connect.microsoft.com/VisualStudio/feedback/details/101729/lnk4006-local-unwind-multiply-defined-in-x64-build
LIBS += \
	comctl32 \
	Winhttp \
	Wininet \
	GDIPlus \
	nlcc_ulib_static$(SUBTARGET_SUFFIX) \
	celog2$(SUBTARGET_SUFFIX) \
	fltlib \
	version 

# for compileing x64-debug and using LIBCMT,  a lib conflict error occurs, but x86 is ok
# 
ifeq ($(BUILDTYPE), release)
	LIBS += LIBCMT
else
	LIBS += LIBCMTD
endif
  
ifeq ($(TARGETENVARCH), x86)
	LIBS += madCHook32
else
	LIBS += madCHook64
endif


#
# define and set precompile header to boost compiling speed significantly
#

######CFLAGS += /Yc"stdafx.h" /Yu"stdafx.h" /Fp"officepep.pch" 
######CXXFLAGS += /Yc"stdafx.h"  /Yu"stdafx.h" /Fp"officepep.pch"
#ifeq ($(BUILDTYPE), release)
#	ifeq ($(TARGETENVARCH), x86)
#		CXXFLAGS += /Yc"stdafx.h"  /Yu"stdafx.h" /Fp"$(BIN_DIR)/$(SUBTARGET)_$(BUILDTYPE)_$(TARGETENVARCH).pch"
#	else
#		CXXFLAGS += /Yc"stdafx.h"  /Yu"stdafx.h" /Fp"officepep_release64.pch"
#	endif
#else
#	ifeq ($(TARGETENVARCH), x86)
#		CXXFLAGS += /Yc"stdafx.h"  /Yu"stdafx.h" /Fp"officepep_debug86.pch"
#	else
#		CXXFLAGS += /Yc"stdafx.h"  /Yu"stdafx.h" /Fp"officepep_debug64.pch"
#	endif
#endif


#
# suspicious on build machine, but PreCompiled works fine at local developer machine
#

# use conformant name for precomiled header file
#CXXFLAGS += /Yc"stdafx.h"  /Yu"stdafx.h" /Fp"$(SUBTARGET)_$(BUILDTYPE)_$(TARGETENVARCH).pch"
# add the precompiled header file  into CLEANFILES
#CLEANFILES += $(SUBTARGET)_$(BUILDTYPE)_$(TARGETENVARCH).pch $(SUBTARGET)_$(BUILDTYPE)_$(TARGETENVARCH).pchast


