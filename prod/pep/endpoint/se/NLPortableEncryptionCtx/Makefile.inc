
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER	= VS2008
PLATFORMSDK = 7.0
#DYNAMICCRT	= yes

# IMPORTANT: Temporary override to handle this code not UNICODE ready. Corbin
# wrote this code that does not support UNICODE and we have no time to change it.
# Should change this project to use UNICODE.
#CFLAGS2 = -U_UNICODE -UUNICODE
#RCFLAGS2 = -U_UNICODE -UUNICODE

SUBTARGET	= NLPortableEncryptionCtx

SRC = \
	src/FileProcess.cpp \
	src/NLPortableEncryptionCtx.cpp \
	src/ShellCtxMenu.cpp \
	src/ShellExt.cpp \
	src/ShellExtReg.cpp \
	src/ShUtils.cpp


RCSRC = src/NLPortableEncryptionCtx.rc

INCPATH += \
	./include \
	../nl_sysenc_lib/include \
	$(NLBUILDROOT)/prod/common/include \
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
	CELog2$(SUBTARGET_SUFFIX) \
	nl_sysenc_lib_static$(SUBTARGET_SUFFIX) \
	cebrain_static$(SUBTARGET_SUFFIX) \
	fltlib
