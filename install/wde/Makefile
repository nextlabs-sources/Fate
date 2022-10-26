ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile.ver

PRODUCT_DIR_NAME=wde
VERSION_STR=$(VERSION_MAJOR_WDE).$(VERSION_MINOR_WDE).$(VERSION_MAINTENANCE_WDE).$(VERSION_PATCH_WDE)
ISM_TEMPLATE_FILE_NAME=WindowsDesktopEnforcer.ism
ISM_RELEASE=msi
ISM_PRODUCT_CONFIG32=32bit
ISM_PRODUCT_CONFIG64=64bit
MSI_FILE32=EDP-DesktopEnforcerForWindows-setup.msi
MSI_FILE64=EDP-DesktopEnforcerForWindows-setup64.msi

ifeq ($(BUILDTYPE),)
	BUILDTYPE=dev
endif

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

include $(NLBUILDROOT)/install/scripts/Makefile.install
