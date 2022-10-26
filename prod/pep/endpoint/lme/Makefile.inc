
VERSION_PRODUCT		= $(VERSION_PRODUCT_LME)
VERSION_MAJOR		= $(VERSION_MAJOR_LME)
VERSION_MINOR		= $(VERSION_MINOR_LME)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_LME)
VERSION_PATCH		= $(VERSION_PATCH_LME)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = nllme

SRC = \
  src/AppShare.cpp \
  src/Blocks.cpp \
  src/CollDesktop.cpp \
  src/CollFrames.cpp \
  src/CollManager.cpp \
  src/CollMonitors.cpp \
  src/CollProcesses.cpp \
  src/CollSimple.cpp \
  src/CollWindow.cpp \
  src/CollWindows.cpp \
  src/Conference.cpp \
  src/Helper.cpp \
  src/HookBase.cpp \
  src/HookDll.cpp \
  src/HookedAudioMediaChannel.cpp \
  src/HookedConfEntityView.cpp \
  src/HookedConfSess.cpp \
  src/HookedConfSessPart.cpp \
  src/HookedConfSessPartEndpoint.cpp \
  src/HookedFlash.cpp \
  src/HookedLmDoc.cpp \
  src/HookedMediaChannel.cpp \
  src/HookedOperationContext.cpp \
  src/HookedSessPart.cpp \
  src/HookedSignalingChannel.cpp \
  src/HookedSignalingMessage.cpp \
  src/MsgHook.cpp \
  src/OfficeShare.cpp \
  src/PartDB.cpp \
  src/PCStream.cpp \
  src/Platform.cpp \
  src/Policy.cpp \
  src/Sink.cpp \
  src/SinkConfEntityView.cpp \
  src/SinkConfEntityViewCollectionEvents.cpp \
  src/SinkConfMediaChannel.cpp \
  src/SinkConfMediaChannelCollection.cpp \
  src/SinkConfSess.cpp \
  src/SinkConfSessPart.cpp \
  src/SinkMediaChannel.cpp \
  src/SinkSess.cpp \
  src/SinkSessPart.cpp \
  src/SinkSessPartCollection.cpp \
  src/SinkSessPartEndpointCollection.cpp \
  src/SinkUnknown.cpp \
  src/log.cpp\
  src/stdafx.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/inc \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

ifeq ($(BUILDTYPE), debug)
LIBPATH += \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/checked
else
LIBPATH += \
  $(NLBUILDROOT)/prod/common/lib/mch/3.0/lib/free
endif