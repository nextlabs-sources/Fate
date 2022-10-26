# Makefile inc file for the brain module
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
SUBTARGET = cebrain
SUBTARGET_STATIC = yes

# Define the objects of the target module
SRC = src/osServCYGWIN_NT-5.1.cpp           \
      src/utilsCYGWIN_NT-5.1.cpp            \
      src/nlstrCYGWIN_NT-5.1.cpp            \
      src/nlthreadCYGWIN_NT-5.1.cpp         \
      src/nlthreadpool.cpp


# Define the custom header include path
INCPATH += \
  include  \
  $(NLBUILDROOT)/prod/common/include \
  $(JDKDIR)/include \
  $(JDKDIR)/include/win32

# Define the custom library path
LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) 


# Define the custom library to link
# starts with -l eg. -lmylib -lmyother_lib
LIBS += Mpr


# Define the test target
TESTTARGET = braintest

TESTSRC = test/test.cpp         \
          test/testnlstr.cpp    \
          test/testpool.cpp

TESTLIBS += cebrain_static$(SUBTARGET_SUFFIX)   \
            Mpr                                 \
            Wtsapi32                            \
            Imagehlp                            \
            Advapi32                            \
            Secur32                             \
            User32
