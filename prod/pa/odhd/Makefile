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
include $(NLBUILDROOT)/build/Makefile.ver

VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

include $(NLBUILDROOT)/build/Makefile.vs

PROJECT=odhd.vcproj
RCSRC=src/ohdui.rc
BIN_DIR=$(BUILDTYPE)_win_$(TARGETENVARCH)
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)
DISTHEADERDIR=$(NLBUILDROOT)/bin/include/odhd
TARGETS=dist_header outlook2003 outlook2007 outlook2010 outlook2013 outlook2016 

ifeq ($(TARGETENVARCH), x86)
	PROJECT_CONFIG_OUTLOOK2003=2K3$(BUILDTYPE)|Win32
	PROJECT_CONFIG_OUTLOOK2007=2K7$(BUILDTYPE)|Win32
	PROJECT_CONFIG_OUTLOOK2010=2010$(BUILDTYPE)|Win32
	PROJECT_CONFIG_OUTLOOK2013=2013$(BUILDTYPE)|Win32
	PROJECT_CONFIG_OUTLOOK2016=2016$(BUILDTYPE)|Win32
	DLL_NAME_2003=odhd2K332
	DLL_NAME_2007=odhd2K732
	DLL_NAME_2010=odhd201032
	DLL_NAME_2013=odhd201332
	DLL_NAME_2016=odhd201632
else
	PROJECT_CONFIG_OUTLOOK2003=2K3$(BUILDTYPE)|$(TARGETENVARCH)
	PROJECT_CONFIG_OUTLOOK2007=2K7$(BUILDTYPE)|$(TARGETENVARCH)
	PROJECT_CONFIG_OUTLOOK2010=2010$(BUILDTYPE)|$(TARGETENVARCH)
	PROJECT_CONFIG_OUTLOOK2013=2013$(BUILDTYPE)|$(TARGETENVARCH)
	PROJECT_CONFIG_OUTLOOK2016=2016$(BUILDTYPE)|$(TARGETENVARCH)
	DLL_NAME_2003=odhd2K3
	DLL_NAME_2007=odhd2K7
	DLL_NAME_2010=odhd2010
	DLL_NAME_2013=odhd2013
	DLL_NAME_2016=odhd2016
endif

BUILD_LOGFILE_OUTLOOK2003=outlook2003_$(BUILDTYPE)_$(TARGETENVARCH)_build.log
BUILD_LOGFILE_OUTLOOK2007=outlook2007_$(BUILDTYPE)_$(TARGETENVARCH)_build.log
BUILD_LOGFILE_OUTLOOK2010=outlook2010_$(BUILDTYPE)_$(TARGETENVARCH)_build.log
BUILD_LOGFILE_OUTLOOK2013=outlook2013_$(BUILDTYPE)_$(TARGETENVARCH)_build.log
BUILD_LOGFILE_OUTLOOK2016=outlook2016_$(BUILDTYPE)_$(TARGETENVARCH)_build.log

#
# Targets
#

.PHONY: all
all: $(TARGETS)

.PHONY: outlook2003
outlook2003:
	@echo ""
	@echo "Building $(PROJECT) for Outlook 2003"
	$(MSVSIDE) $(PROJECT) /build "$(PROJECT_CONFIG_OUTLOOK2003)" /out $(BUILD_LOGFILE_OUTLOOK2003) ; \
	COMPILE_STATUS=$$? ;												\
	if [ -f $(BUILD_LOGFILE_OUTLOOK2003) ] ; then						\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2003)]]" ;			\
		cat $(BUILD_LOGFILE_OUTLOOK2003) ;								\
		echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2003)]]" ;			\
	else																\
		echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2003)" ;			\
	fi ;																\
	exit $$COMPILE_STATUS
	@if [ $(OFFICIALCERT) -ne 0 ]; then									\
		echo "outlook2003: $(SIGNTOOL_OFFICIAL) outlook2003_$(BIN_DIR)/$(DLL_NAME_2003).dll" ; \
		$(SIGNTOOL_OFFICIAL) outlook2003_$(BIN_DIR)/$(DLL_NAME_2003).dll ; \
	else																\
		echo "outlook2003: $(SIGNTOOL_DEBUG) outlook2003_$(BIN_DIR)/$(DLL_NAME_2003).dll" ; \
		$(SIGNTOOL_DEBUG) outlook2003_$(BIN_DIR)/$(DLL_NAME_2003).dll ; \
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then								\
		mkdir -p $(BUILDOUTPUTDIR) ;									\
	fi
	cp -f outlook2003_$(BIN_DIR)/$(DLL_NAME_2003).dll $(BUILDOUTPUTDIR)
	cp -f outlook2003_$(BIN_DIR)/$(DLL_NAME_2003).lib $(BUILDOUTPUTDIR)
	cp -f outlook2003_$(BIN_DIR)/$(DLL_NAME_2003).pdb $(BUILDOUTPUTDIR)

.PHONY: outlook2007
outlook2007:
	@echo ""
	@echo "Building $(PROJECT) for Outlook 2007"
	$(MSVSIDE) $(PROJECT) /build "$(PROJECT_CONFIG_OUTLOOK2007)" /out $(BUILD_LOGFILE_OUTLOOK2007) ; \
	COMPILE_STATUS=$$? ;												\
	if [ -f $(BUILD_LOGFILE_OUTLOOK2007) ] ; then						\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2007)]]" ;			\
		cat $(BUILD_LOGFILE_OUTLOOK2007) ;								\
		echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2007)]]" ;			\
	else																\
		echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2007)" ;			\
	fi ;																\
	exit $$COMPILE_STATUS
	@if [ $(OFFICIALCERT) -ne 0 ]; then									\
		echo "outlook2007: $(SIGNTOOL_OFFICIAL) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll" ; \
		$(SIGNTOOL_OFFICIAL) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll ; \
	else																\
		echo "outlook2007: $(SIGNTOOL_DEBUG) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll" ; \
		$(SIGNTOOL_DEBUG) outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll ; \
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then								\
		mkdir -p $(BUILDOUTPUTDIR) ;									\
	fi
	cp -f outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).dll $(BUILDOUTPUTDIR)
	cp -f outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).lib $(BUILDOUTPUTDIR)
	cp -f outlook2007_$(BIN_DIR)/$(DLL_NAME_2007).pdb $(BUILDOUTPUTDIR)

