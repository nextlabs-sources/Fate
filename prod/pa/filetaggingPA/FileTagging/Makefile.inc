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
SUBTARGET = pa_filetagging

DFLAGS+=-D_USRDLL -DFILETAGGING_EXPORTS -D_WINDLL -D_ATL_STATIC_REGISTRY 

WINDDK = c:/Winddk/7600.16385.1

# Define the objects of the target module
SRC +=  \
	src/stdafx.cpp \
	src/NL_ListView_ComboBox.cpp \
	src/NXTLBS_ListView.cpp \
	src/FileTag.cpp \
	src/FileTagging.cpp \
	src/FileTagExport.cpp \
	src/FileTagMgr.cpp \
	src/FileTagViewDlg.cpp \
	src/WorkThreads.cpp \
	src/FileTagPanel.cpp \
	src/PromptDlg.cpp \
	src/Utils.cpp \
	src/HyperLinkStatic.cpp \
	src/DlgTagError.cpp

RCSRC = src/FileTagging.rc
  
# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include \
  ../../paf/include \
  ../includes \
  $(NLEXTERNALDIR)/wtl/wtl80/include \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

  

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/resattrmgr/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/resattrlib/$(BINDIR) \
  $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/$(BINDIR)  


# Define the custom library to link
# eg. mylib myother_lib
LIBS += resattrmgr$(SUBTARGET_SUFFIX) resattrlib$(SUBTARGET_SUFFIX) nl_sysenc_lib_static$(SUBTARGET_SUFFIX) cebrain_static$(SUBTARGET_SUFFIX) \
	CELog2$(SUBTARGET_SUFFIX) comctl32 comdlg32 gdi32 winspool oleaut32 uuid ole32 odbc32 odbccp32 shell32 advapi32 
	
ifeq ($(TARGETENVARCH), x86)
	LIBS += $(WINDDK)/lib/wnet/i386/fltlib 
else
	LIBS += $(WINDDK)/lib/wnet/amd64/fltlib
endif

DFLAGS +=

