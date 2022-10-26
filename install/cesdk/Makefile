# Makefile to create CESDK


#
# Check for errors
#
ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif



#
# Prepare environment
#

INSTALL_DIR=$(NLBUILDROOT)/install/cesdk
INSTALL_PC_DIR=$(NLBUILDROOT)/install/pc
INSTALL_BUILD_DIR=$(INSTALL_DIR)/build
ASSEMBLY_DIR=$(INSTALL_BUILD_DIR)/data
CESDK_DIR_NAME=PolicyAdapterSDK
PACKAGE_DIR=$(INSTALL_BUILD_DIR)/$(CESDK_DIR_NAME)
PACKAGE_INCLUDE_DIR=$(PACKAGE_DIR)/include
PACKAGE_X86_DIR=$(PACKAGE_DIR)/win_x86
PACKAGE_X64_DIR=$(PACKAGE_DIR)/win_x64
PACKAGE_CS_DIR=$(PACKAGE_DIR)/win_cs
PACKAGE_JAR_DIR=$(PACKAGE_DIR)/jar
PACKAGE_INSTALLER_DIR=$(PACKAGE_DIR)/installer
PACKAGE_INSTALLER_X86_DIR=$(PACKAGE_INSTALLER_DIR)/win_x86
PACKAGE_INSTALLER_X64_DIR=$(PACKAGE_INSTALLER_DIR)/win_x64
PUBLISH_DIR=$(NLBUILDROOT)/bin/installer
XLIB_DIR=$(NLBUILDROOT)/xlib
XLIB_BIN32_DIR=$(XLIB_DIR)/release_win_x86
XLIB_BIN64_DIR=$(XLIB_DIR)/release_win_x64


include $(NLBUILDROOT)/build/Makefile.ver

VERSION_STR=$(VERSION_MAJOR_PC).$(VERSION_MINOR_PC).$(VERSION_MAINTENANCE_PC).$(VERSION_PATCH_PC)

ifeq ($(VERSION_BUILD),)
	VERSION_BUILD=1
endif

VERSION_BUILD_SHORT=$(firstword $(subst /, ,$(VERSION_BUILD)))

PACKAGE_FILE_SDK=$(CESDK_DIR_NAME)-$(VERSION_STR)-$(VERSION_BUILD_SHORT)-$(shell date +"%Y%m%d").zip


#
# Print environment
#

$(info -----------------------------------------------------------------------------)
$(info VERSION_STR          = $(VERSION_STR))
$(info VERSION_BUILD_SHORT  = $(VERSION_BUILD_SHORT))
$(info CESDK_DIR_NAME       = $(CESDK_DIR_NAME))
$(info NLBUILDROOT          = $(NLBUILDROOT))
$(info ASSEMBLY_DIR         = $(ASSEMBLY_DIR))
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
assembly: assemble_fate_include assemble_fate_x86 assemble_fate_x64 assemble_fate_dotnet assemble_fate_java assemble_external_x86 assemble_external_x64 assemble_install

.PHONY: assemble_fate_include
assemble_fate_include:
	@echo "INFO: Prepare include"
	@if [ ! -d "$(PACKAGE_INCLUDE_DIR)" ]; then				\
		mkdir -p "$(PACKAGE_INCLUDE_DIR)" ;					\
	fi
	cp -f "$(NLBUILDROOT)/prod/common/include/nlconfig.hpp" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/include/NextLabs_Types.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/resattrmgr/include/resattrmgr.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/resattrlib/include/resattrlib.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/celog/include/celog.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/celog/include/celog_policy_file.hpp" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/celog/include/celog_policy_windbg.hpp" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/nl_klog/include/nl_klog.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/nlcc/nlcc.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/nlcc/klib/include/nlcc_klib.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/common/NextLabsTaggingLib/include/NextLabsTaggingLib.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0/eframework/platform/policy_controller.hpp" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/pc/cesdk/include/CEsdk.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/pc/cesdk/include/CEsdk_helper.hpp" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include/ceservice.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/pc/key_management/KeyManagementConsumer/include/KeyManagementConsumer.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/pep/endpoint/se/include/NextLabsEncryption_Types.h" "$(PACKAGE_INCLUDE_DIR)"
	cp -f "$(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/include/nl_sysenc_lib.h" "$(PACKAGE_INCLUDE_DIR)"

.PHONY: assemble_fate_x86
assemble_fate_x86:
	@echo "INFO: Prepare Fate x86"
	@if [ ! -d "$(PACKAGE_X86_DIR)" ]; then					\
		mkdir -p "$(PACKAGE_X86_DIR)" ;						\
	fi
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/cesdk32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/cesdk32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/resattrmgr32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/resattrmgr32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/resattrlib32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/resattrlib32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pdflib32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/tag_office2k732.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pafUI32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pafUI32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pa_encrypt32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pa_encrypt32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pa_filetagging32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pa_filetagging32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pgp_adapter32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/pgp_adapter32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/TagViewMenu32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/TagViewMenu32.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/zip_adapter32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/zip_adapter32.lib" "$(PACKAGE_X86_DIR)"	
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/celog32.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/nl_klog.lib" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/nl_klog.sys" "$(PACKAGE_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/nlcc_klib.lib" "$(PACKAGE_X86_DIR)"

