# Universal master makefile for all modules 
# by Dominic Lam

# Use defaults when build environment is not explicitly specified.

ifeq ($(ProgramW6432), )
	ENV_OS=x86
	PROGRAM_FILES_X86=C:/Program Files
else
	ENV_OS=x64
	PROGRAM_FILES_X86=C:/Program Files (x86)
endif

DOXYGEN=$(PROGRAM_FILES_X86)/doxygen/bin/doxygen.exe

OFFICIALCERT=0

#SIGNTOOL_OFFICIAL_TOOL=$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe
SIGNTOOL_OFFICIAL_TOOL=echo
SIGNTOOL_OFFICIAL_ARGS=sign /ac c:/release/bin/DigiCertAssuredIDRootCA.cer /f c:/release/bin/NextLabs.pfx /p IiVf1itvOrqJ /n "NextLabs Inc." /fd sha256 /tr http://timestamp.digicert.com
SIGNTOOL_OFFICIAL='$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS)

ifeq ($(P4ROOT), )
	P4ROOT = c:/dev/p4
endif

ifneq ($(NLENFORCERSDIR), )
	NLBUILDROOT = $(NLENFORCERSDIR)
endif

# Enforcers platform directory
ifeq ($(NLPLATFORMSSDIR), )
	export NLPLATFORMSDIR = $(NLBUILDROOT)/platforms
endif

# External directory
ifeq ($(NLEXTERNALDIR), )
	ifeq ($(EXTERNALSDIR), )
		ifeq ($(arg2), )
			export NLEXTERNALDIR = $(P4ROOT)/$(P4CLIENT)/main/external
		else
			export NLEXTERNALDIR = $(arg2)
		endif
	else
		export NLEXTERNALDIR = $(EXTERNALSDIR)
	endif
endif

# Enforcers platform directory
ifeq ($(JDKDIR), )
	JDKDIR = $(JAVA_HOME)
endif

# Product to build
ifeq ($(PRODUCT), )
else
	include $(NLBUILDROOT)/prods/$(PRODUCT)/env.$(PRODUCT)
endif

# TARGETENV, TARGETENVOS and TARGETENVARCH defines
TARGETENVOS   = $(shell echo ${TARGETENV} | cut -d_ -f1)
TARGETENVARCH = $(shell echo ${TARGETENV} | cut -d_ -f2)
PREFAST=1

# Including the OS dependent environment 
OS_NAME_MAP = ";Darwin=Darwin;CYGWIN_NT-5.1=CYGWIN_NT-5.1;CYGWIN_NT-6.1=CYGWIN_NT-5.1;CYGWIN_NT-5.2=CYGWIN_NT-5.1;CYGWIN_NT-5.2-WOW64=CYGWIN_NT-5.2-WOW64;Linux=Linux;CYGWIN_NT-6.1-WOW64=CYGWIN_NT-5.1;"
UNAME = $(shell uname -s)
OS = $(shell echo ${OS_NAME_MAP} | sed 's/.*;${UNAME}=\([^;]*\);.*/\1/')

# Default binary target version information
include $(NLBUILDROOT)/build/config/version

ifeq ($(VERSION_BUILD), )
	VERSION_BUILD=$(shell date +"%y.%j.%H%M")DX-$(HOSTNAME)-$(USERNAME)-$(shell date +"%Y.%m.%d-%H:%M")
endif

# Default to not build static subtarget (library)	
SUBTARGET_STATIC = no

# By default, use //depot/dev/build/config/use_default.rc. RCSRC in Makefile.inc overrides 
# this value.
RCSRC = $(SRCDIR)/use_default.rc

# $(ASMOBJS) should list all pre-compiled objects (including relative path and file extension).
# ASMOBJS in Makefile.inc overrides this value.
ASMOBJS =

