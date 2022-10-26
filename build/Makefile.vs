# =============================================================================
# Top-level makefile include by makefiles that wrap around VisualStudio projects
# =============================================================================

ifeq ($(ProgramW6432), )
	PROGRAM_FILES_X86=C:/Program Files
else
	PROGRAM_FILES_X86=C:/Program Files (x86)
endif

NLBUILDROOT_DOS=$(subst /,\\,$(NLBUILDROOT))

OFFICIALCERT=0
# SIGNTOOL_OFFICIAL_TOOL=$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe
SIGNTOOL_OFFICIAL_TOOL=echo
SIGNTOOL_OFFICIAL_ARGS=sign /ac c:/release/bin/DigiCertAssuredIDRootCA.cer /f c:/release/bin/NextLabs.pfx /p IiVf1itvOrqJ /n "NextLabs Inc." /fd sha256 /tr http://timestamp.digicert.com
SIGNTOOL_OFFICIAL='$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS)
SIGNTOOL_DEBUG='$(SIGNTOOL_OFFICIAL_TOOL)' sign /f "$(NLBUILDROOT_DOS)\\build\\certificates\\NextLabsDebug.pfx" /p 123blue! /n "NextLabsDebug" /fd sha256
MSVSIDE=x:/common7/IDE/devenv.exe

ifeq ($(TARGETENVARCH),)
	TARGETENVARCH=x86
endif

ifneq ($(BUILDTYPE), release)
	BUILDTYPE=debug
endif

BIN_DIR=$(BUILDTYPE)_win_$(TARGETENVARCH)
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)

ifeq ($(VERSION_BUILD), )
	VERSION_BUILD=$(shell date +"%y.%j.%H%M")DX-$(HOSTNAME)-$(USERNAME)-$(shell date +"%Y.%m.%d-%H:%M")
endif


$(info --------------------------------------------------------------------------)
$(info [Targets])
$(info PROJECT=$(PROJECT))
$(info OFFICIALCERT=$(OFFICIALCERT))
$(info SIGNTOOL_OFFICIAL=$(SIGNTOOL_OFFICIAL))
$(info SIGNTOOL_DEBUG=$(SIGNTOOL_DEBUG))
$(info RCSRC=$(RCSRC))
$(info [Parameters])
$(info BUILDTYPE=$(BUILDTYPE))
$(info TARGETENVARCH=$(TARGETENVARCH))
$(info NLBUILDROOT=$(NLBUILDROOT))
$(info NLEXTERNALDIR=$(NLEXTERNALDIR))
$(info BUILDOUTPUTDIR=$(BUILDOUTPUTDIR))
$(info BIN_DIR=$(BIN_DIR))
$(info [VERSION])
$(info PRODUCT=$(VERSION_PRODUCT))
$(info RELEASE=$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MAINTENANCE).$(VERSION_PATCH) ($(VERSION_BUILD)))
$(info ---------------------------------------------------------------------------)


.PHONY: all
all: versionInfo $(TARGETS)

.PHONY: versionInfo
versionInfo:
	@if [ "$(RCSRC)" != "" ]; then \
		perl $(NLBUILDROOT)/build/updateVersionInfo_make.pl $(RCSRC) $(VERSION_MAJOR) $(VERSION_MINOR) $(VERSION_MAINTENANCE) $(VERSION_PATCH) "$(VERSION_BUILD)" "$(VERSION_PRODUCT)" $(TARGETENVARCH); \
		echo " --- Modified .rc file ---" ; \
		egrep "FILEVERSION|PRODUCTVERSION|CompanyName|FileDescription|FileVersion|LegalCopyright|ProductName|ProductVersion" $(RCSRC) ; \
	fi
