ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif


#
# Default values
#

include $(NLBUILDROOT)/build/Makefile.ver

WINSDK_ROOT = O:/
VERSION_STR=$(VERSION_MAJOR_EDP).$(VERSION_MINOR_EDP).$(VERSION_MAINTENANCE_EDP).$(VERSION_PATCH_EDP)
OFFICIALCERT=0
# SIGNTOOL_OFFICIAL_TOOL=$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe
SIGNTOOL_OFFICIAL_TOOL=echo
SIGNTOOL_OFFICIAL_ARGS=sign /ac c:/release/bin/DigiCertAssuredIDRootCA.cer /f c:/release/bin/NextLabs.pfx /p IiVf1itvOrqJ /n "NextLabs Inc." /fd sha256 /tr http://timestamp.digicert.com
SIGNTOOL_OFFICIAL='$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS)
# SIGNTOOL_DEV=$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe
SIGNTOOL_DEV=echo
SIGNTOOL_DEV_ARGS=sign /f ${NLBUILDROOT}/build/NextLabsDebug.pfx /p 123blue! /n "Nextlabs Debug" /fd sha256

ifeq ($(BUILDTYPE),)
	BUILDTYPE=dev
endif

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

VERSION_BUILD_SHORT=$(firstword $(subst /, ,$(VERSION_BUILD)))

#
# Prepare environment
#

ifeq ($(ProgramW6432), )
	ENV_OS=x86
	PROGRAM_FILES_X86=C:/Program Files
else
	ENV_OS=x64
	PROGRAM_FILES_X86=C:/Program Files (x86)
endif

PROGRAM_FILES_X86_DOS=$(subst /,\,$(PROGRAM_FILES_X86))

PERL=perl

INSTALL_DIR=$(NLBUILDROOT)/install/reskit
INSTALL_BUILD_DIR=$(INSTALL_DIR)/build
DATA_DIR=$(INSTALL_BUILD_DIR)/data
DATA_INSTALLER_DIR=$(DATA_DIR)/installer
PACKAGE_FILE_NAME=ResourceKit-$(VERSION_STR)-$(VERSION_BUILD_SHORT)-$(shell date +"%Y%m%d%H%M").zip
PUBLISH_DIR=$(NLBUILDROOT)/bin/installer

PACKAGE_DIR_WDE32=$(NLBUILDROOT)/install/wde/build/package32
PACKAGE_DIR_WDE64=$(NLBUILDROOT)/install/wde/build/package64
PACKAGE_DIR_NE32=$(NLBUILDROOT)/install/ne/build/package32
PACKAGE_DIR_NE64=$(NLBUILDROOT)/install/ne/build/package64
PACKAGE_DIR_RDE32=$(NLBUILDROOT)/install/rde/build/package32
PACKAGE_DIR_RDE64=$(NLBUILDROOT)/install/rde/build/package64
PACKAGE_DIR_OE32=$(NLBUILDROOT)/install/oe/build/package32
PACKAGE_DIR_LME32=$(NLBUILDROOT)/install/lme/build/package32
PACKAGE_DIR_OCE32=$(NLBUILDROOT)/install/oce/build/package32
PACKAGE_DIR_SE32=$(NLBUILDROOT)/install/se/build/package32
PACKAGE_DIR_SE64=$(NLBUILDROOT)/install/se/build/package64
PACKAGE_DIR_KMC32=$(NLBUILDROOT)/install/KMService/build/package32
PACKAGE_DIR_KMC64=$(NLBUILDROOT)/install/KMService/build/package64

MSI_FILE_WDE32=EDP-DesktopEnforcerForWindows-setup.msi
MSI_FILE_WDE64=EDP-DesktopEnforcerForWindows-setup64.msi
MSI_FILE_NE32=EDP-NetworkEnforcer-setup.msi
MSI_FILE_NE64=EDP-NetworkEnforcer-setup64.msi
MSI_FILE_RDE32=EDP-RemovableDeviceEnforcer-setup.msi
MSI_FILE_RDE64=EDP-RemovableDeviceEnforcer-setup64.msi
MSI_FILE_OE32=EDP-OutlookEnforcer-setup.msi
MSI_FILE_LME32=EDP-LiveMeetingEnforcer-setup.msi
MSI_FILE_OCE32=EDP-OfficeCommunicatorEnforcer-setup.msi
MSI_FILE_SE32=EDP-SystemEncryptionForWindows-setup.msi
MSI_FILE_SE64=EDP-SystemEncryptionForWindows-setup64.msi
MSI_FILE_KMC32=CC-KeyManagementForWindows-setup.msi
MSI_FILE_KMC64=CC-KeyManagementForWindows-setup64.msi

GPOPREP=$(DATA_INSTALLER_DIR)/EnforcerGPOPrep.exe


#
# Print environment
#