# FORAPP may be overriden in Makefile.inc. Currently, FORAPP is used only to prefix local build 
# output directory at the moment to avoid clobbering of object and other intermediate files. 
# It is mainly used to handle a project that uses the same code but different compiler directives 
# to build binaries for different versions of Office or Outlook.
FORAPP = 

# Always include Makefile.inc, and first to take precedence
include $(NLBUILDROOT)/build/Makefile.ver
include Makefile.inc

include $(NLBUILDROOT)/build/env.CYGWIN_NT-5.1
MYHOST=${shell hostname}
sinclude c:/env.${MYHOST}

ifeq ($(BUILDTYPE), release)
	ifeq ($(FORAPP),)
		BINDIR		= release_${BASE}
		LIBDIR		= $(BINDIR)
		BINDIR_DIST	= $(BINDIR)
		LIBDIR_DIST	= $(BINDIR_DIST)
	else	
		BINDIR		= $(FORAPP)_release_${BASE}
		LIBDIR		= $(BINDIR)
		BINDIR_DIST	= release_${BASE}
		LIBDIR_DIST	= $(BINDIR_DIST)
	endif
else
	BUILDTYPE = debug

	ifeq ($(FORAPP),)
		BINDIR		= debug_${BASE}
		LIBDIR		= $(BINDIR)
		BINDIR_DIST	= $(BINDIR)
		LIBDIR_DIST	= $(BINDIR_DIST)
	else	
		BINDIR		= $(FORAPP)_debug_${BASE}
		LIBDIR		= $(BINDIR)
		BINDIR_DIST	= debug_${BASE}
		LIBDIR_DIST	= $(BINDIR_DIST)
	endif
endif

INCDIR    = include

# Phony target to make sure they will get built
.PHONY: subdirs $(SUBDIRS)
.PHONY: versionInfo
.PHONY: clean
.PHONY: $(TARGET)

# Go to subdirectory and do recursive make 
# ifneq ($(strip $(SUBDIRS)),)
# subdirs: $(SUBDIRS)
#   for dir in $(SUBDIRS); do $(MAKE) -C $$dir; done
# endif

