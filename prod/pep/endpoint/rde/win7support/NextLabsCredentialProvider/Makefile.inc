
VERSION_PRODUCT		= $(VERSION_PRODUCT_RDE)
VERSION_MAJOR		= $(VERSION_MAJOR_RDE)
VERSION_MINOR		= $(VERSION_MINOR_RDE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_RDE)
VERSION_PATCH		= $(VERSION_PATCH_RDE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = NextLabsCredentialProvider

SRC = \
  src/helpers.cpp \
  src/guid.cpp \
  src/CNextLabsProvider.cpp \
  src/CNextLabsCredential.cpp \
  src/Dll.cpp 

INCPATH += \
  include 

LIBS += secur32 shlwapi ole32
