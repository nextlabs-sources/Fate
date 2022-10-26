
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 

SUBTARGET = AdobePEPTrm

SRC = \
  src/dllmain.cpp \
  src/ApiHook.cpp \
  src/OverLay.cpp \
  src/Encrypt.cpp

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/inc \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/pep/endpoint/adobepep/plugin/Samples/adobepep/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/NLVisualLabelingPA/include \
  $(NLBUILDROOT)/prod/common/celog2/include \
  $(NLBUILDROOT)/prod/pa/paf/include \
  
     
LIBPATH += \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/lib/free \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/celog2/$(BINDIR)  \
  
LIBS += nlcc_ulib_static$(SUBTARGET_SUFFIX) Shlwapi GDIPlus\
		CELog2$(SUBTARGET_SUFFIX)  \
  
