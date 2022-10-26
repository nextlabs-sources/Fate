
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

TARGET = TagFiles

SRC = \
      src/TagFiles.cpp \
      src/stdafx.cpp

INCPATH += \
  ./include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/resattrmgr/$(BINDIR) \
  $(NLBUILDROOT)/prod/common/resattrlib/$(BINDIR)

ifeq ($(TARGETENVARCH),)
	LIBS += resattrmgr$(SUBTARGET_SUFFIX)
	LIBS += resattrlib$(SUBTARGET_SUFFIX)
else	
	ifeq ($(TARGETENVARCH),x86)
		LIBS += resattrmgr$(SUBTARGET_SUFFIX)
		LIBS += resattrlib$(SUBTARGET_SUFFIX)
	else	
		LIBS += resattrmgr
		LIBS += resattrlib
	endif
endif