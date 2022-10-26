
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = nlcommonlib

SRC = \
  src/CommContainer.cpp \
  src/dllmain.cpp \
  src/NamedPipeAdapter.cpp \
  src/nlcommonlib.cpp \
  src/stdafx.cpp

INCPATH += \
  include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

LIBPATH += 

LIBS += 
