
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = ObligationManager


SRC = \
  src/stdafx.cpp \
  src/DestinyNotify.cpp \
  src/DestinyNotifyRequestHandler.cpp \

RCSRC = \
  src/DestinyNotify.rc \


INCPATH += \
  include \
  ../../../modules/IPCstub/include \
  ../../../modules/IPCproxy/include \
  $(NLENFORCERSDIR)/platforms/common/include \
  $(NLENFORCERSDIR)/platforms/common/brain/include

LIBPATH += ../../../modules/IPCproxy/release_bin_win32 \
       ../../../modules/IPCstub/release_bin_win32 \

LIBS += ipcproxy_static ipcstub
