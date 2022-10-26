ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile.ver

PRODUCT_DIR_NAME=ne
VERSION_STR=$(VERSION_MAJOR_NE).$(VERSION_MINOR_NE).$(VERSION_MAINTENANCE_NE).$(VERSION_PATCH_NE)
ISM_TEMPLATE_FILE_NAME=NetworkEnforcer.ism
ISM_RELEASE=msi
ISM_PRODUCT_CONFIG32=32bit
ISM_PRODUCT_CONFIG64=64bit
MSI_FILE32=EDP-NetworkEnforcer-setup.msi
MSI_FILE64=EDP-NetworkEnforcer-setup64.msi

ifeq ($(BUILDTYPE),)
	BUILDTYPE=dev
endif

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

include $(NLBUILDROOT)/install/scripts/Makefile.install
