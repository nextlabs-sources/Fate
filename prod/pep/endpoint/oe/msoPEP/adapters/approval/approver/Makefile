ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

OFFICIALCERT=0
# SIGNTOOL_OFFICIAL_TOOL=$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe
SIGNTOOL_OFFICIAL_TOOL=echo
SIGNTOOL_OFFICIAL_ARGS=sign /ac c:/release/bin/DigiCertAssuredIDRootCA.cer /f c:/release/bin/NextLabs.pfx /p IiVf1itvOrqJ /n "NextLabs Inc." /fd sha256 /tr http://timestamp.digicert.com
SIGNTOOL_OFFICIAL='$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS)
MSVSIDE=x:/common7/IDE/devenv.exe
PROJECT=approver.vcproj

ifeq ($(TARGETENVARCH),)
	TARGETENVARCH=x86
endif

# Note: Cannot use $(error ) here because it will stop make TARGETSFILE=.../targets
ifneq ($(TARGETENVARCH), x86)
  $(warning Only x86 architecture is supported.)
endif

ifneq ($(BUILDTYPE), release)
	BUILDTYPE=debug
endif

BINDIR=$(BUILDTYPE)_win_$(TARGETENVARCH)
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)


$(info --------------------------------------------------------------------------)
$(info [Project])
$(info PROJECT=$(PROJECT))
$(info [Parameters])
$(info BUILDTYPE=$(BUILDTYPE))
$(info TARGETENVARCH=$(TARGETENVARCH))
$(info NLBUILDROOT=$(NLBUILDROOT))
$(info NLEXTERNALDIR=$(NLEXTERNALDIR))
$(info BUILDOUTPUTDIR=$(BUILDOUTPUTDIR))
$(info BINDIR=$(BINDIR))
$(info ---------------------------------------------------------------------------)


.PHONY: all
.PHONY: outlook2003
.PHONY: outlook2007
.PHONY: clean

all: outlook2003 outlook2007

outlook2003:
	@if [ "$(TARGETENVARCH)" == "x86" ]; then													\
		echo ""	;																				\
		echo "Building $(PROJECT) for Outlook 2003"	;											\
		echo "Compilation output is directed to ./outlook2003_$(BIN_DIR)/BuildLog.htm" ;		\
		if [ "$(BUILDTYPE)" == "debug" ]; then													\
			$(MSVSIDE) $(PROJECT) /build "outlook2003_debug|Win32" ;							\
		else																					\
			$(MSVSIDE) $(PROJECT) /build "outlook2003_release|Win32" ;							\
		fi ;																					\
		if [ $(OFFICIALCERT) -ne 0 ]; then														\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) outlook2003_$(BIN_DIR)/CE_Approver200332.dll" ; \
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) outlook2003_$(BIN_DIR)/CE_Approver200332.dll ; \
		fi ;																					\
		if [ ! -d $(BUILDOUTPUTDIR) ]; then														\
			mkdir -p $(BUILDOUTPUTDIR) ;														\
		fi ;																					\
		cp -f outlook2003_$(BIN_DIR)/CE_Approver200332.dll $(BUILDOUTPUTDIR) ;					\
		cp -f outlook2003_$(BIN_DIR)/CE_Approver200332.lib $(BUILDOUTPUTDIR) ;					\
		if [ "$(BUILDTYPE)" == "debug" ]; then													\
			cp -f outlook2003_$(BIN_DIR)/CE_Approver200332.pdb $(BUILDOUTPUTDIR) ;				\
		fi ;																					\
	else																						\
		echo "x64 not supported on target outlook2003." ;										\
	fi
	
outlook2007:
	@if [ "$(TARGETENVARCH)" == "x86" ]; then													\
		echo ""	;																				\
		echo "Building $(PROJECT) for Outlook 2007"	;											\
		echo "Compilation output is directed to ./outlook2007_$(BIN_DIR)/BuildLog.htm" ;		\
		if [ "$(BUILDTYPE)" == "debug" ]; then													\
			$(MSVSIDE) $(PROJECT) /build "outlook2007_debug|Win32" ;							\
		else																					\
			$(MSVSIDE) $(PROJECT) /build "outlook2007_release|Win32" ;							\
		fi ;																					\
		if [ $(OFFICIALCERT) -ne 0 ]; then														\
			echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) outlook2007_$(BIN_DIR)/CE_Approver200732.dll" ; \
			'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) outlook2007_$(BIN_DIR)/CE_Approver200732.dll ; \
		fi ;																					\
		if [ ! -d $(BUILDOUTPUTDIR) ]; then														\
			mkdir -p $(BUILDOUTPUTDIR) ;														\
		fi ;																					\
		cp -f outlook2007_$(BIN_DIR)/CE_Approver200732.dll $(BUILDOUTPUTDIR) ;					\
		cp -f outlook2007_$(BIN_DIR)/CE_Approver200732.lib $(BUILDOUTPUTDIR) ;					\
		if [ "$(BUILDTYPE)" == "debug" ]; then													\
			cp -f outlook2007_$(BIN_DIR)/CE_Approver200732.pdb $(BUILDOUTPUTDIR) ;				\
		fi ;																					\
	else																						\
		echo "x64 not supported on target outlook2007." ;										\
	fi

clean:
	@rm -rf outlook2003_release_win_x* outlook2003_debug_win_x* outlook2007_release_win_x* outlook2007_debug_win_x*
