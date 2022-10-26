
VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = CEOffice


SRC = \
	src/CEOffice.cpp \
	src/Office.cpp \
	src/CEOffice_i.c \
	src/stdafx.cpp

RCSRC = src/CEOffice.rc

# .idl file must be placed in SRC directory.
IDL_SRC = src/CEOffice.idl

INCPATH += \
	./include \
	./idlgen \
	$(NLBUILDROOT)/prod/common \
	$(NLBUILDROOT)/prod/common/include \
	$(NLBUILDROOT)/prod/common/lib/mch/3.0/inc

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free
  
LIBS += ws2_32
