
VERSION_PRODUCT		= $(VERSION_PRODUCT_RDE)
VERSION_MAJOR		= $(VERSION_MAJOR_RDE)
VERSION_MINOR		= $(VERSION_MINOR_RDE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_RDE)
VERSION_PATCH		= $(VERSION_PATCH_RDE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

SUBTARGET = nl_devenf_plugin

SRC = src/nl_devenf_lib.cpp \
      src/nl_devenf_plugin.cpp \
      src/nl_devenf_device.cpp \
      src/nl_devenf_sdk_wrapper.cpp

INCPATH += \
  include \
  ../inc \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pep/common/nl_device/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  c:/winddk/7600.16385.1/inc/ddk \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(NLBUILDROOT)/prod/pep/common/nl_device/$(BINDIR)

LIBS += ole32 psapi nl_device_static$(SUBTARGET_SUFFIX) setupapi
