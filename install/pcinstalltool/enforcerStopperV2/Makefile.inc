
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET= EnforcerStopperV2
SUBTARGET_STATIC=yes

SRC = \
  src/enforcerStopper.cpp \
  src/nlQuench.cpp

RCSRC = src/enforcerStopper.rc

INCPATH += \
  include                         \
  $(NLBUILDROOT)/prod/pc/cesdk/include         \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/CEPrivate/include  \
  $(NLBUILDROOT)/prod/pc/quenchhash/include

LIBPATH += \
	$(NLBUILDROOT)/install/pcinstalltool/lib

LIBS += \
	ceprivate_static$(SUBTARGET_SUFFIX) \
	PDPStop_static$(SUBTARGET_SUFFIX) \
	advapi32 \
	iphlpapi \
	Psapi \
        cequenchhash_static \
	cebrain_static$(SUBTARGET_SUFFIX) \
	cecem_static$(SUBTARGET_SUFFIX) \
	ceconn_static$(SUBTARGET_SUFFIX) \
	ceeval_static$(SUBTARGET_SUFFIX) \
	celogging_static$(SUBTARGET_SUFFIX) \
	cemarshal50_static$(SUBTARGET_SUFFIX) \
	cepepman_static$(SUBTARGET_SUFFIX) \
	cetransport_static$(SUBTARGET_SUFFIX) \
	cesec_static$(SUBTARGET_SUFFIX) \
	ceservice_static$(SUBTARGET_SUFFIX) \

LIBS += Mpr

DFLAGS +=

LDFLAGS  += -SUBSYSTEM:WINDOWS
