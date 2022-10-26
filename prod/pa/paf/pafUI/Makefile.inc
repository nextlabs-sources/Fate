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
SUBTARGET = pafUI

DFLAGS+=-D_USRDLL -DADAPTER_BASE_EXPORTS -DPAFUI_EXPORTS 

OTHERSRC_DIR=HierarchicalStructureClassification/src

MANIFEST = $(BINDIR)/pafUI$(SUBTARGET_SUFFIX).dll.manifest

# Define the objects of the target module
SRC +=  \
	src/stdafx.cpp \
	src/global.cpp \
	src/mainFrameDlg.cpp \
	src/pafUI.cpp \
	src/progressdlg.cpp \
	src/UIInterface.cpp \
	src/log.cpp \

SRC += \
       HierarchicalStructureClassification/src/classifydata.cpp \
       HierarchicalStructureClassification/src/classifylistctrl.cpp \
       HierarchicalStructureClassification/src/dlgclassify2.cpp \
       HierarchicalStructureClassification/src/dlgtemplate.cpp \
       HierarchicalStructureClassification/src/dlgunit.cpp \
       HierarchicalStructureClassification/src/host.cpp \
       HierarchicalStructureClassification/src/HyperLinkStatic.cpp \
       HierarchicalStructureClassification/src/labelinfo.cpp \
       HierarchicalStructureClassification/src/propsheet.cpp \
       HierarchicalStructureClassification/src/sa.cpp \
       HierarchicalStructureClassification/src/xmlparser.cpp \
       HierarchicalStructureClassification/src/user.cpp \
  
RCSRC = src/pafUI.rc

# Define the custom header include path
INCPATH += \
  include  \
  include/pgp \
  include/pgp/win32 \
  ../include \
  HierarchicalStructureClassification/inc \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 \
  
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  
LIBS += Comctl32

# Define the custom library to link
# eg. mylib myother_lib

DFLAGS +=

