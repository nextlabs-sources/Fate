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
SOLUTION=msoPEP.sln
PROJECT=msoPEP.vcproj
RCSRC=src/msoPEP.rc
BIN_DIR=$(BUILDTYPE)_win_$(TARGETENVARCH)
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)

ifeq ($(TARGETENVARCH), x86)
	PROJECT_CONFIG_OUTLOOK2007=2K7$(BUILDTYPE)|Win32
	PROJECT_CONFIG_OUTLOOK2010=2K10$(BUILDTYPE)|Win32
	PROJECT_CONFIG_OUTLOOK2013=2K13$(BUILDTYPE)|Win32
	PROJECT_CONFIG_OUTLOOK2016=2K16$(BUILDTYPE)|Win32
	DLL_NAME_2007=mso2K7PEP32
	DLL_NAME_2010=mso2010PEP32
	DLL_NAME_2013=mso2013PEP32
	DLL_NAME_2016=mso2016PEP32
	TARGETS=outlook2010 outlook2013 outlook2016
else
	PROJECT_CONFIG_OUTLOOK2007=2K7$(BUILDTYPE)|$(TARGETENVARCH)
	PROJECT_CONFIG_OUTLOOK2010=2K10$(BUILDTYPE)|$(TARGETENVARCH)
	PROJECT_CONFIG_OUTLOOK2013=2K13$(BUILDTYPE)|$(TARGETENVARCH)
	PROJECT_CONFIG_OUTLOOK2016=2K16$(BUILDTYPE)|$(TARGETENVARCH)
	DLL_NAME_2007=mso2K7PEP
	DLL_NAME_2010=mso2010PEP
	DLL_NAME_2013=mso2013PEP
	DLL_NAME_2016=mso2016PEP
	TARGETS=outlook2010 outlook2013 outlook2016
endif

include $(NLBUILDROOT)/build/Makefile.ver

VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

include $(NLBUILDROOT)/build/Makefile.vs

BUILD_LOGFILE_OUTLOOK2007=outlook2007_$(BUILDTYPE)_$(TARGETENVARCH)_build.log
BUILD_LOGFILE_OUTLOOK2010=outlook2010_$(BUILDTYPE)_$(TARGETENVARCH)_build.log
BUILD_LOGFILE_OUTLOOK2013=outlook2013_$(BUILDTYPE)_$(TARGETENVARCH)_build.log
BUILD_LOGFILE_OUTLOOK2016=outlook2016_$(BUILDTYPE)_$(TARGETENVARCH)_build.log


#
# Targets
#

.PHONY: all
all: $(TARGETS)

.PHONY: outlook2007
outlook2007:
	@if [ "$(TARGETENVARCH)" == "x86" ]; then												\
		echo ""	;																			\
		echo "Building $(PROJECT) for Outlook 2007"	;										\
		$(MSVSIDE) $(SOLUTION) /build "$(PROJECT_CONFIG_OUTLOOK2007)" /project $(PROJECT) /out $(BUILD_LOGFILE_OUTLOOK2007) ; \
		COMPILE_STATUS=$$? ;																\
		if [ -f $(BUILD_LOGFILE_OUTLOOK2007) ] ; then										\
			echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2007)]]" ;							\
			cat $(BUILD_LOGFILE_OUTLOOK2007) ;												\
			echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2007)]]" ;							\
		else																				\
			echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2007)" ;							\
		fi ;																				\
		if [ $$COMPILE_STATUS -ne 0 ]; then													\
			exit $$COMPILE_STATUS ;															\
		fi ;																				\
		if [ $(OFFICIALCERT) -ne 0 ]; then													\
			echo "outlook2007: $(SIGNTOOL_OFFICIAL) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll" ; \
			$(SIGNTOOL_OFFICIAL) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll ;				\
		else																				\
			echo "outlook2007: $(SIGNTOOL_DEBUG) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll" ; \
			$(SIGNTOOL_DEBUG) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll ;					\
		fi ;																				\
		if [ ! -d $(BUILDOUTPUTDIR) ]; then													\
			mkdir -p $(BUILDOUTPUTDIR) ;													\
		fi ;																				\
		cp -f outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll $(BUILDOUTPUTDIR) ;				\
		cp -f outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).lib $(BUILDOUTPUTDIR) ;				\
		cp -f outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).pdb $(BUILDOUTPUTDIR) ;				\
	else																					\
		echo "x64 not supported on target outlook2007." ;									\
	fi
	
