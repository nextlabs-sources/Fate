
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = IPCProxy
SUBTARGET_STATIC = yes

# Define the objects of the target module
SRC = stdafx.cpp           \
      IPCProxy.cpp            

INCPATH += \
  ${EXTERNALSDIR}/j2sdk/jdk1.6.0_18/include/ \
  ${EXTERNALSDIR}/j2sdk/jdk1.6.0_18/include/win32 \
  $(NLENFORCERSDIR)/sdk/include 

LIBPATH += \
  ../Shared/$(BUILDTYPE)_$(TARGETENV) 

LIBS += Shared
