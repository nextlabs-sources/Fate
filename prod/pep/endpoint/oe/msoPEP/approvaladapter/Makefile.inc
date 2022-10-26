
VERSION_PRODUCT		= $(VERSION_PRODUCT_OE)
VERSION_MAJOR		= $(VERSION_MAJOR_OE)
VERSION_MINOR		= $(VERSION_MINOR_OE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_OE)
VERSION_PATCH		= $(VERSION_PATCH_OE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = approvaladapter

# Currently, top-level cannot support souce file lives outside ./src. Therefore, ../common/log.cpp is a problem.
SRC = \
	src/log.cpp \
	src/approvaladapter.cpp \
	src/stdafx.cpp

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/pep/endpoint/oe/msoPEP/common \
  $(NLBUILDROOT)/prod/pep/endpoint/oe/msoPEP/adaptercomm/include \
  $(NLBUILDROOT)/prod/pep/common/eframework \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR)
  
LIBS += ole32 oleaut32 adaptercomm$(SUBTARGET_SUFFIX)

DFLAGS += -DFTPADAPTER_EXPORTS
