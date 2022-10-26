# Makefile to create CESDK


#
# Check for errors
#
ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif


#
# Prepare environment
#

INSTALL_DIR=$(NLBUILDROOT)/install/nxlfilter
INSTALL_BUILD_DIR=$(INSTALL_DIR)/build
NXL_FILTER_DIR_NAME=NXLFilter
PACKAGE_DIR=$(INSTALL_BUILD_DIR)/$(NXL_FILTER_DIR_NAME)
PUBLISH_DIR=$(NLBUILDROOT)/bin/installer

include $(NLBUILDROOT)/build/Makefile.ver

VERSION_STR=$(VERSION_MAJOR_PC).$(VERSION_MINOR_PC).$(VERSION_MAINTENANCE_PC).$(VERSION_PATCH_PC)

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

VERSION_BUILD_SHORT=$(firstword $(subst /, ,$(VERSION_BUILD)))

PACKAGE_FILE_SDK=$(NXL_FILTER_DIR_NAME)-$(VERSION_STR)-$(VERSION_BUILD_SHORT)-$(shell date +"%Y%m%d").zip


#
# Print environment
#

$(info -----------------------------------------------------------------------------)
$(info VERSION_STR          = $(VERSION_STR))
$(info VERSION_BUILD_SHORT  = $(VERSION_BUILD_SHORT))
$(info NXL_FILTER_DIR_NAME = $(NXL_FILTER_DIR_NAME))
$(info NLBUILDROOT          = $(NLBUILDROOT))
$(info NLEXTERNALDIR        = $(NLEXTERNALDIR))
$(info PACKAGE_FILE_SDK     = $(PACKAGE_FILE_SDK))
$(info PACKAGE_DIR          = $(PACKAGE_DIR))
$(info PUBLISH_DIR          = $(PUBLISH_DIR))
$(info -----------------------------------------------------------------------------)


#
# Targets
#

.PHONY: all
all: assembly package publish

.PHONY: assembly
assembly: assemble_nxlfilter 


.PHONY: assemble_nxlfilter
assemble_nxlfilter:
	@echo "INFO: Prepare NXLFilter"
	@if [ ! -d "$(PACKAGE_DIR)" ]; then					\
		mkdir -p "$(PACKAGE_DIR)" ;						\
	fi
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/NXLFilter.dll" "$(PACKAGE_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/NXLFilter32.dll" "$(PACKAGE_DIR)"

.PHONY: package
package:
	@echo "Package NXLFilter"
	@if [ ! -d "$(INSTALL_BUILD_DIR)" ]; then				\
		mkdir -p "$(INSTALL_BUILD_DIR)"	;				\
	fi
	cd $(INSTALL_BUILD_DIR); zip -r $(PACKAGE_FILE_SDK) $(NXL_FILTER_DIR_NAME)

.PHONY: publish
publish:
	@echo "Publish SDK package"
	@if [ ! -d "$(PUBLISH_DIR)" ]; then					\
		mkdir -p "$(PUBLISH_DIR)" ;						\
	fi
	cp -f $(INSTALL_BUILD_DIR)/*.zip "$(PUBLISH_DIR)"
	@echo "Content of $(PUBLISH_DIR)"
	@ls -al "$(PUBLISH_DIR)"

.PHONY: clean
clean:
	@if [ -e Makefile ]; then							\
		rm -rf build ;									\
	fi
