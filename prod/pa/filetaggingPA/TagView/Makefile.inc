# Makefile inc file for the brain module
# The source files are OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

# Define the compiler to be used. At present it can be VS2008/VS2008
COMPILER = VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = TagView

DFLAGS+=-D_USRDLL -D_ATL_DLL -D_WIN32_WINNT=0x0500 

# Define the objects of the target module
SRC +=  \
	src/stdafx.cpp \
	src/TagView.cpp \
	src/TagView_i.c \
	src/compreg.cpp \
	src/ShellExt.cpp \

RCSRC = src/TagView.rc
  
# Define the custom header include path
INCPATH += \
  include  \
  

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  
LIBS += comsvcs winmm

# Define the custom library to link
# eg. mylib myother_lib

DFLAGS +=

