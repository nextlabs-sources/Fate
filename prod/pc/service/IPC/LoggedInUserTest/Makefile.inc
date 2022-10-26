
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
TARGET = LoggingInUserTest

# Define the objects of the target module
SRC = stdafx.cpp           \
      LoggedInUserTest.cpp            

LIBPATH += \
  ../Shared/$(BUILDTYPE)_$(TARGETENV) 

LIBS += Shared Netapi32
