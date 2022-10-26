# Makefile inc file for the celog module
# The source files are OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = resattrmgr
MANIFEST = $(BINDIR)/resattrmgr$(SUBTARGET_SUFFIX).dll.manifest

# On Windows, add MD
# Fix Bug 11206 by sharing dynamic linked CRT with tag_office2k7.dll
DYNAMICCRT=yes

CUSTOMTARGETS += copyheaders

ifeq ($(OS), CYGWIN_NT-5.1) 
DFLAGS += -DBUILDING_RESATTRMGR
endif

# Define the objects of the target module
ifeq ($(OS), Linux)
SRC += \
	src/NLCELog.cpp \
	src/resattrmgr.cpp \
    src/utils.cpp \
    src/ole.cpp \
    src/pdf.cpp \
    src/ooxml.cpp \
    src/pdfattrs.cpp \
    src/fsysea.cpp
    
# Define the custom header include path
INCPATH += \
	include \
       $(NLBUILDROOT)/prod/common/resattrlib/include	\
       $(NLBUILDROOT)/prod/common/celog2/include \
  	$(NLBUILDROOT)/prod/common/NextLabsTaggingLib/include \
       $(NLBUILDROOT)/prod/common/include \
        /usr/local/include/podofo \
        /usr/local/include/libgsf-1 \
        /usr/include/glib-2.0 \
        /usr/include/libxml2 \
        /usr/lib64/glib-2.0/include \
        ../../../../podofo-0.8.2/podofo \
        ../../../../podofo-0.8.2/src \
        ../../../../podofo-0.8.2/podofo-build \
        ../../../pep/indexer \
        ../../../pep/configuration

# Define the custom library path
LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/resattrlib/${BINDIR} \
  $(NLBUILDROOT)/prod/common/celog2/${BINDIR}		  \
	lib \
        /usr/local/lib \
	/opt/podofo/lib64 \
        ../../../pep/indexer

# Define the custom library to link
# starts with -l eg. -lmylib -lmyother_lib
LIBS += resattrlib$(SUBTARGET_SUFFIX) celog2$(SUBTARGET_SUFFIX) podofo gsf-1 attr indexer

else
SRC += \
	src/EncodeBaseInfoMgr.cpp \
	src/EncodeBomMgr.cpp \
	src/EncodeSteamMgr \
	src/ntfsAttrs.cpp \
	src/StreamReaderWriter.cpp \
	src/NLCELog.cpp \
	src/defimplreader.cpp \
	src/FileAttributeReader.cpp \
	src/FileAttributeWriter.cpp \
	src/FileAttributeRemove.cpp \
	src/attrmgr.cpp \
	src/resattrmgr.cpp \
	src/nxlAttrs.cpp \
	src/pdfattrs.cpp \
	src/tiffattrs.cpp \
	src/oleAttrs.cpp \
	src/Office2k7_attrs.cpp \
	src/base64.cpp \
	src/pdf_comment_attr.cpp \
	src/NxlFormatFile.cpp

# Define the custom header include path
INCPATH += \
	include \
	libtiff \
	$(NLEXTERNALDIR)/podofo-0.8.1-NextLabs-WFSE/src		      \
	$(NLBUILDROOT)/prod/common/resattrlib/include \
	$(NLBUILDROOT)/prod/common/celog2/include		      \
	$(NLBUILDROOT)/prod/common/NextLabsTaggingLib/include \
	$(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include \
	$(NLBUILDROOT)/prod/common/include \
	$(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0

# Define the custom library path
LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR)			\
	$(NLBUILDROOT)/prod/common/resattrlib/${BINDIR}	\
       $(NLBUILDROOT)/prod/common/NextLabsTaggingLib/${BINDIR}		\
       $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/${BINDIR}	\
       $(NLBUILDROOT)/prod/common/celog2/${BINDIR}		\
	lib \

ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
	$(NLEXTERNALDIR)/podofo-0.8.1-NextLabs-WFSE/release_lib_win32_vs2008	\
	$(NLEXTERNALDIR)/tiff-4.0.0beta2/libtiff
else
  LIBPATH += \
	$(NLEXTERNALDIR)/podofo-0.8.1-NextLabs-WFSE/release_lib_x64_vs2008 \
	$(NLEXTERNALDIR)/tiff-4.0.0beta2/release_lib_x64
endif

DFLAGS += \
    -DBUILDING_RESATTRMGR

# Define the custom library to link
# starts with -l eg. -lmylib -lmyother_lib
LIBS += ole32 resattrlib$(SUBTARGET_SUFFIX) celog2$(SUBTARGET_SUFFIX) NextLabsTaggingLib_static$(SUBTARGET_SUFFIX) nl_sysenc_lib$(SUBTARGET_SUFFIX) Shlwapi PoDoFoLib delayimp libtiff 
endif    
