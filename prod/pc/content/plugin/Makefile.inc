
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = nlca_plugin

SRC = src/nlca_plugin.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/pc/content/analyze/include		\
  $(NLBUILDROOT)/prod/pc/content/client/include \
  $(NLBUILDROOT)/prod/pc/content/framework/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/content/client/$(BINDIR) \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

LIBS += \
  ole32 \
  ntquery \
  nlca_client$(SUBTARGET_SUFFIX)
