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
SUBTARGET = pgp_adapter
MANIFEST = $(BINDIR)/pgp_adapter$(SUBTARGET_SUFFIX).dll.manifest

# On Windows, add MD
# Fix the call error by sharing dynamic linked CRT with pa_encrypt.dll
DYNAMICCRT=yes

DFLAGS+=-D_USRDLL -DADAPTER_BASE_EXPORTS -DPGP_WIN32 -DPGP_UNICODE=1 -D_WINDLL

ifeq ($(TARGETENVARCH), x86)
	 DFLAGS+=-D_USE_32BIT_TIME_T
endif

# Define the objects of the target module
SRC +=  \
	src/stdafx.cpp \
	src/GsmSdk.cpp \
	src/log.cpp \
	src/passcache.cpp \
	src/pgp_adapter.cpp \
	src/PgpSdkAdapter.cpp \
	src/PgpSdkFunc.cpp \
	src/util.cpp \

RCSRC = src/pgp_adapter.rc
  
# Define the custom header include path
INCPATH += \
  include  \
  ../include \
  include/pgp \
  include/pgp/win32 \
  

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  
LIBS += shlwapi

# Define the custom library to link
# eg. mylib myother_lib

DFLAGS +=

