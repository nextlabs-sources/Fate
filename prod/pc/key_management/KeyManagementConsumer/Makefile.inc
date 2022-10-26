
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

SUBTARGET = KeyManagementConsumer

SRC = src/stdafx.cpp					\
      src/KeyManagementConsumer.cpp     \
      src/KeyManagementEncryption.cpp	\
      src/dllmain.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/brain/include

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += cesdk$(SUBTARGET_SUFFIX) cebrain_static$(SUBTARGET_SUFFIX)

ifeq ($(OS), CYGWIN_NT-5.1)
	LIBS += Mpr
endif

DFLAGS += -DKEYMANAGEMENTDLL

