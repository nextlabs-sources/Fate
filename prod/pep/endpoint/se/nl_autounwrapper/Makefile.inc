
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER	= VS2008
PLATFORMSDK = 7.0


TARGET	= nl_autounwrapper
LDFLAGS += -ENTRY:wWinMainCRTStartup -SUBSYSTEM:WINDOWS
MANIFEST    = $(BINDIR)/nl_autounwrapper.exe.manifest

SRC = \
	src/nl_autounwrapper.cpp \
	src/nl_autounwrapperdlg.cpp \
	src/stdafx.cpp

RCSRC = src/nl_autounwrapper.rc

INCPATH += \
	./include \
	../include \
	../../../../common/feature_manager \
	../nl_sysenc_lib/include \
	../../../../common/include \
	$(NLBUILDROOT)/prod/common/resattrlib/include	\
	$(NLBUILDROOT)/prod/common/resattrmgr/include	\
	$(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0	\
	$(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
    $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR)

# MSVC libs must be placed in front of DDK libs to avoid link error.
ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib \
		$(MSVC_ROOT)/VC/atlmfc/lib \
		c:/winddk/7600.16385.1/lib/wnet/i386
else
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib/amd64 \
		$(MSVC_ROOT)/VC/atlmfc/lib/amd64 \
		c:/winddk/7600.16385.1/lib/wnet/amd64
endif

LIBS += \
    fltlib \
    cebrain_static$(SUBTARGET_SUFFIX) \
    CELog2$(SUBTARGET_SUFFIX) \
    nl_sysenc_lib_static$(SUBTARGET_SUFFIX) \
	Shlwapi

# 
ifeq ($(TARGETENVARCH), x64)
	DFLAGS += -D_M_AMD64
endif
