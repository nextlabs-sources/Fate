
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

SUBDIRS = 

COMPILER = VS2008
PLATFORMSDK=7.0

TARGET              = pcs_rpc

SRC = \
	pcs_rpc.cpp \
	pcs_idl_lex.cpp \
	pcs_idl_grammar_tab.cpp

INCPATH = \
	./include \
	./src \
	$(MSVC_ROOT)/VC/include \
	$(WINSDK_ROOT)/include \
	$(NLBUILDROOT)/prod/pc/cesdk/include \
	$(NLEXTERNALDIR)/boost/boost_1_42_0

ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib \
		$(WINSDK_ROOT)/Lib
else
	LIBPATH += \
		$(MSVC_ROOT)/VC/lib/amd64 \
		$(WINSDK_ROOT)/Lib\x64
endif

# Need to turn off PREfast here, because the pcs_idl_grammar_tab.cpp
# generated by bison contains unreachable code which PREfast detects
# as erros.
override PREFAST=0
