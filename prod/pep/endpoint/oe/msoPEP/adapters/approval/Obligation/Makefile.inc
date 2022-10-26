
VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = Obligation

SRC = \
	src/Log.cpp \
	src/OBEmail.cpp \
	src/Removable.cpp \
	src/WinAD.cpp \
	src/EMailDlg.cpp \
	src/FtpDlg.cpp \
	src/ISBListBox.cpp \
	src/stdafx.cpp \
	src/Obligation.cpp
	
RCSRC = src/Obligation.rc

INCPATH += \
	./include

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)
