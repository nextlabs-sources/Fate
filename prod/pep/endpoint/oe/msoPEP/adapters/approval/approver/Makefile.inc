
VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = CE_Approval2003

SRC = \
	src/approver.cpp \
	src/approverobj.cpp \
	src/backthread.c \
	src/gpgUtility.c \
	src/Log.c \
	src/multiftp.c \
	src/olHandler.c \
	src/WinAD.c \
	src/zipencryptor.c \
	src/stdafx.cpp \

RCSRC = src/approver.rc

# .idl file must be placed in SRC directory.
IDL_SRC = src/approver.idl

INCPATH += \
  ./include \
  ./idlgen \
  ./gpg \
  ../ftpcli/include
  
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

LIBS += ftpcli$(SUBTARGET_SUFFIX) secur32

ifeq ($(OFFICEVERSION), 2003)
	DFLAGS += -DOUTLOOK_2003
else
	DFLAGS += -DOUTLOOK_2007
endif
