
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# This is the old way of including manifest.  It should be changed to use -MANIFEST    
    
ifeq ($(OS), CYGWIN_NT-5.1) 
    MANIFEST=$(BINDIR)/nlca_service.exe.manifest
endif

TARGET = nlca_service

SRC = \
  src/nlca_service.cpp \
  src/nlca_service_efilter.cpp

INCPATH += \
  $(NLBUILDROOT)/prod/common/celog/include           \
  $(NLBUILDROOT)/prod/common/include           \
  $(NLBUILDROOT)/prod/common/brain/include			    \
  $(NLBUILDROOT)/prod/pc/content/analyze/include \
  $(NLBUILDROOT)/prod/pc/content/framework/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 

LIBPATH += \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)             \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR)             \
  $(NLBUILDROOT)/prod/pc/content/framework/$(BINDIR) \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

NLCA_SERVICE_FLAGS = -DBOOST_REGEX_RECURSIVE -DNLCA_PARALLEL_SEARCH /openmp

CFLAGS   += $(NLCA_SERVICE_FLAGS)
CXXFLAGS += $(NLCA_SERVICE_FLAGS)

LIBS += ole32 ntquery \
  celog$(SUBTARGET_SUFFIX) \
  cebrain_static$(SUBTARGET_SUFFIX) \
  nlca_framework$(SUBTARGET_SUFFIX)

