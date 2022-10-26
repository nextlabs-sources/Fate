ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile.ver

PRODUCT_DIR_NAME=KMService
VERSION_STR=$(VERSION_MAJOR_KMC).$(VERSION_MINOR_KMC).$(VERSION_MAINTENANCE_KMC).$(VERSION_PATCH_KMC)
ISM_TEMPLATE_FILE_NAME=KeyManagement.ism
ISM_RELEASE=msi
ISM_PRODUCT_CONFIG32=32bit
ISM_PRODUCT_CONFIG64=64bit
MSI_FILE32=CC-KeyManagementForWindows-setup.msi
MSI_FILE64=CC-KeyManagementForWindows-setup64.msi

ifeq ($(BUILDTYPE),)
	BUILDTYPE=dev
endif

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

include $(NLBUILDROOT)/install/scripts/Makefile.install
