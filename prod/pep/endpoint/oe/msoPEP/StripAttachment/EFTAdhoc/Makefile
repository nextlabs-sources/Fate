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
#		if [ $$COMPILE_STATUS -ne 0 ]; then		\
#			exit $$COMPILE_STATUS ;				\
#		fi ;									\
#

ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

ifeq ($(BUILDTYPE),)
  $(error  BUILDTYPE undefined.)
endif


#
# Variables
#

PROJECT_FILE=EFTAdhoc.vcproj
RCSRC=EFTAdhoc.rc
BIN_DIR=$(BUILDTYPE)_win_$(TARGETENVARCH)
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)
TARGETS=compile

ifeq ($(TARGETENVARCH), x86)
	PROJECT_CONFIG=$(BUILDTYPE)|Win32
	DLL_NAME=EFTAdhoc32
	#DLL_NAME=EFTAdhoc
else
	PROJECT_CONFIG=$(BUILDTYPE)|$(TARGETENVARCH)
	DLL_NAME=EFTAdhoc
endif

# Makefile.vs contains target all which depends on $(TARGETS)
include $(NLBUILDROOT)/build/Makefile.vs

BUILD_LOGFILE=$(BUILDTYPE)_$(TARGETENVARCH)_build.log


$(info --------------------------------------------------------------------------)
$(info TARGETS=$(TARGETS))
$(info PROJECT_CONFIG=$(PROJECT_CONFIG))
$(info PROJECT_FILE=$(PROJECT_FILE))
$(info RCSRC=$(RCSRC))
$(info DLL_NAME=$(DLL_NAME))
$(info NLBUILDROOT=$(NLBUILDROOT))
$(info NLEXTERNALDIR=$(NLEXTERNALDIR))
$(info BUILDOUTPUTDIR=$(BUILDOUTPUTDIR))
$(info ---------------------------------------------------------------------------)


#
# Targets
#

.PHONY: compile
compile:
	@echo "Building $(PROJECT_FILE) ($(BUILDTYPE)) for $(PROJECT_NAME)"
	$(MSVSIDE) $(PROJECT_FILE) /build "$(PROJECT_CONFIG)" /out $(BUILD_LOGFILE) ; \
	COMPILE_STATUS=$$? ;									\
	if [ -f $(BUILD_LOGFILE) ] ; then						\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE)]]" ;			\
		cat $(BUILD_LOGFILE) ;								\
		echo "[[DUMP END - $(BUILD_LOGFILE)]]" ;			\
	else													\
		echo "INFO: Cannot find $(BUILD_LOGFILE)" ;			\
	fi ;													\
	exit $$COMPILE_STATUS
	@if [ $(OFFICIALCERT) -ne 0 ]; then								\
		$(SIGNTOOL_OFFICIAL) $(BIN_DIR)/$(DLL_NAME).dll ;			\
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then							\
		mkdir -p $(BUILDOUTPUTDIR) ;								\
	fi
	cp -f $(BIN_DIR)/$(DLL_NAME).dll $(BUILDOUTPUTDIR)
	cp -f $(BIN_DIR)/$(DLL_NAME).lib $(BUILDOUTPUTDIR)
	cp -f $(BIN_DIR)/$(DLL_NAME).pdb $(BUILDOUTPUTDIR)

.PHONY: clean
clean:
	@rm -rf release_win_x* Release_win_x* debug_win_x* Debug_win_x* *.suo *.ncb
