
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = ceBootstrapInjection

SRC = \
  src/stdafx.cpp \
  src/ceBootstrapInjection.cpp \
  src/Detours.cpp \
  src/LockMgr.cpp \
  src/ModuleScope.cpp \

INCPATH += \
  include \
  $(NLENFORCERSDIR)/platforms/common/include \
  $(NLENFORCERSDIR)/platforms/common/brain/include  \
  ../../lib/madCHook

LIBPATH += ../../lib/madCHook

LIBS += kernel32 ole32 ntquery oleaut32
