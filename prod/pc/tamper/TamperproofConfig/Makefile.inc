
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = nlTamperproofConfig
SUBTARGET_STATIC = yes

SRC = src/nlTamperproofConfig.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include    \
  $(NLBUILDROOT)/prod/common/include         \
  $(NLBUILDROOT)/prod/common/celog/include			\
  $(NLBUILDROOT)/prod/common/brain/include

LIBPATH += \
  $(NLBUILDROOT)/prod/common/brain/$(BINDIR)      \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR)             \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEM/$(BINDIR) \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += \
  cesdk$(SUBTARGET_SUFFIX) \
  celog$(SUBTARGET_SUFFIX) \
  cebrain$(SUBTARGET_SUFFIX)
