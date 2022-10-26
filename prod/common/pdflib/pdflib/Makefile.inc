
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = pdflib
MANIFEST = $(BINDIR)/pdflib$(SUBTARGET_SUFFIX).dll.manifest

DYNAMICCRT=yes

SRC = \
  src/dllmain.cpp \
  src/NL_base64.cpp \
  src/NL_IncrementalUpdate.cpp \
  src/NL_PDFMgr.cpp \
  src/NL_PDFParser.cpp \
  src/NL_PDFXRefParserObject.cpp \
  src/NL_PDFXRefStreamParserObject.cpp \
  src/NL_Utils.cpp \
  src/PDFLib.cpp \
  src/stdafx.cpp 

INCPATH += \
  include \
  ${NLEXTERNALDIR}/podofo-0.8.1-NextLabs-WFSE/src \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
	${NLEXTERNALDIR}/podofo-0.8.1-NextLabs-WFSE/release_lib_win32_vs2008 
else
  LIBPATH += \
	${NLEXTERNALDIR}/podofo-0.8.1-NextLabs-WFSE/release_lib_x64_vs2008 	
endif

LIBS += PoDoFoLib
