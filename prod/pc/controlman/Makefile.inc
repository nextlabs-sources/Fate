
VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

# Define the target
TARGET = controlmodule

# Define the objects of the target module
SRC =  \
  src/controlmodule.cpp \
  src/tamperproof.c     \
  src/policy.cpp        \
  src/users.cpp         \
  src/utils.cpp 


# Define the custom header include path
INCPATH +=                                             \
  $(NLPLATFORMSDIR)/common/include                     \
  $(NLPLATFORMSDIR)/common/brain/include               \
  $(NLPLATFORMSDIR)/linux/drivers/syslistener/include  \
  include 

# Define the custom library path
LIBPATH += \
  $(BUILDOUTPUTDIR)/lib

# Define the custom library to link
# eg. mylib myother_lib
LIBS += \
  jvm                                           \
  rt                                            \
  cebrain                                       \
  pthread

