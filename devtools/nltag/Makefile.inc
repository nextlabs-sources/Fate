
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = nltag

SRC = \
	  src/ExtTag.cpp \
      src/nltag.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(NLBUILDROOT)/prod/common/celog/$(BINDIR) \
  $(BUILDOUTPUTDIR)/$(LIBDIR)
LIBS += shlwapi
#TBD: POON: This is a temporary fix. Will need to resolve top level Makefile include Makefile.inc and env.* order.
ifeq ($(TARGETENVARCH),)
	LIBS += celog$(SUBTARGET_SUFFIX)
else	
	ifeq ($(TARGETENVARCH),x86)
		LIBS += celog$(SUBTARGET_SUFFIX)
	else	
		LIBS += celog
	endif
endif

CUSTOM_DEFINES =

CFLAGS   += $(CUSTOM_DEFINES)
CXXFLAGS += $(CUSTOM_DEFINES)