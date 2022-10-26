COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 

# Note: Disable code signing in compilation target (SIGNCODE=0). To sign a plugin using Adobe 
# certificate, add a CUSTOMTARGETS=sign_adobe in Makefile.inc.
ifeq ($(ADOBEAPP), reader)
	SUBTARGET = CE_Reader
	FORAPP = reader
	SIGNCODE=0
	CUSTOMTARGETS += sign_adobe
else
	SUBTARGET = CE_Acrobat
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
  src/PIMain.c 
  
INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLEXTERNALDIR)/boost/boost_1_43_0		      \
  $(NLBUILDROOT)/prod/pep/endpoint/adobepep/plugin/Headers/API \
  $(NLBUILDROOT)/prod/pep/endpoint/adobepep/plugin/Headers/SDK \

LIBPATH += \
	$(BUILDOUTPUTDIR)/$(LIBDIR_DIST)	\
	$(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free \
	lib

ifeq ($(TARGETENVARCH), x86)
	LIBPATH += \
		$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib 
else
	LIBPATH += \
		$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib
endif
    
LIBS += ole32 Ws2_32 Shlwapi GDIPlus Gdi32

