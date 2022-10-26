
VERSION_PRODUCT		= $(VERSION_PRODUCT_PCOE)
VERSION_MAJOR		= $(VERSION_MAJOR_PCOE)
VERSION_MINOR		= $(VERSION_MINOR_PCOE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PCOE)
VERSION_PATCH		= $(VERSION_PATCH_PCOE)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = PCStop

SRC = \
  src/PCStop.cpp

RCSRC = src/PCStop.rc

INCPATH +=

LIBPATH += \
	$(NLBUILDROOT)/install/pcinstalltool/lib \
	$(BUILDOUTPUTDIR)/$(BINDIR) \

LIBS += \
	enforcerStopperV2_static$(SUBTARGET_SUFFIX) \
	ceprivate_static$(SUBTARGET_SUFFIX) \
	PDPStop_static$(SUBTARGET_SUFFIX) \
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
	advapi32 \
	iphlpapi \
	Psapi

LIBS += Mpr

DFLAGS +=

LDFLAGS  += -SUBSYSTEM:WINDOWS