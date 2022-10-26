
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER	= VS2008
PLATFORMSDK = 7.0


TARGET	= NLPortableEncryption

SRC = \
	src/NLPortableEncryption.cpp

INCPATH += \
	./include \
	$(NLBUILDROOT)/prod/common/include \
	$(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
	$(NLEXTERNALDIR)/boost/boost_1_45_0

ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		c:/winddk/7600.16385.1/lib/wnet/i386
else
	LIBPATH += \
		c:/winddk/7600.16385.1/lib/wnet/amd64
endif	

LIBPATH += \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/celog2/$(BINDIR)

LIBS += \
	Shlwapi fltlib cebrain_static$(SUBTARGET_SUFFIX) CELog2$(SUBTARGET_SUFFIX) nl_sysenc_lib_static$(SUBTARGET_SUFFIX)