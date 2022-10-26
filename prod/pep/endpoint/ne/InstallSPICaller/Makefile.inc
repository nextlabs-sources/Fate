
VERSION_PRODUCT		= $(VERSION_PRODUCT_NE) 
VERSION_MAJOR		= $(VERSION_MAJOR_NE)
VERSION_MINOR		= $(VERSION_MINOR_NE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NE)
VERSION_PATCH		= $(VERSION_PATCH_NE)

COMPILER=VS2008
PLATFORMSDK=7.0

# Changing TARGET_SUFFIX will not work until order of including Makefile.inc 
# and env.CYGWIN_NT-5.1 is changed. In the meantime, change target name to work 
# around this problem.
ifeq ($(TARGETENVARCH), x86)
	TARGET = InstallSPICaller32
else
	TARGET = InstallSPICaller
endif

TARGET_SUFFIX = 32

SRC = \
  src/InstallSPICaller.cpp

LIBPATH += \
  $(NLBUILDROOT)/$(BINDIR)

