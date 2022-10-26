
VERSION_PRODUCT		= $(VERSION_PRODUCT_RDE)
VERSION_MAJOR		= $(VERSION_MAJOR_RDE)
VERSION_MINOR		= $(VERSION_MINOR_RDE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_RDE)
VERSION_PATCH		= $(VERSION_PATCH_RDE)

COMPILER = VS2008
PLATFORMSDK=7.0

SUBDIRS = 
SUBTARGET = nlPnpDriverInstaller

SRC = src/nlPnpDriverInstaller.cpp 

INCPATH += \
  ./include \
  $(MSVC_ROOT)/VC/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
  $(NLBUILDROOT)/prod/common/celog2/include         \
  c:/winddk/7600.16385.1/inc/api

# These includes are from Enforcer branch. If encounter issue, 
# check if c:/winddk/7600.16385.1/inc/api is appropriate.
#  ../inc \
#  ../../../../../sdk/include \
#  c:/winddk/6001.18001/inc/api \
#  c:/winddk/6001.18001/inc/ddk

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  c:/winddk/7600.16385.1/inc/ddk

# These includes are from Enforcer branch. If encounter issue, 
# check if c:/winddk/7600.16385.1/inc/ddk is appropriate.
#  c:/winddk/6001.18001/lib/w2k/i386 \
#  $(NLBUILDROOT)/prods/SDK/build.output/$(LIBDIR) \

LIBS += ole32 \
       psapi \
       advapi32 \
       kernel32 \
       setupapi \
       shell32 \
       user32 \
       CELog2$(SUBTARGET_SUFFIX)