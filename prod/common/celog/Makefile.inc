# Makefile inc file for the celog module
# The source files are OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

# Define the subdirectories that need to be built
SUBDIRS = 

# Define the target
SUBTARGET = celog

# Define the objects of the target module
SRC = src/celog.cpp

# Define the custom header include path
INCPATH += \
  include \
  $(NLEXTERNALDIR)/boost/boost_1_36_0

# Define the custom library path
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

# Define the custom library to link
# starts with -l eg. -lmylib -lmyother_lib
LIBS += 

DFLAGS +=

# Define the test target
#TESTTARGET   = celogtest

#TESTSRC      = \
#  test/celog_test.cpp 

#TESTLIBPATH += \
#  $(NLPLATFORMSDIR)/common/celog/$(BINDIR)

#TESTLIBS    += \
#   celog

