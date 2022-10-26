# Makefile inc file for the pepman module
# The source files are not OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
TARGET=cepdpman

# Define the objects of the target module
SRC = src/PDPMan.cpp           \
     src/BootstrapInjectionService.cpp \
     src/NotQuiteNullDacl.cpp \
     src/SecurityAttributesFactory.cpp \
     src/ctrlmodCYGWIN_NT-5.1.cpp     \
     src/parseargsCYGWIN_NT-5.1.cpp \
     src/registry.cpp

# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/cesdk/include                    \
  $(NLBUILDROOT)/prod/common/include                 \
  $(NLBUILDROOT)/prod/common/brain/include           \
  $(NLBUILDROOT)/prod/common/celog/include           \
  $(NLBUILDROOT)/prod/pc/marshal/include      \
  $(NLBUILDROOT)/prod/pc/quenchhash/include   \
  $(NLBUILDROOT)/prod/pc/service/CONN/include      \
  $(NLBUILDROOT)/prod/pc/service/EVAL/include      \
  $(NLBUILDROOT)/prod/pc/service/CEPrivate/include \
  $(NLBUILDROOT)/prod/pc/tamper/CEProtect/include \
  $(NLBUILDROOT)/prod/pc/service/LOG/include \
  $(NLBUILDROOT)/prod/pc/service/Generic/include \
  $(NLBUILDROOT)/prod/pc/service/Sec/include \
  $(NLBUILDROOT)/prod/pc/transport/include    \
  $(NLBUILDROOT)/prod/pc/TransCtrl/include    \
  $(NLBUILDROOT)/prod/pc/kif/include \
  $(NLBUILDROOT)/prod/pc/IPCproxy/include \
  $(JDKDIR)/include \
  $(JDKDIR)/include/win32 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(JDKDIR)/lib \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

ifeq ($(OS), CYGWIN_NT-5.1)
 LIBPATH += $(NLPLATFORMSDIR)/win32/modules/IPCproxy/$(BINDIR)
endif

## Common libraries
LIBS += \
	jvm \
     cesdk$(SUBTARGET_SUFFIX)			\
     celog$(SUBTARGET_SUFFIX)			\
       cepdpconn$(SUBTARGET_SUFFIX)		\
       cepdpeval$(SUBTARGET_SUFFIX)     \
       cepdplog$(SUBTARGET_SUFFIX)      \
       cepdpprotect$(SUBTARGET_SUFFIX)  \
       cepdpprivate$(SUBTARGET_SUFFIX)  \
       cetransctrl$(SUBTARGET_SUFFIX) \
     cekif$(SUBTARGET_SUFFIX)

LIBS +=         \
  cepdpgeneric$(SUBTARGET_SUFFIX)  \
  cepdpsec$(SUBTARGET_SUFFIX)      \
  cequenchhash_static$(SUBTARGET_SUFFIX) \
  dbghelp \
  Advapi32 \
  iphlpapi \
  IPCproxy_static$(SUBTARGET_SUFFIX) \
  Userenv

LDFLAGS += -FORCE:MULTIPLE -MANIFEST

ifeq ($(OS), Darwin)
  LDFLAGS += -framework JavaVM
endif

ifeq ($(OS), Linux)
  INCPATH += $(EXTERNALSDIR)/boost/boost_1_43_0
endif
