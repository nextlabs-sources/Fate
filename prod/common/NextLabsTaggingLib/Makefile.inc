
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

SUBTARGET        = NextLabsTaggingLib
SUBTARGET_STATIC = yes

SRC = \
      src/NextLabsTagMap.cpp \
      src/NextLabsTaggingLib.cpp

MANIFEST = $(BINDIR)/NextLabsTaggingLib$(SUBTARGET_SUFFIX).dll.manifest
DYNAMICCRT=yes

INCPATH = \
  include \
  $(NLBUILDROOT)/prod/common/celog/include         \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/pep/endpoint/se/include         \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include         \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

# Define the custom library path
LIBPATH = \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR)

ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
  c:/winddk/7600.16385.1/lib/wnet/i386
else
  LIBPATH += \
  c:/winddk/7600.16385.1/lib/wnet/amd64
endif

LIBS += \
    nl_sysenc_lib$(SUBTARGET_SUFFIX) \
    fltlib \
	Shlwapi
