
VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = CE_Explorer

SRC = \
	src/CE_Explorer.cpp \
	src/COEShell.cpp \
	src/CE_Explorer_i.c \
	src/stdafx.cpp

RCSRC = src/CE_Explorer.rc

# .idl file must be placed in SRC directory.
IDL_SRC = src/CE_Explorer.idl

INCPATH += \
  ./include

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)
  
LIBS += ws2_32
