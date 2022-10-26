
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

TARGET = KeyUtil

SRC = src/stdafx.cpp           \
      src/KeyUtility.cpp            \

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/pc/key_management/KeyManagementConsumer/include

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL/$(BINDIR) \
  $(NLBUILDROOT)/prod/pc/key_management/KeyManagementConsumer/$(BINDIR)

LIBS += \
  KeyManagementConsumer$(SUBTARGET_SUFFIX) \
  cesdk$(SUBTARGET_SUFFIX)

ifeq ($(OS), CYGWIN_NT-5.1)
	LIBS += Mpr
endif