OBJS     = $(addprefix $(BINDIR)/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

RCOBJS   = $(addprefix $(BINDIR)/, $(addsuffix .res, $(basename $(notdir $(RCSRC)))))

INCP     = $(addprefix -I, $(INCPATH))

TESTOBJS = $(addprefix $(TESTDIR)/$(BINDIR)/, $(addsuffix .o, $(basename $(notdir $(TESTSRC)))))

IDLSRC_HEADER	= $(subst .idl,.h,$(subst $(SRCDIR),$(INCDIR),$(IDL_SRC)))
IDLSRC_IID		= $(subst .idl,_i.c,$(IDL_SRC))
IDLSRC_PROXY	= $(subst .idl,_p.c,$(IDL_SRC))
IDLSRC_DLLDATA	= $(SRCDIR)/dlldata.c

# Computing the files to be cleaned
#
# MIDL must be run before code compile, IDL target must comes before .o targets.
CLEANFILES += *release_win_x86 *debug_win_x86 *release_win_x64 *debug_win_x64 \
	*Release_win_x86 *Debug_win_x86 *Release_win_x64 *Debug_win_x64 Debug Release \
	*.pdb *.ncb *.suo

ifneq ($(IDL_SRC), )
  CLEANFILES += $(IDLGENDIR) $(IDLSRC_HEADER) $(IDLSRC_IID) $(IDLSRC_PROXY) $(IDLSRC_DLLDATA)
  TARGETS    += $(subst .idl,.tlb,$(subst $(SRCDIR),$(BINDIR),$(IDL_SRC)))
endif

CLEANFILES   += $(OBJS) $(TESTOBJS)

ifneq ($(TARGET), )
  CLEANFILES += $(BINDIR)
  TARGETS    += $(TARGET)
endif

ifneq ($(SUBTARGET), )
  CLEANFILES += $(BINDIR)
  TARGETS    += $(SUBTARGET)
endif

ifneq ($(TESTTARGET), )
  CLEANFILES += $(TESTDIR)/$(BINDIR)
  TARGETS    += $(TESTTARGET)
endif

ifneq ($(TESTSUBTARGET), )
  CLEANFILES += $(TESTDIR)/$(BINDIR)
  TARGETS    += $(TESTSUBTARGET)
endif

ifneq ($(CUSTOM), )
  CUSTOMTARGETS  += $(CUSTOM)
endif

# Rule for building C files
$(BINDIR)/%.o: $(SRCDIR)/%.c
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	$(CC) $(CFLAGS) $(INCP) $(DFLAGS) $(CFLAGS2) -c $< $(CCOUTPUTFLAG)$@

# Rule for building C++ files
$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	$(CXX) $(CXXFLAGS) $(INCP) $(DFLAGS) $(CFLAGS2) -c $< $(CCOUTPUTFLAG)$@

$(BINDIR)/%.o: $(OTHERSRC_DIR)/%.cpp
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	$(CXX) $(CXXFLAGS) $(INCP) $(DFLAGS) $(CFLAGS2) -c $< $(CCOUTPUTFLAG)$@

# Rule for building the resources
$(BINDIR)/%.res: $(SRCDIR)/%.rc
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	$(RC) $(RCFLAGS) $(RCFLAGS2) $(INCP) $(CCOUTPUTFLAG)$@ $<

# Rule for building MIDL files
$(BINDIR)/%.tlb $(INCDIR)/%.h $(SRCDIR)/%_i.c $(SRCDIR)/%_p.c: $(SRCDIR)/%.idl
	@if [ ! -d $(IDLGENDIR) ] ; then \
	  mkdir -p $(IDLGENDIR) ; \
	fi
	$(MIDL) $(MIDL_CFLAGS) $(MIDL_DFLAGS) $(INCP) /out $(IDLGENDIR) $<
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	cp -f $(IDLGENDIR)/*.h $(INCDIR)
	cp -f $(IDLGENDIR)/*.c $(SRCDIR)
	cp -f $(IDLGENDIR)/*.tlb $(BINDIR)

# Rule for building test files
$(TESTDIR)/$(BINDIR)%.o: $(TESTDIR)/%.c
	@if [ ! -d $(TESTDIR)/$(BINDIR) ] ; then \
	  mkdir -p $(TESTDIR)/$(BINDIR) ; \
	fi
	$(CC) $(CFLAGS) $(INCP) $(DFLAGS) $(CFLAGS2) -c $< $(CCOUTPUTFLAG)$@

$(TESTDIR)/$(BINDIR)%.o: $(TESTDIR)/%.cpp
	@if [ ! -d $(TESTDIR)/$(BINDIR) ] ; then \
	  mkdir -p $(TESTDIR)/$(BINDIR) ; \
	fi
	$(CXX) $(CXXFLAGS) $(INCP) $(DFLAGS) $(CFLAGS2) -c $< $(CCOUTPUTFLAG)$@

# Rule for building Visual Studio solutions
%.sln: .
	@echo Building solution $@
	@cmd /C "devenv $@ /build $(BUILDTYPE)"

# Rule for building Visual Studio projects
%.vcproj: .
	@echo "Building project $@ ($(BUILDTYPE)|$(TARGETENVARCH))"
	cmd /C "$(NLBUILDROOT)/build/devenv_wrapper $@ $(BUILDTYPE) $(TARGETENVARCH)"

BUILDOUTPUTDIR = $(NLBUILDROOT)/bin

##############################################################################
# Validation rules for build parameters
##############################################################################
ifeq ($(BUILDTYPE),)
  $(error BUILDTYPE undefined. It must have a value of "release" or "debug".)
endif

ifeq ($(TARGETENVARCH),)
  $(error TARGETENVARCH undefined. It must have a value of "x86" or "x64".)
endif

ifeq ($(TARGETENVOS),)
  $(error TARGETENVOS undefined.)
endif

$(info --------------------------------------------------------------------------)
$(info [Target])
$(info TARGET=$(TARGET))
$(info SUBTARGET=$(SUBTARGET))
$(info CUSTOM=$(CUSTOM))
$(info [Parameters])
$(info BUILDTYPE=$(BUILDTYPE))
$(info TARGETENVOS=$(TARGETENVOS))
$(info TARGETENVARCH=$(TARGETENVARCH))
$(info SUBTARGET_STATIC=$(SUBTARGET_STATIC))
$(info PREFAST=$(PREFAST))
$(info NLBUILDROOT=$(NLBUILDROOT))
$(info NLEXTERNALDIR=$(NLEXTERNALDIR))
$(info NLENFORCERSDIR=$(NLENFORCERSDIR) (legacy))
$(info JDKDIR=$(JDKDIR))
$(info BUILDOUTPUTDIR=$(BUILDOUTPUTDIR))
$(info BINDIR=$(BINDIR))
$(info LIBDIR=$(LIBDIR))
$(info BINDIR_DIST=$(BINDIR_DIST))
$(info LIBDIR_DIST=$(LIBDIR_DIST))
$(info LIBPATH=$(LIBPATH))
$(info [Tools])
$(info CC=$(CC))
$(info LD=$(LD))
$(info RC=$(RC))
$(info MIDL=$(MIDL))
$(info STATIC_LD=$(STATIC_LD))
$(info [VERSION])
$(info PRODUCT=$(VERSION_PRODUCT))
$(info RELEASE=$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MAINTENANCE).$(VERSION_PATCH) ($(VERSION_BUILD)))
$(info ---------------------------------------------------------------------------)

all: versionInfo $(TARGETS) $(CUSTOMTARGETS)

versionInfo:
	@if [ "$(RCSRC)" == "$(SRCDIR)/use_default.rc" ]; then \
		cp --force $(NLBUILDROOT)/build/config/use_default.rc $(RCSRC); \
		sed --in-place -e 's#VERSION_MAJOR#$(VERSION_MAJOR)#g' $(RCSRC); \
		sed --in-place -e 's#VERSION_MINOR#$(VERSION_MINOR)#g' $(RCSRC); \
		sed --in-place -e 's#VERSION_MAINTENANCE#$(VERSION_MAINTENANCE)#g' $(RCSRC); \
		sed --in-place -e 's#VERSION_PATCH#$(VERSION_PATCH)#g' $(RCSRC); \
		sed --in-place -e 's#VERSION_PRODUCT#$(VERSION_PRODUCT)#g' $(RCSRC); \
		sed --in-place -e 's#VERSION_BUILD#$(VERSION_BUILD)#g' $(RCSRC); \
		sed --in-place -e 's#VERSION_ARCHITECTURE#$(TARGETENVARCH)#g' $(RCSRC); \
	else \
		perl $(NLBUILDROOT)/build/updateVersionInfo_make.pl $(RCSRC) $(VERSION_MAJOR) $(VERSION_MINOR) $(VERSION_MAINTENANCE) $(VERSION_PATCH) "$(VERSION_BUILD)" "$(VERSION_PRODUCT)" $(TARGETENVARCH); \
	fi

# Rule for building C# files
ifneq ($(CSTARGET), )
all:$(CSTARGET)
$(CSTARGET): $(SRC) 
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	$(CSC) $(TARGET_TARGET) $(CSTARGET_DEBUG) $(REFERENCE) $(CSFLAGS) $(CSLIB) $(CSOUTPUT)$(BINDIR)/$(CSTARGET)$(TARGET_EXT) $(SRC)
	@if [ $(OFFICIALCERT) -eq 0 ]; then \
		echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(BINDIR)/$(CSTARGET)$(TARGET_EXT)" ; \
		'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(BINDIR)/$(CSTARGET)$(TARGET_EXT) ; \
	else \
		echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BINDIR)/$(CSTARGET)$(TARGET_EXT)" ; \
		'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BINDIR)/$(CSTARGET)$(TARGET_EXT) ; \
	fi
	@if [ ! -d ${BUILDOUTPUTDIR}/$(BINDIR_DIST) ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(BINDIR_DIST) ; \
	fi
	cp -f $(BINDIR)/$(CSTARGET)$(TARGET_EXT) ${BUILDOUTPUTDIR}/$(BINDIR_DIST)
	@if [ $(SUBTARGET_DEBUG). != . ] ; then \
	  cp -f $(BINDIR)/$(CSTARGET).$(SUBTARGET_DEBUG) ${BUILDOUTPUTDIR}/$(BINDIR_DIST) ; \
	fi         
endif

ifneq ($(CSSUBTARGET), )
all: $(CSSUBTARGET)
$(CSSUBTARGET): $(SRC)
	@if [ ! -d $(LIBDIR) ] ; then \
	  mkdir -p $(LIBDIR) ; \
	fi
	$(CSC) $(SUBTARGET_TARGET) $(CSTARGET_DEBUG) $(REFERENCE) $(CSFLAGS) $(CSLIB) $(CSOUTPUT)$(LIBDIR)/$(CSSUBTARGET).$(SUBTARGET_EXT) $(SRC)

	@if [ $(BUILDTYPE) == "release" ] ; then \
	  if [ $(STRIP). != . ] ; then \
	    $(STRIP) $(SFLAGS) $(LIBDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ; \
	  fi \
	fi
	@if [ $(OFFICIALCERT) -eq 0 ]; then \
		echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(LIBDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT)" ; \
		'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(LIBDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ; \
	else \
		echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(LIBDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT)" ; \
		'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(LIBDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ; \
	fi
	@if [ ! -d ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(LIBDIR_DIST)  ; \
	fi
	cp -f $(LIBDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) 
	@if [ $(SUBTARGET_DEBUG). != . ] ; then \
	  cp -f $(LIBDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_DEBUG) ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ; \
	fi       
	cp -f $(LIBDIR)/*.$(SUBTARGET_EXT) ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ; \

endif

# Rule for building Target executables
# Manifest line is very windows specific, but it's the best place for it
$(TARGET): $(OBJS) $(ASMOBJS) $(RCOBJS)
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	$(LD) $(LDFLAGS) $^ $(LDOUTPUTFLAG)$(BINDIR)/$@$(TARGET_EXT) $(LIBP) $(RUNP) $(LIBRARIES)

	@if [ $(BUILDTYPE) == "release" ] ; then \
	  if [ $(STRIP). != . ] ; then \
	    $(STRIP) $(SFLAGS) $(BINDIR)/$@ ; \
	  fi \
	fi
	@if [ ${MANIFEST}. != . ] ; then \
		$(WINSDK_ROOT)/bin/mt.exe -manifest ${MANIFEST} -outputresource:$(BINDIR)/$@$(TARGET_EXT)\;1; \
	fi
	@if [ $(OFFICIALCERT) -eq 0 ]; then \
		echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(BINDIR)/$@$(TARGET_EXT)" ; \
		'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(BINDIR)/$@$(TARGET_EXT) ; \
	else \
		echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BINDIR)/$@$(TARGET_EXT)" ; \
		'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BINDIR)/$@$(TARGET_EXT) ; \
	fi
	@if [ ! -d ${BUILDOUTPUTDIR}/$(BINDIR_DIST) ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(BINDIR_DIST) ; \
	fi
	cp -f $(BINDIR)/$@$(TARGET_EXT) ${BUILDOUTPUTDIR}/$(BINDIR_DIST)
	@if [ $(SUBTARGET_DEBUG). != . ] ; then \
	  cp -f $(BINDIR)/$@.$(SUBTARGET_DEBUG) ${BUILDOUTPUTDIR}/$(BINDIR_DIST) ; \
	fi \

# Rule for building subtarget share library
$(SUBTARGET): $(OBJS) $(ASMOBJS) $(RCOBJS) 
	-@if [ -f $(SUBTARGET).doxygen ] ; then \
	  "$(DOXYGEN)" $(SUBTARGET).doxygen ; \
	fi
	@if [ ! -d $(BINDIR) ] ; then \
	  mkdir -p $(BINDIR) ; \
	fi
	@if [ ! -d ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ; \
	fi
	$(LD) $(LDFLAGS) $(SUBTARGETFLAGS) $^ $(LDOUTPUTFLAG)$(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) $(LIBP) $(LIBRARIES)
	@if [ $(SUBTARGET_STATIC) == "yes" ] ; then \
	  $(STATIC_LD) $(STATIC_LDFLAGS) $(OBJS) $(LDOUTPUTFLAG)$(BINDIR)/$(SUBTARGET)_static$(SUBTARGET_SUFFIX).lib ; \
	  cp -f $(BINDIR)/$(SUBTARGET)_static$(SUBTARGET_SUFFIX).lib ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ; \
	fi
	@if [ $(BUILDTYPE) == "release" ] ; then \
	  if [ $(STRIP). != . ] ; then \
	    $(STRIP) $(SFLAGS) $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ; \
	  fi \
	fi
	@if [ ${MANIFEST}. != . ] ; then \
		$(WINSDK_ROOT)/bin/mt.exe -manifest ${MANIFEST} -outputresource:$(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT)\;2; \
	fi
	@if [ $(OFFICIALCERT) -eq 0 ]; then \
		echo "'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT)" ; \
		'${SIGNTOOL_DEV}' ${SIGNTOOL_DEV_ARGS} $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ; \
	else \
		echo "'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT)" ; \
		'$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS) $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ; \
	fi
	@if [ ! -d ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(LIBDIR_DIST)  ; \
	fi
	cp -f $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_EXT) ${BUILDOUTPUTDIR}/$(LIBDIR_DIST)  
	@if [ $(SUBTARGET_LIB). != . ] ; then \
	  cp -f $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_LIB) ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ; \
	fi
	@if [ $(SUBTARGET_DEBUG). != . ] ; then \
	  cp -f $(BINDIR)/$(SUBTARGET_PREFIX)$@$(SUBTARGET_SUFFIX).$(SUBTARGET_DEBUG) ${BUILDOUTPUTDIR}/$(LIBDIR_DIST) ; \
	fi  

$(TESTTARGET): $(TESTOBJS)
	@if [ ! -d $(TESTDIR)/$(BINDIR) ] ; then \
	  mkdir -p $(TESTDIR)/$(BINDIR) ; \
	fi
	$(LD) $(LDFLAGS) $^ $(LDOUTPUTFLAG)$(TESTDIR)/$(BINDIR)/$@$(TARGET_EXT) $(TESTLIBP) $(LIBP) $(TESTRUNP) $(TESTLIBRARIES)

$(TESTSUBTARGET): $(TESTOBJS)
	@if [ ! -d $(TESTDIR)/$(BINDIR) ] ; then \
	  mkdir -p $(TESTDIR)/$(BINDIR) ; \
	fi
	$(LD) $(LDFLAGS)  $(TESTSUBTARGETFLAGS) $^ $(LDOUTPUTFLAG)$(TESTDIR)/$(BINDIR)/$@.$(SUBTARGET_EXT) $(TESTLIBP) $(LIBP) $(TESTLIBRARIES)

# Empty rules for dynamically generated files to avoid the following error.
#	c1 : fatal error C1083: Cannot open source file: 'src/dlldata.c': No such file or directory
$(IDLSRC_IID):

$(IDLSRC_PROXY):

$(IDLSRC_DLLDATA):

# Rule for cleaning up
clean:
	@rm -fr $(CLEANFILES)
