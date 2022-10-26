
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 

# Note: Disable code signing in compilation target (SIGNCODE=0). To sign a plugin using Adobe 
# certificate, add a CUSTOMTARGETS=sign_adobe in Makefile.inc.
ifeq ($(ADOBEAPP), reader)
	SUBTARGET = NLReaderPEP
	FORAPP = reader
	SIGNCODE=0
	CUSTOMTARGETS += sign_adobe
else
	SUBTARGET = NLAcrobatPEP
	FORAPP = acrobat
endif

DFLAGS += -DPDMETADATA_HFT=1 -DPLUGIN=1 -DACRO_SDK_PLUGIN_SAMPLE=1 -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -DWIN_PLATFORM -DWIN_ENV

ifeq ($(ADOBEAPP), reader)
	DFLAGS += -DACRO_SDK_LEVEL=0x00090000 -DREADER_PLUGIN
else
	DFLAGS += -DACRO_SDK_LEVEL=0x000A0000 -DPI_ACROCOLOR_VERSION=0x00060000
endif

SRC = \
  src/StarterInit.cpp \
  src/WinAPIFile.cpp \
  src/PIMain.c \
  src/utilities.cpp \
  src/Encrypt.cpp \
  src/Inheritance.cpp \
  src/SaveAsObligation.cpp \
  src/PDDocInsertPages.cpp \
  src/Classify.cpp \
  src/Send.cpp \
  src/Edit.cpp \
  src/OverLay.cpp \
  src/AdobeXI.cpp \
  src/ReaderToolsWndProcess.cpp \
  src/GetMsgHook.cpp \
  src/HookDlg.cpp \
  src/contentstorage.cpp
  
INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/inc \
  $(NLBUILDROOT)/prod/common/nlcommonlib/include	\
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0/eframework/policy \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0/eframework/platform \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include \
  $(NLBUILDROOT)/prod/common/pdflib/pdflib/include \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pa/filetaggingPA/FileTagging/include \
  $(NLBUILDROOT)/prod/pa/paf/include \
  $(NLEXTERNALDIR)/podofo-0.8.1-NextLabs-WFSE/src		      \
  $(NLEXTERNALDIR)/boost/boost_1_43_0		      \
  $(NLBUILDROOT)/prod/pep/endpoint/adobepep/plugin/Headers/API \
  $(NLBUILDROOT)/prod/pep/endpoint/adobepep/plugin/Headers/SDK \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/common/celog2/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/NLVisualLabelingPA/include

LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR_DIST)	\
	$(NLBUILDROOT)/prod/common/celog2/$(BINDIR)  \
	$(NLBUILDROOT)/prod/common/lib/mch/3.1.13/lib/free \
	lib

ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		$(NLEXTERNALDIR)/podofo-0.8.1-NextLabs-WFSE/release_lib_win32_vs2008	\
		$(NLEXTERNALDIR)/tiff-4.0.0beta2/libtiff \
		$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib 
else
	LIBPATH += \
		$(NLEXTERNALDIR)/podofo-0.8.1-NextLabs-WFSE/release_lib_x64_vs2008		\
		$(NLEXTERNALDIR)/tiff-4.0.0beta2/release_lib_x64 \
		$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib
endif
    
LIBS += ole32 Ws2_32 nlcc_ulib_static32 Shlwapi GDIPlus Gdi32 \
		CELog2$(SUBTARGET_SUFFIX)  \

