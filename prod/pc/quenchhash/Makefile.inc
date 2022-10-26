# Makefile inc file for the quenchhash module
# The source files are OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = cequenchhash
SUBTARGET_STATIC=yes

# Define the objects of the target module
SRC = src/quenchhash.cpp           

INCPATH += include

# Define the custom library to link
LIBS += \
	advapi32 \
	iphlpapi

