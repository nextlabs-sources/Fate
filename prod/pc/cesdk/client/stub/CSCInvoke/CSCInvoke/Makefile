# Notes: To each Visual Studio compilation output. We chain a few lines of scripts
# together. In doing so, Make executes all of them in one process. We want the exit 
# status of this process to be the exit status of code compilation. So we add this 
# line of code at the end. Note that there is no line continuation character "\" at
# the end of this line because this is the end of a chain of commands to be executed
# in one shell process. However, the line before this should have a "\".
#		exit $$COMPILE_STATUS
#
# On the other hand, if we put MSVC call within a if statement where the whole compilation
# binary signing and copying to distribution directory are all in one line. That means
# the command "exit $$COMPILE_STATUS" cannot be the last commnand to be executed in a
# process. In this case, we need to check exit status before calling "exit $$COMPILE_STATUS".
#		if [ $$COMPILE_STATUS -ne 0 ]; then \
#			exit $$COMPILE_STATUS ; \
#		fi ; \
#

ifeq ($(ProgramW6432), )
	PROGRAM_FILES_X86=C:/Program Files
else
	PROGRAM_FILES_X86=C:/Program Files (x86)
endif

ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

NLBUILDROOT_DOS=$(subst /,\\,$(NLBUILDROOT))

OFFICIALCERT=0
# SIGNTOOL_OFFICIAL_TOOL=$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe
SIGNTOOL_OFFICIAL_TOOL=echo
SIGNTOOL_OFFICIAL_ARGS=sign /ac c:/release/bin/DigiCertAssuredIDRootCA.cer /f c:/release/bin/NextLabs.pfx /p IiVf1itvOrqJ /n "NextLabs Inc." /fd sha256 /tr http://timestamp.digicert.com
SIGNTOOL_OFFICIAL='$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS)
SIGNTOOL_DEBUG='$(SIGNTOOL_OFFICIAL_TOOL)' sign /f "$(NLBUILDROOT_DOS)\\build\\certificates\\NextLabsDebug.pfx" /p 123blue! /n "NextLabsDebug" /fd sha256
MSVSIDE=x:/common7/IDE/devenv.exe
PROJECT=CSCInvoke.csproj

ifeq ($(TARGETENVARCH),)
	TARGETENVARCH=x86
endif

ifneq ($(BUILDTYPE), release)
	BUILDTYPE=debug
endif

# IMPORTANT: Temporary send NextLabs.CSCInvoke.dll to debug_win_x64 and release_win_x64 
# to avoid crashing with the version produced by SPE for now.
BIN_DIR=$(BUILDTYPE)_dotnet2
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)
PROJECT_CONFIG=$(BUILDTYPE)|AnyCPU
BUILD_LOGFILE=$(BUILDTYPE)_build.log


$(info --------------------------------------------------------------------------)
$(info [Project])
$(info PROJECT=$(PROJECT))
$(info [Parameters])
$(info BUILDTYPE=$(BUILDTYPE))
$(info NLBUILDROOT=$(NLBUILDROOT))
$(info NLEXTERNALDIR=$(NLEXTERNALDIR))
$(info BUILDOUTPUTDIR=$(BUILDOUTPUTDIR))
$(info BIN_DIR=$(BIN_DIR))
$(info ---------------------------------------------------------------------------)


.PHONY: all
.PHONY: clean

all:
	@echo ""
	@echo "Building $(PROJECT) ($(BUILDTYPE)) for NextLabs SharePoint Enforcer"
	rm -f $(BUILD_LOGFILE)
	$(MSVSIDE) $(PROJECT) /build "$(PROJECT_CONFIG)" /out $(BUILD_LOGFILE) ; \
	COMPILE_STATUS=$$? ; \
	if [ -f $(BUILD_LOGFILE) ] ; then \
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE)]]" ; \
		cat $(BUILD_LOGFILE) ; \
		echo "[[DUMP END - $(BUILD_LOGFILE)]]" ; \
	else \
		echo "INFO: Cannot find $(BUILD_LOGFILE)" ; \
	fi ; \
	exit $$COMPILE_STATUS
	@if [ $(OFFICIALCERT) -ne 0 ]; then \
		echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BUILDTYPE)/NextLabs.CSCInvoke.dll" ; \
		'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BUILDTYPE)/NextLabs.CSCInvoke.dll ; \
	else \
		echo "$(SIGNTOOL_DEBUG) $(BUILDTYPE)/NextLabs.CSCInvoke.dll" ; \
		$(SIGNTOOL_DEBUG) $(BUILDTYPE)/NextLabs.CSCInvoke.dll ; \
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then \
		mkdir -p $(BUILDOUTPUTDIR) ; \
	fi
	cp -f $(BUILDTYPE)/NextLabs.CSCInvoke.dll $(BUILDOUTPUTDIR)
	cp -f $(BUILDTYPE)/NextLabs.CSCInvoke.pdb $(BUILDOUTPUTDIR)

clean:
	@if [ -e ./Makefile.inc ]; then \
		rm -rf release debug obj* *.suo *.ncb *.pdb $(BUILDOUTPUTDIR)/NextLabs.CSCInvoke.dll $(BUILDOUTPUTDIR)/NextLabs.CSCInvoke.pdb ; \
	fi
