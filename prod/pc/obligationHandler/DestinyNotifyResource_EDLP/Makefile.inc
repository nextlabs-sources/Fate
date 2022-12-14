
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = ComplianceAgentNotifyResource_EDLP
DFLAGS += -DDESTINYNOTIFYRESOURCE_EXPORTS

SRC = \
  src/stdafx.cpp \
  src/DestinyNotifyResource.cpp \

RCSRC = src/DestinyNotifyResource_EDLP.rc

INCPATH += \
  include \
  ../DestinyNotify/include         

LIBPATH +=

LIBS += ole32 ntquery
