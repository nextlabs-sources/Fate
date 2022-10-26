
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER = VS2008
PLATFORMSDK=7.0

TARGET = nlsce

SRC = \
  src/stdafx.cpp \
  src/SCE.cpp \
  src/SCEServer.cpp \
  src/evaluate.cpp \
  src/Utility.cpp \
  src/PrtSrnKey.cpp \
  src/SCEProtocol.cpp \
  src/CApplication.cpp \
  src/CPEPClient.cpp \
  
INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)
  
ifeq ($(TARGETENVARCH), x86)
LIBPATH += \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib
else
LIBPATH += \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib
endif
  
LIBS +=  celog$(SUBTARGET_SUFFIX) gdi32 Psapi
