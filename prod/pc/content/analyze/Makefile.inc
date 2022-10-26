
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER = VS2008
PLATFORMSDK=7.0

SUBDIRS = 
TARGET  = nlcrack

SRC = src/nlcrack.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pc/content/service/include \
  $(NLBUILDROOT)/prod/pc/content/client/include \
  $(NLBUILDROOT)/prod/pc/content/framework/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

ifeq ($(TARGETENVARCH),x86)
BOOSTDIR = $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib
else
BOOSTDIR = $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib
endif

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/content/libs \
  $(NLBUILDROOT)/prod/pc/content/framework/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)             \
  $(BOOSTDIR)

# Recursive is waiting for a library change and we really don't need parallel for the crack tool
# UNUSED_NLCRACK_FLAGS = -DBOOST_REGEX_RECURSIVE -DNLCA_PARALLEL_SEARCH /openmp
NLCRACK_FLAGS = -D_WIN32_DCOM

CFLAGS   += $(NLCRACK_FLAGS)
CXXFLAGS += $(NLCRACK_FLAGS)

LIBS += ole32 ntquery
