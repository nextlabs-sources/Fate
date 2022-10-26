
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = nlca_client

SRC = src/nlca_client.cpp

INCPATH += \
  include \
  ../../analyze/include \
  ../../framework/include \
  ../../client/include \
  $(NLPLATFORMSDIR)/common/include           \
  $(NLPLATFORMSDIR)/common/brain/include           \
  $(NLEXTERNALDIR)/boost/boost_1_36_0

LIBPATH += \
  $(NLEXTERNALDIR)/boost/boost_1_36_0/libs/regex/build/vc80 \
  $(NLPLATFORMSDIR)/win32/modules/content/client/$(BINDIR)	\
  ../../libs

LIBS += ole32 ntquery nlca_client
