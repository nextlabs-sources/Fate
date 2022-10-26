
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = nlping

SRC = \
      src/nlping.cpp

# Define the custom header include path
INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += \
  psapi

CUSTOM_DEFINES = -DNOMINMAX

CFLAGS   += $(CUSTOM_DEFINES)
CXXFLAGS += $(CUSTOM_DEFINES)
