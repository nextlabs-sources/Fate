
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

# Changing TARGET_SUFFIX will not work until order of including Makefile.inc 
# and env.CYGWIN_NT-5.1 is changed. In the meantime, change target name to work 
# around this problem.
ifeq ($(TARGETENVARCH), x86)
	TARGET = NlRegisterPlugins32
else
	TARGET = NlRegisterPlugins
endif

TARGET_SUFFIX = 32

SRC = \
  src/Main.cpp
 
INCPATH += \
	./include
	
LIBS += \
	Msi
	
LDFLAGS  += -SUBSYSTEM:WINDOWS