
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

ifeq ($(OS), CYGWIN_NT-5.1) 
  MANIFEST=$(BINDIR)/nlca_framework$(SUBTARGET_SUFFIX).dll.manifest
endif

SUBTARGET = nlca_framework


SRC = \
  src/nlca_threadpool.cpp \
  src/nlca_rpc.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/celog/include           \
  $(NLBUILDROOT)/prod/common/include           \
  $(NLBUILDROOT)/prod/common/brain/include			    \
  $(NLBUILDROOT)/prod/pc/content/analyze/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 

LIBPATH += \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)             \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR) \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

NLCA_FRAMEWORK_FLAGS = -DNLCA_PARALLEL_SEARCH /openmp -D_WIN32_DCOM

CFLAGS   += $(NLCA_FRAMEWORK_FLAGS)
CXXFLAGS += $(NLCA_FRAMEWORK_FLAGS)

LIBS += ole32 ntquery \
    celog$(SUBTARGET_SUFFIX)			\
    cebrain_static$(SUBTARGET_SUFFIX)
