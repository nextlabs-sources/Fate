
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER	= VS2008
PLATFORMSDK = 7.0

SUBDIRS		= 
SUBTARGET	= IconBadging

SRC = \
	src/dllmain.cpp \
	src/IconBadging.cpp \
	src/SeIconOverlay.cpp \
	src/stdafx.cpp \
	src/IconBadging_i.c

RCSRC = src/IconBadging.rc

# .idl file must be placed in SRC directory.
IDL_SRC = src/IconBadging.idl

INCPATH += \
	./include \
	./idlgen \
	$(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
    $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR)

# MSVC libs must be placed in front of DDK libs to avoid link error.
ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib \
		c:/winddk/7600.16385.1/lib/wnet/i386
else
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib/amd64 \
		c:/winddk/7600.16385.1/lib/wnet/amd64
endif

LIBS += \
    fltlib \
    cebrain_static$(SUBTARGET_SUFFIX) \
    CELog2$(SUBTARGET_SUFFIX) \
    nl_sysenc_lib_static$(SUBTARGET_SUFFIX)	\
	Shlwapi

# Need this to turn on generated code in src/IconBadging_p.c
ifeq ($(TARGETENVARCH), x64)
	DFLAGS += -D_M_AMD64
endif
