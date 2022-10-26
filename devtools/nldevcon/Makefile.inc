
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER = VS2008
PLATFORMSDK=7.0

TARGET = nldevcon
MANIFEST = $(BINDIR)/nldevcon.exe.manifest

WINDDK = C:/WinDDK/7600.16385.1

SRC = \
      src/nldevcon.cpp	 \
      src/install.cpp	 \
      src/enum.cpp		 \
      src/devregprop.cpp \
      src/svccon.cpp
	  
RCSRC = src/nldevcon.rc

INCPATH += \
  include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

LIBPATH = \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-$(TARGETENVARCH)/lib

# MSVC libs must be placed in front of DDK libs to avoid link error.
ifeq ($(TARGETENVARCH), x86)
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib \
  c:/winddk/7600.16385.1/lib/wnet/i386 \
  c:/winddk/7600.16385.1/lib/crt/i386
else
  LIBPATH += \
  $(MSVC_ROOT)/VC/lib/amd64 \
  c:/winddk/7600.16385.1/lib/wnet/amd64	      \
  c:/winddk/7600.16385.1/lib/crt/amd64 
endif

LIBS += \
    Setupapi \
    Cfgmgr32
	
LDFLAGS += -FORCE:MULTIPLE -MANIFEST -MANIFESTUAC:"level='requireAdministrator' uiAccess='false'"

LOCAL_DEFINES += \
  -DVERSION_MAJOR=$(VERSION_MAJOR) \
  -DVERSION_MINOR=$(VERSION_MINOR)

CFLAGS   += $(LOCAL_DEFINES)
CXXFLAGS += $(LOCAL_DEFINES)
