
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = IPCJNI

# Define the objects of the target module
SRC = stdafx.cpp           \
      oswrapper.cpp        \
	  IPCJNI.cpp    \
	  SessionInfo.cpp 

INCPATH += \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include/win32 \
  $(NLBUILDROOT)/prod/pc/service/IPC/IPCJNI/generated_code \
  $(NLBUILDROOT)/prod/pc/content/client/include \
  $(NLBUILDROOT)/prod/pc/content/analyze/include \
  $(NLBUILDROOT)/prod/pc/content/framework/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0		   \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  ../../../../../../build

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/service/IPC/Shared/$(BINDIR)			\
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

LIBS += Shared_static$(SUBTARGET_SUFFIX) Imagehlp Netapi32 secur32 Crypt32 mpr Psapi

DFLAGS += -DIPCJNI_EXPORTS -DPSAPI_VERSION=1
