
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = cetransctrl

SRC = src/TransCtrl.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include    \
  $(NLBUILDROOT)/prod/pc/transport/include    \
  $(NLBUILDROOT)/prod/pc/kif/include    \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/brain/include

LIBPATH += \
  $(NLPLATFORMSDIR)/common/brain/$(BINDIR)      \
  $(NLBUILDROOT)/prod/pc/kif/$(BINDIR) \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += \
  cesdk$(SUBTARGET_SUFFIX) \
  cekif$(SUBTARGET_SUFFIX)
