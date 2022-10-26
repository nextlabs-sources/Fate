
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = IPCStub

# Define the objects of the target module
SRC = stdafx.cpp           \
      dll.cpp            \
      IIPCRequestHandler.cpp            \
      IPCMessageChannel.cpp 			\
      IPCStub.cpp 			\
      RequestDispatcher.cpp

#INCPATH += \
#  ${EXTERNALSDIR}/j2sdk/jdk1.6.0_18/include/ \
#  ${EXTERNALSDIR}/j2sdk/jdk1.6.0_18/include/win32 \
#  $(NLENFORCERSDIR)\platforms\win32\modules\content\client\include \
#  $(NLENFORCERSDIR)\platforms\win32\modules\content\analyze\include \
#  $(NLENFORCERSDIR)\platforms\win32\modules\content\framework\include \
#  ${EXTERNALSDIR}\boost\boost_1_38_0 \
#  $(NLENFORCERSDIR)\platforms\common\include \
#  $(NLENFORCERSDIR)\sdk\include

LIBPATH += \
  ../Shared/$(BUILDTYPE)_$(TARGETENV) 
#  ${EXTERNALSDIR}\boost\boost_1_38_0\libs\regex\build\vc90 \
#  $(NLENFORCERSDIR)\platforms\win32\modules\content\libs

LIBS += Shared_static

DFLAGS += -DIPCSTUB_EXPORTS -D_USRDLL
