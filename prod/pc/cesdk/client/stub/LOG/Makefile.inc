
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = celogging
SUBTARGET_STATIC=yes

SRC = src/celog.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include           \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include     \
  $(NLBUILDROOT)/prod/pc/marshal/include  \
  $(NLBUILDROOT)/prod/pc/transport/include  \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/PEPMan/include 

LIBPATH += \
  $(NLPLATFORMSDIR)/common/brain/$(BINDIR)      \
  $(NLENFORCERSDIR)/sdk/client/stub/CEM/$(BINDIR) \
  $(NLENFORCERSDIR)/framework/transport/$(BINDIR) \
  $(NLENFORCERSDIR)/framework/marshal/$(BINDIR) \
  $(NLENFORCERSDIR)/sdk/client/stub/PEPMan/$(BINDIR) \
  $(NLENFORCERSDIR)/sdk/client/stub/CONN/$(BINDIR)  \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += \
  cebrain$(SUBTARGET_SUFFIX)			\
  cecem$(SUBTARGET_SUFFIX) \
  cemarshal50$(SUBTARGET_SUFFIX) \
  cetransport$(SUBTARGET_SUFFIX) \
  cepepman$(SUBTARGET_SUFFIX)
