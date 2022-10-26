
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET=ipcstub

DFLAGS+=-D_USRDLL -DIPCSTUB_EXPORTS 

SRC += src/dll.cpp           \
     src/IIPCRequestHandler.cpp \
     src/IPCMessageChannel.cpp \
     src/IPCStub.cpp \
     src/RequestDispatcher.cpp \
     src/stdafx.cpp

INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/pc/IPCproxy/include \
  $(EXTERNALSDIR)/boost/boost_1_45_0 \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include
  
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/pc/IPCproxy/$(BINDIR)

LIBS += ipcproxy_static$(SUBTARGET_SUFFIX)
