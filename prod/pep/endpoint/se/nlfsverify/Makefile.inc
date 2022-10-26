
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = nlfsverify

SRC = \
      src/nlfsverify.cpp

INCPATH += \
  include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += \
  psapi

CUSTOM_DEFINES =

CFLAGS   += $(CUSTOM_DEFINES)
CXXFLAGS += $(CUSTOM_DEFINES)