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


#
# Variables
#

PROJECT=OfficeTagLib.csproj
BIN_DIR=$(BUILDTYPE)_win_$(TARGETENVARCH)
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)
DLL_NAME=OfficeTagLib
TARGETS=OfficeTagLib
DLL_PATH_NAME=bin/$(TARGETENVARCH)/$(BUILDTYPE)/$(DLL_NAME).dll
ifeq ($(TARGETENVARCH), x86)
	PROJECT_CONFIG=$(BUILDTYPE)|x86
else
	PROJECT_CONFIG=$(BUILDTYPE)|x64
endif

include $(NLBUILDROOT)/build/Makefile.ver

VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

include $(NLBUILDROOT)/build/Makefile.vs

BUILD_LOGFILE=$(BUILDTYPE)_$(TARGETENVARCH)_build.log

#
# Targets
#

.PHONY: all
all: $(TARGETS)

.PHONY: OfficeTagLib
OfficeTagLib:
	echo "Building $(PROJECT)..."	;									\
	$(MSVSIDE) $(PROJECT) /build "$(PROJECT_CONFIG)" /out $(BUILD_LOGFILE) ;                                \
	COMPILE_STATUS=$$?;                                                                                     \
	if [ -f $(BUILD_LOGFILE) ] ; then									\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE)]]" ;							\
		cat $(BUILD_LOGFILE) ;										\
		echo "[[DUMP END - $(BUILD_LOGFILE)]]" ;							\
	else													\
		echo "INFO: Cannot find $(BUILD_LOGFILE)" ;							\
	fi;													\
	if [ $$COMPILE_STATUS -ne 0 ]; then							    	  	\
		exit $$COMPILE_STATUS;										\
	fi;													\
	if [ $(OFFICIALCERT) -ne 0 ]; then									\
		echo "$(SIGNTOOL_OFFICIAL) $(DLL_PATH_NAME)" ; 							\
		$(SIGNTOOL_OFFICIAL) $(DLL_PATH_NAME);								\
	else													\
		echo "$(SIGNTOOL_DEBUG) $(DLL_PATH_NAME)" ; 							\
		$(SIGNTOOL_DEBUG) $(DLL_PATH_NAME) ;								\
	fi ;													\
	if [ ! -d $(BUILDOUTPUTDIR) ]; then									\
		mkdir -p $(BUILDOUTPUTDIR) ;									\
	fi ;													\
	cp -f $(DLL_PATH_NAME) $(BUILDOUTPUTDIR);								\
	cp -f bin/$(TARGETENVARCH)/$(BUILDTYPE)/$(DLL_NAME).pdb $(BUILDOUTPUTDIR) ;				\

.PHONY: clean
clean:
	@rm -rf bin
	@rm -rf obj