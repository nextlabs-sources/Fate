
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = nlca_client

SRC = \
  src/nlca_client_helper.cpp 

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/include           \
  $(NLBUILDROOT)/prod/common/brain/include			    \
  $(NLBUILDROOT)/prod/pc/content/analyze/include \
  $(NLBUILDROOT)/prod/pc/content/framework/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 

LIBPATH += \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR) \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

LIBS += \
    ole32					\
    ntquery					\
    cebrain_static$(SUBTARGET_SUFFIX)
