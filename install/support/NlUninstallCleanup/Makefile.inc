
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER = VS2008
PLATFORMSDK=7.0

TARGET = NLUninstallCleanup

SRC = \
	src/nl_baseCleanup.cpp \
	src/nl_fileCleanup.cpp \
	src/nl_serviceCleanup.cpp \
	src/nl_registryCleanup.cpp \
	src/nl_configFile.cpp \
	src/nl_uninstallCleanup.cpp

INCPATH += \
  include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

LIBPATH = \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib 
else
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib/amd64 
endif

LOCAL_DEFINES += \
  -DVERSION_MAJOR=$(VERSION_MAJOR) \
  -DVERSION_MINOR=$(VERSION_MINOR)

CFLAGS   += $(LOCAL_DEFINES)
CXXFLAGS += $(LOCAL_DEFINES)
