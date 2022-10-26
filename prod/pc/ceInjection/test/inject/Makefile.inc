# Makefile inc file for the ceInjection module
# The source files are win32 platform only

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

# Define the target
TARGET=inject

# Define the objects of the target module
SRC = src/inject.cpp

# Define the custom header include path
INCPATH += \
  $(NLPLATFORMSDIR)/win32/lib/madCHook


# Define the custom library path
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLPLATFORMSDIR)/win32/lib/madCHook \
  $(NLPLATFORMSDIR)/common/celog/$(BINDIR)         

# Define the custom library to link
# eg. mylib myother_lib
LIBS += \
  madCHook-static \
  oleaut32 

DFLAGS +=
