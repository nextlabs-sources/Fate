
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = ipcproxy
SUBTARGET_STATIC = yes

SRC = \
  src/ILog.cpp \
  src/IPCProxy.cpp \
  src/LogFactory.cpp \
  src/NotQuiteNullDacl.cpp \
  src/SecurityAttributesFactory.cpp \
  src/SimpleLog.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/include                 \
  $(NLBUILDROOT)/prod/common/brain/include           \

LIBPATH +=

LIBS += ole32 ntquery
