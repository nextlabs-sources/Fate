
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = StopEnforcer

SRC = \
  src/enforcerStopper.cpp

RCSRC = src/enforcerStopper.rc

INCPATH += \
  include                         \
  $(NLBUILDROOT)/prod/pc/cesdk/include         \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEPrivate/include 

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(BINDIR) \
  $(NLENFORCERSDIR)/sdk/client/stub/CEPrivate/$(BINDIR)  \
  $(NLENFORCERSDIR)/sdk/client/applications/pdpStopperLib/$(BINDIR)

LIBS +=     \
  cesdk$(SUBTARGET_SUFFIX) \
  PDPStop$(SUBTARGET_SUFFIX)

DFLAGS +=

LDFLAGS  += -SUBSYSTEM:WINDOWS
