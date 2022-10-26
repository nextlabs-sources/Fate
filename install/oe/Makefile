ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile.ver

PRODUCT_DIR_NAME=oe
VERSION_STR=$(VERSION_MAJOR_OE).$(VERSION_MINOR_OE)
ISM_TEMPLATE_FILE_NAME=OutlookEnforcer.ism
ISM_RELEASE=msi
# ISM_PRODUCT_CONFIG32=32bit
ISM_PRODUCT_CONFIG64=64bit
# MSI_FILE32=EDP-OutlookEnforcer-setup.msi
MSI_FILE64=EDP-OutlookEnforcer-setup64.msi

ifeq ($(BUILDTYPE),)
	BUILDTYPE=dev
endif

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

include $(NLBUILDROOT)/install/scripts/Makefile.install
