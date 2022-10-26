VERSION_PRODUCT  = $(VERSION_PRODUCT_NLE)
VERSION_MAJOR  = $(VERSION_MAJOR_NLE)
VERSION_MINOR  = $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH  = $(VERSION_PATCH_NLE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = NXLFilter
DYNAMICCRT = yes
MANIFEST = $(BINDIR)/NXLFilter$(SUBTARGET_SUFFIX).dll.manifest


SRC = \
	src/Dll.cpp \
	src/FilterBase.cpp \
	src/NXLFilter.cpp

INCPATH += \
  ./inc \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 \
  $(NLBUILDROOT)/prod/pep/endpoint/se/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
  $(NLBUILDROOT)/prod/common/include \


LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR) \
	$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

LIBS += \
  xmllite\
  shlwapi\
  Ntquery\
  ole32
  
  