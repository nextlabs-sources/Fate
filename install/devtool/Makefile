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
#SIGNTOOL_DEV="$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe"
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

INSTALL_DIR=$(NLBUILDROOT)/install/devtool
INSTALL_BUILD_DIR=$(INSTALL_DIR)/build
DATA_DIR=$(INSTALL_BUILD_DIR)/data
PACKAGE_FILE_NAME=DeveloperToolkit-$(VERSION_STR)-$(VERSION_BUILD_SHORT)-$(shell date +"%Y%m%d%H%M").zip
PUBLISH_DIR=$(NLBUILDROOT)/bin/installer


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
all: assembly publish

.PHONY: assembly
assembly:
	@echo "Prepare assembly"
	@if [ -d $(DATA_DIR) ]; then						\
		echo "INFO: Removing old build directory" ;		\
		rm -rf $(INSTALL_BUILD_DIR) ;					\
	fi
	"$(PERL)" $(INSTALL_DIR)/prepareAssembly.pl --buildType=$(BUILDTYPE) --buildNum=$(VERSION_BUILD) --version=$(VERSION_STR)

.PHONY: publish
publish:
	@echo "Publish developer toolkit"
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
