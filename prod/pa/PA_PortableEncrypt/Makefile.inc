VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

SUBDIRS = 

COMPILER = VS2008
PLATFORMSDK=7.0


SUBTARGET        = pa_pe
SUBTARGET_STATIC = yes
MANIFEST = $(BINDIR)/pa_pe$(SUBTARGET_SUFFIX).dll.manifest

SRC = \
      src/dllmain.cpp \
      src/stdafx.cpp \
      src/PA_PortableEncrypt.cpp 
      
DYNAMICCRT=yes

INCPATH = \
  include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/include \
  $(NLBUILDROOT)/prod/pa/paf/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include 

ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/resattrmgr/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/resattrlib/$(BINDIR)
else
  LIBPATH += \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/resattrmgr/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/resattrlib/$(BINDIR)
endif
  
LIBS += nl_sysenc_lib$(SUBTARGET_SUFFIX) \
		resattrmgr$(SUBTARGET_SUFFIX) \
		resattrlib$(SUBTARGET_SUFFIX)

DFLAGS += -DPORTABLEENCRYPTPA_EXPORTS