.PHONY: outlook2010
outlook2010:
	@echo ""
	@echo "Building $(PROJECT) for Outlook 2010"
	$(MSVSIDE) $(PROJECT) /build "$(PROJECT_CONFIG_OUTLOOK2010)" /out $(BUILD_LOGFILE_OUTLOOK2010) ; \
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
		$(SIGNTOOL_DEBUG) outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).dll ; \
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then								\
		mkdir -p $(BUILDOUTPUTDIR) ;									\
	fi
	cp -f outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).dll $(BUILDOUTPUTDIR)
	cp -f outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).lib $(BUILDOUTPUTDIR)
	cp -f outlook2010_$(BIN_DIR)/$(DLL_NAME_2010).pdb $(BUILDOUTPUTDIR)

.PHONY: outlook2013
outlook2013:
	@echo ""
	@echo "Building $(PROJECT) for Outlook 2013"
	$(MSVSIDE) $(PROJECT) /build "$(PROJECT_CONFIG_OUTLOOK2013)" /out $(BUILD_LOGFILE_OUTLOOK2013) ; \
	COMPILE_STATUS=$$? ;												\
	if [ -f $(BUILD_LOGFILE_OUTLOOK2013) ] ; then						\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2013)]]" ;			\
		cat $(BUILD_LOGFILE_OUTLOOK2013) ;								\
		echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2013)]]" ;			\
	else																\
		echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2013)" ;			\
	fi ;																\
	exit $$COMPILE_STATUS
	@if [ $(OFFICIALCERT) -ne 0 ]; then									\
		echo "outlook2013: $(SIGNTOOL_OFFICIAL) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll" ; \
		$(SIGNTOOL_OFFICIAL) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll ; \
	else																\
		echo "outlook2013: $(SIGNTOOL_DEBUG) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll" ; \
		$(SIGNTOOL_DEBUG) outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll ; \
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then								\
		mkdir -p $(BUILDOUTPUTDIR) ;									\
	fi
	cp -f outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).dll $(BUILDOUTPUTDIR)
	cp -f outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).lib $(BUILDOUTPUTDIR)
	cp -f outlook2013_$(BIN_DIR)/$(DLL_NAME_2013).pdb $(BUILDOUTPUTDIR)

.PHONY: outlook2016
outlook2016:
	@echo ""
	@echo "Building $(PROJECT) for Outlook 2016"
	$(MSVSIDE) $(PROJECT) /build "$(PROJECT_CONFIG_OUTLOOK2016)" /out $(BUILD_LOGFILE_OUTLOOK2016) ; \
	COMPILE_STATUS=$$? ;												\
	if [ -f $(BUILD_LOGFILE_OUTLOOK2016) ] ; then						\
		echo "[[DUMP BEGIN - $(BUILD_LOGFILE_OUTLOOK2016)]]" ;			\
		cat $(BUILD_LOGFILE_OUTLOOK2016) ;								\
		echo "[[DUMP END - $(BUILD_LOGFILE_OUTLOOK2016)]]" ;			\
	else																\
		echo "INFO: Cannot find $(BUILD_LOGFILE_OUTLOOK2016)" ;			\
	fi ;																\
	exit $$COMPILE_STATUS
	@if [ $(OFFICIALCERT) -ne 0 ]; then									\
		echo "outlook2016: $(SIGNTOOL_OFFICIAL) outlook2016_$(BIN_DIR)/$(DLL_NAME_2016).dll" ; \
		$(SIGNTOOL_OFFICIAL) outlook2016_$(BIN_DIR)/$(DLL_NAME_2016).dll ; \
	else																\
		echo "outlook2016: $(SIGNTOOL_DEBUG) outlook2016_$(BIN_DIR)/$(DLL_NAME_2016).dll" ; \
		$(SIGNTOOL_DEBUG) outlook2016_$(BIN_DIR)/$(DLL_NAME_2016).dll ; \
	fi
	@if [ ! -d $(BUILDOUTPUTDIR) ]; then								\
		mkdir -p $(BUILDOUTPUTDIR) ;									\
	fi
	cp -f outlook2016_$(BIN_DIR)/$(DLL_NAME_2016).dll $(BUILDOUTPUTDIR)
	cp -f outlook2016_$(BIN_DIR)/$(DLL_NAME_2016).lib $(BUILDOUTPUTDIR)
	cp -f outlook2016_$(BIN_DIR)/$(DLL_NAME_2016).pdb $(BUILDOUTPUTDIR)

dist_header:
	@if [ ! -d ${DISTHEADERDIR} ]; then							\
	  mkdir -p ${DISTHEADERDIR};								\
	fi
	cp -f include/basetype.h ${DISTHEADERDIR};
	cp -f include/odhd_inc.h ${DISTHEADERDIR};

.PHONY: clean
clean:
	@rm -rf outlook2003_release_win_x* outlook2003_debug_win_x* \
		outlook2007_release_win_x* outlook2007_debug_win_x* \
		outlook2010_release_win_x* outlook2010_debug_win_x* \
		outlook2013_release_win_x* outlook2013_debug_win_x* \
		outlook2016_release_win_x* outlook2016_debug_win_x*


