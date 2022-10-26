# Makefile to assist in compiling Java code


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
# Paths and tools
#

#JDK_DIR=$(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18
JDK_DIR=$(NLEXTERNALDIR)/j2sdk/jdk1.5.0_09
ANT_DIR=$(NLEXTERNALDIR)/apache-ant/apache-ant-1.7.0

JAVAC=$(JDK_DIR)/bin/javac.exe
JAR=$(JDK_DIR)/bin/jar.exe
JAVAH=$(JDK_DIR)/bin/javah.exe
ANT=$(ANT_DIR)/bin/ant
# SIGNTOOL_OFFICIAL_TOOL=$(PROGRAM_FILES_X86)/Windows Kits/8.0/bin/x64/signtool.exe
SIGNTOOL_OFFICIAL_TOOL=echo
SIGNTOOL_OFFICIAL_ARGS=sign /ac c:/release/bin/DigiCertAssuredIDRootCA.cer /f c:/release/bin/NextLabs.pfx /p IiVf1itvOrqJ /n "NextLabs Inc." /fd sha256 /tr http://timestamp.digicert.com
SIGNTOOL_OFFICIAL='$(SIGNTOOL_OFFICIAL_TOOL)' $(SIGNTOOL_OFFICIAL_ARGS)

BIN_DIR=java
BUILDOUTPUTDIR=$(NLBUILDROOT)/bin/$(BIN_DIR)


#
# Variables
#

OFFICIALCERT=0

$(info --------------------------------------------------------------------------)
$(info --- Makefile.java ---)
$(info [PARAMETERS])
$(info OFFICIALCERT=$(OFFICIALCERT))
$(info JDK_DIR=$(JDK_DIR))
$(info SIGNTOOL_OFFICIAL=$(SIGNTOOL_OFFICIAL))
$(info [PATHS])
$(info BIN_DIR=$(BIN_DIR))
$(info BUILDOUTPUTDIR=$(BUILDOUTPUTDIR))
$(info ---------------------------------------------------------------------------)