$(info -----------------------------------------------------------------------------)
$(info Build type                  = $(BUILDTYPE))
$(info Build #                     = $(VERSION_BUILD))
$(info Install directory           = $(INSTALL_DIR))
$(info Data directory              = $(DATA_DIR))
$(info Package file                = $(PACKAGE_FILE_NAME))
$(info Publish directory           = $(PUBLISH_DIR))
$(info Current OS                  = $(ENV_OS))
$(info Program files (x86)         = $(PROGRAM_FILES_X86))
$(info Program files (x86) DOS     = $(PROGRAM_FILES_X86_DOS))
$(info -----------------------------------------------------------------------------)


#
# Targets
#

.PHONY: all
all: assembly gpo publish

.PHONY: assembly
assembly:
	@echo "Prepare assembly"
	@if [ -d $(DATA_DIR) ]; then						\
		echo "INFO: Removing old build directory" ;		\
		rm -rf $(INSTALL_BUILD_DIR) ;					\
	fi
	"$(PERL)" $(INSTALL_DIR)/prepareAssembly.pl --buildType=$(BUILDTYPE) --buildNum=$(VERSION_BUILD) --version=$(VERSION_STR)

.PHONY: gpo
gpo:
	@echo "Prepare installer transform files"
	if [ -e $(PACKAGE_DIR_WDE32)/$(MSI_FILE_WDE32) ]; then												\
		cd $(PACKAGE_DIR_WDE32); $(GPOPREP) -i $(MSI_FILE_WDE32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "${SIGNTOOL_DEV} ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;								\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_WDE32)) ;	\
		rm -f GPOTransform.mst ;																		\
	fi
	if [ -e $(PACKAGE_DIR_NE32)/$(MSI_FILE_NE32) ]; then												\
		cd $(PACKAGE_DIR_NE32); $(GPOPREP) -i $(MSI_FILE_NE32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;							\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_NE32)) ;		\
		rm -f GPOTransform.mst ;																		\
	fi
	if [ -e $(PACKAGE_DIR_RDE32)/$(MSI_FILE_RDE32) ]; then												\
		cd $(PACKAGE_DIR_RDE32); $(GPOPREP) -i $(MSI_FILE_RDE32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;							\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_RDE32)) ;	\
		rm -f GPOTransform.mst ;																		\
	fi
	if [ -e $(PACKAGE_DIR_OE32)/$(MSI_FILE_OE32) ]; then												\
		cd $(PACKAGE_DIR_OE32); $(GPOPREP) -i $(MSI_FILE_OE32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;							\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_OE32)) ;		\
		rm -f GPOTransform.mst ;																		\
	fi
	if [ -e $(PACKAGE_DIR_LME32)/$(MSI_FILE_LME32) ]; then												\
		cd $(PACKAGE_DIR_LME32); $(GPOPREP) -i $(MSI_FILE_LME32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;							\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_LME32)) ;	\
		rm -f GPOTransform.mst ;																		\
	fi
	if [ -e $(PACKAGE_DIR_OCE32)/$(MSI_FILE_OCE32) ]; then												\
		cd $(PACKAGE_DIR_OCE32); $(GPOPREP) -i $(MSI_FILE_OCE32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;							\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_OCE32)) ;	\
		rm -f GPOTransform.mst ;																		\
	fi
	if [ -e $(PACKAGE_DIR_SE32)/$(MSI_FILE_SE32) ]; then												\
		cd $(PACKAGE_DIR_SE32); $(GPOPREP) -i $(MSI_FILE_SE32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;							\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_SE32)) ;		\
		rm -f GPOTransform.mst ;																		\
	fi
	if [ -e $(PACKAGE_DIR_KMC32)/$(MSI_FILE_KMC32) ]; then												\
		cd $(PACKAGE_DIR_KMC32); $(GPOPREP) -i $(MSI_FILE_KMC32) ;										\
		if [ $(OFFICIALCERT) -eq 0 ]; then																\
			echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst" ;							\
			'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} GPOTransform.mst ;									\
		else																							\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst" ;				\
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) GPOTransform.mst ;					\
		fi ;																							\
		cp -f GPOTransform.mst $(DATA_INSTALLER_DIR)/$(subst setup.msi,gpo.mst,$(MSI_FILE_KMC32)) ;	\
		rm -f GPOTransform.mst ;																		\
	fi

.PHONY: publish
publish:
	@echo "Publish resource kits"
	@if [ ! -d $(PUBLISH_DIR) ]; then					\
		mkdir -p $(PUBLISH_DIR)	;						\
	fi
	cd $(DATA_DIR); zip -r $(INSTALL_BUILD_DIR)/$(PACKAGE_FILE_NAME) *
	cp -f $(INSTALL_BUILD_DIR)/$(PACKAGE_FILE_NAME) $(PUBLISH_DIR)
	@echo "Content of $(PUBLISH_DIR)"
	@ls -al $(PUBLISH_DIR)

.PHONY: clean
clean:
	@if [ -d $(DATA_DIR) ]; then						\
		rm -rf build ;									\
	fi