.PHONY: outlook2010
outlook2010:
	@echo ""
	@echo "Building $(PROJECT) for Outlook 2010"
	$(MSVSIDE) $(SOLUTION) /build "$(PROJECT_CONFIG_OUTLOOK2010)" /project $(PROJECT) /out $(BUILD_LOGFILE_OUTLOOK2010) ; \
	COMPILE_STATUS=$$? ;												\
	if [ -f $(BUILD_LOGFILE_OUTLOOK2010) ] ; then						\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2010)]]" ;			\
		cat $(BUILD_LOGFILE_OUTLOOK2010) ;								\
		echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2010)]]" ;			\
	else																\
		echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2010)" ;			\
	fi ;																\
	exit $$COMPILE_STATUS
	@if [ $(OFFICIALCERT) -ne 0 ]; then									\
		echo "outlook2010: $(SIGNTOOL_OFFICIAL) outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).dll" ; \
		$(SIGNTOOL_OFFICIAL) outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).dll ; \
	else																\
		echo "outlook2010: $(SIGNTOOL_DEBUG) outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).dll" ; \
		$(SIGNTOOL_DEBUG) outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).dll ;	\
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then								\
		mkdir -p $(BUILDOUTPUTDIR) ;									\
	fi
	cp -f outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).dll $(BUILDOUTPUTDIR)
	cp -f outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).lib $(BUILDOUTPUTDIR)
	cp -f outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).pdb $(BUILDOUTPUTDIR)

.PHONY: outlook2013
outlook2013:
	echo ""	;																			\
	echo "Building $(PROJECT) for Outlook 2013"	;										\
	$(MSVSIDE) $(SOLUTION) /build "$(PROJECT_CONFIG_OUTLOOK2013)" /project $(PROJECT) /out $(BUILD_LOGFILE_OUTLOOK2013) ; \
	COMPILE_STATUS=$$? ;																\
	if [ -f $(BUILD_LOGFILE_OUTLOOK2013) ] ; then										\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2013)]]" ;							\
		cat $(BUILD_LOGFILE_OUTLOOK2013) ;												\
		echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2013)]]" ;							\
	else																				\
		echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2013)" ;							\
	fi ;																				\
	if [ $$COMPILE_STATUS -ne 0 ]; then													\
		exit $$COMPILE_STATUS ;															\
	fi ;																				\
	if [ $(OFFICIALCERT) -ne 0 ]; then													\
		echo "outlook2013: $(SIGNTOOL_OFFICIAL) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll" ; \
		$(SIGNTOOL_OFFICIAL) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll ;				\
	else																				\
		echo "outlook2013: $(SIGNTOOL_DEBUG) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll" ; \
		$(SIGNTOOL_DEBUG) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll ;				\
	fi ;																				\
	if [ ! -d $(BUILDOUTPUTDIR) ]; then													\
		mkdir -p $(BUILDOUTPUTDIR) ;													\
	fi ;																				\
	cp -f outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll $(BUILDOUTPUTDIR) ;				\
	cp -f outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).lib $(BUILDOUTPUTDIR) ;				\
	cp -f outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).pdb $(BUILDOUTPUTDIR) ;				\

outlook2016:
	echo ""	;																			\
	echo "Building $(PROJECT) for Outlook 2016"	;										\
	$(MSVSIDE) $(SOLUTION) /build "$(PROJECT_CONFIG_OUTLOOK2016)" /project $(PROJECT) /out $(BUILD_LOGFILE_OUTLOOK2016) ; \
	COMPILE_STATUS=$$? ;																\
	if [ -f $(BUILD_LOGFILE_OUTLOOK2016) ] ; then										\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2016)]]" ;							\
		cat $(BUILD_LOGFILE_OUTLOOK2016) ;												\
		echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2016)]]" ;							\
	else																				\
		echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2016)" ;							\
	fi ;																				\
	if [ $$COMPILE_STATUS -ne 0 ]; then													\
		exit $$COMPILE_STATUS ;															\
	fi ;																				\
	if [ $(OFFICIALCERT) -ne 0 ]; then													\
		echo "OUTLOOK2016: $(SIGNTOOL_OFFICIAL) OUTLOOK2016_$(BIN_DIR)/$(DLL_NAME_2016).dll" ; \
		$(SIGNTOOL_OFFICIAL) OUTLOOK2016_$(BIN_DIR)/$(DLL_NAME_2016).dll ;				\
	else																				\
		echo "OUTLOOK2016: $(SIGNTOOL_DEBUG) OUTLOOK2016_$(BIN_DIR)/$(DLL_NAME_2016).dll" ; \
		$(SIGNTOOL_DEBUG) OUTLOOK2016_$(BIN_DIR)/$(DLL_NAME_2016).dll ;				\
	fi ;																				\
	if [ ! -d $(BUILDOUTPUTDIR) ]; then													\
		mkdir -p $(BUILDOUTPUTDIR) ;													\
	fi ;																				\
	cp -f OUTLOOK2016_$(BIN_DIR)/$(DLL_NAME_2016).dll $(BUILDOUTPUTDIR) ;				\
	cp -f OUTLOOK2016_$(BIN_DIR)/$(DLL_NAME_2016).lib $(BUILDOUTPUTDIR) ;				\
	cp -f OUTLOOK2016_$(BIN_DIR)/$(DLL_NAME_2016).pdb $(BUILDOUTPUTDIR) ;				\

.PHONY: clean
clean:
	@rm -rf outlook2007_release_win_x* outlook2007_debug_win_x* \
		outlook2010_release_win_x* outlook2010_debug_win_x* \
		outlook2013_release_win_x* outlook2013_debug_win_x* \
		outlook2016_release_win_x* outlook2016_debug_win_x* \
		outlook20*build.log