
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET=cesdk

LIBS += \
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
	celog$(SUBTARGET_SUFFIX)

LIBS += Mpr
LDFLAGS += /MACHINE:$(TARGETENVARCH)

LIBPATH += \
       $(BUILDOUTPUTDIR)/$(LIBDIR)

