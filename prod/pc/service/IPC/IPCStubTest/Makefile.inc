
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = IPCStubTest

# Define the objects of the target module
SRC = stdafx.cpp           \
      IPCStubTest.cpp \
	  StubTestRequestHandler.cpp            

LIBPATH += \
  ../IPCStub/$(BUILDTYPE)_$(TARGETENV) 

LIBS += IPCStub
