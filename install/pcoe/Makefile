ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile.ver

PRODUCT_DIR_NAME=pcoe
VERSION_STR=$(VERSION_MAJOR_PCOE).$(VERSION_MINOR_PCOE)
ISM_TEMPLATE_FILE_NAME_X64=NextLabsEnforcerforMicrosoftOutlook_x64.ism
ISM_TEMPLATE_FILE_NAME_X86=NextLabsEnforcerforMicrosoftOutlook_x86.ism
ISM_RELEASE=Media_MSI
ISM_PRODUCT_CONFIG32=32bit
MSI_FILE32=EDP-EndpointEnforcers.msi
ISM_PRODUCT_CONFIG64=64bit
MSI_FILE64=EDP-EndpointEnforcers64.msi

ifeq ($(BUILDTYPE),)
	BUILDTYPE=dev
endif

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

include $(NLBUILDROOT)/install/pcoe/Makefile.install