.PHONY: assemble_fate_x64
assemble_fate_x64:
	@echo "INFO: Prepare Fate x64"
	@if [ ! -d "$(PACKAGE_X64_DIR)" ]; then				\
		mkdir -p "$(PACKAGE_X64_DIR)" ;					\
	fi
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/cesdk.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/cesdk.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/resattrmgr.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/resattrmgr.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/resattrlib.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/resattrlib.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pdflib.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/tag_office2k7.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pafUI.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pafUI.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pa_encrypt.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pa_encrypt.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pa_filetagging.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pa_filetagging.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pgp_adapter.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/pgp_adapter.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/TagViewMenu.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/TagViewMenu.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/zip_adapter.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/zip_adapter.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/celog.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/nl_klog.lib" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/nl_klog.sys" "$(PACKAGE_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/nlcc_klib.lib" "$(PACKAGE_X64_DIR)"

.PHONY: assemble_fate_dotnet
assemble_fate_dotnet:
	@echo "INFO: Prepare Fate C#"
	@if [ ! -d "$(PACKAGE_CS_DIR)" ]; then					\
		mkdir -p "$(PACKAGE_CS_DIR)" ;						\
	fi
	cp -f "$(NLBUILDROOT)/bin/release_dotnet2/NextLabs.CSCInvoke.dll" "$(PACKAGE_CS_DIR)"

.PHONY: assemble_fate_java
assemble_fate_java:
	@echo "INFO: Prepare Fate Java"
	@if [ ! -d "$(PACKAGE_JAR_DIR)" ]; then					\
		mkdir -p "$(PACKAGE_JAR_DIR)" ;						\
	fi
	cp -f "$(NLBUILDROOT)/bin/java/nlJavaSDK2.jar" "$(PACKAGE_JAR_DIR)"
	cp -f "$(NLBUILDROOT)/bin/java/nlJavaSDK2-doc.jar" "$(PACKAGE_JAR_DIR)"
	cp -f "$(NLBUILDROOT)/bin/java/JavaSDKService.properties" "$(PACKAGE_JAR_DIR)"

.PHONY: assemble_external_x86
assemble_external_x86:
	@echo "INFO: Prepare Sxternal x86"
	@if [ ! -d "$(PACKAGE_X86_DIR)" ]; then					\
		mkdir -p "$(PACKAGE_X86_DIR)" ;						\
	fi
	cp -f "$(XLIB_BIN32_DIR)/atl90.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/Microsoft.VC90.ATL.manifest" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/msvcm90.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/msvcp90.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/msvcr90.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/Microsoft.VC90.CRT.manifest" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/mfc90.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/mfc90u.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/mfcm90.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/mfcm90u.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/Microsoft.VC90.MFC.manifest" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/vcomp90.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)//Microsoft.VC90.OpenMP.manifest" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/freetype6.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/libtiff.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/PoDoFoLib.dll" "$(PACKAGE_X86_DIR)"
	cp -f "$(XLIB_BIN32_DIR)/zlib1.dll" "$(PACKAGE_X86_DIR)"	

.PHONY: assemble_external_x64
assemble_external_x64:
	@echo "INFO: Prepare External x64"
	@if [ ! -d "$(PACKAGE_X64_DIR)" ]; then				\
		mkdir -p "$(PACKAGE_X64_DIR)" ;					\
	fi
	cp -f "$(XLIB_BIN64_DIR)/atl90.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/Microsoft.VC90.ATL.manifest" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/msvcm90.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/msvcp90.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/msvcr90.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/Microsoft.VC90.CRT.manifest" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/mfc90.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/mfc90u.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/mfcm90.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/mfcm90u.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/Microsoft.VC90.MFC.manifest" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/vcomp90.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/Microsoft.VC90.OpenMP.manifest" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/libtiff.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/PoDoFoLib.dll" "$(PACKAGE_X64_DIR)"
	cp -f "$(XLIB_BIN64_DIR)/zlibwapi.dll" "$(PACKAGE_X64_DIR)"

.PHONY: assemble_install
assemble_install:
	@echo "INFO: Prepare Installer"
	@if [ ! -d "$(PACKAGE_INSTALLER_DIR)" ]; then				\
		mkdir -p "$(PACKAGE_INSTALLER_DIR)"	;					\
	fi
	@if [ ! -d "$(PACKAGE_INSTALLER_X86_DIR)" ]; then			\
		mkdir -p "$(PACKAGE_INSTALLER_X86_DIR)"	;				\
	fi
	@if [ ! -d "$(PACKAGE_INSTALLER_X64_DIR)" ]; then			\
		mkdir -p "$(PACKAGE_INSTALLER_X64_DIR)"	;				\
	fi
	cp -f "$(INSTALL_PC_DIR)/Script Files/include/servicecontrol.h" "$(PACKAGE_INSTALLER_DIR)"
	cp -f "$(INSTALL_PC_DIR)/Script Files/PluginInstallerSDK.obl" "$(PACKAGE_INSTALLER_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/nlQuench.exe" "$(PACKAGE_INSTALLER_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x86/PluginInstallerSDK32.dll" "$(PACKAGE_INSTALLER_X86_DIR)"
#TBD: Should determine if this project should be moved to devtools
#C:\depot\Fate\main\prod\pep\endpoint\rde\nlPnpDriverInstaller	
#	cp -f "$(NLBUILDROOT)/bin/release_win_x86/nlPnpDriverInstaller32.dll" "$(PACKAGE_INSTALLER_X86_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/nlQuench.exe" "$(PACKAGE_INSTALLER_X64_DIR)"
	cp -f "$(NLBUILDROOT)/bin/release_win_x64/PluginInstallerSDK.dll" "$(PACKAGE_INSTALLER_X64_DIR)"

.PHONY: package
package:
	@echo "Package CESDK"
	@if [ ! -d "$(INSTALL_BUILD_DIR)" ]; then				\
		mkdir -p "$(INSTALL_BUILD_DIR)"	;				\
	fi
	cd $(INSTALL_BUILD_DIR); zip -r $(PACKAGE_FILE_SDK) $(CESDK_DIR_NAME)

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
