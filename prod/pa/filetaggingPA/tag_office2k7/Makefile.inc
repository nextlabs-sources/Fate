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
SUBTARGET = tag_office2k7
MANIFEST = $(BINDIR)/tag_office2k7$(SUBTARGET_SUFFIX).dll.manifest

# On Windows, add MD
# Fix Bug 11206 by sharing dynamic linked CRT with resattrmgr.dll
DYNAMICCRT=yes

DFLAGS+=-D_USRDLL -DTAG_OFFICE2k7_EXPORTS 

# Define the objects of the target module
SRC +=  \
	src/nlofficerep_only_debug.cpp \
	src/stdafx.cpp \
	src/tag_office2k7.cpp \
	src/opclib.cpp 
	

RCSRC = src/tag_office2k7.rc
  
# Define the custom header include path
INCPATH += \
  include  \
  $(NLENFORCERSDIR)/platforms/common/resattrmgr/include \
  $(NLENFORCERSDIR)/platforms/common/resattrlib/include \
  $(NLENFORCERSDIR)/prod/pep/common/eframework/eframework.R3.0	\
  $(NLENFORCERSDIR)/prod/common/celog/include		      \
  ../../paf/include \
  ../includes \
  UILIB/ListViewEx \
  UILIB/NXTLBS_ListView \
  $(NLEXTERNALDIR)/zip_utils/zip_utils2005 \

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/celog/${BINDIR}

ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
  	$(NLEXTERNALDIR)/zip_utils/zip_utils2005/$(BUILDTYPE)
else
  LIBPATH += \
    $(NLEXTERNALDIR)/zip_utils/zip_utils2005/x64-$(BUILDTYPE)	
endif





LIBS += zip_utils KtmW32 celog$(SUBTARGET_SUFFIX)
    
# Define the custom library to link
# eg. mylib myother_lib

DFLAGS +=

