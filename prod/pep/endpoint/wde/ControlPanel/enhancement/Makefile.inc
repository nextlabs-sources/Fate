
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = enhancement

SRC = \
      src/dllmain.cpp \
      src/enhancement.cpp \
      src/NLEDPManager.cpp \
      src/enhancement_i.c \

RCSRC =	src/enhancement.rc 

# .idl file must be placed in SRC directory.
IDL_SRC = src/enhancement.idl

INCPATH += \
  ./ \
  ./include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/common/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)

DFLAGS += -DENHANCEMENT_EXPORTS