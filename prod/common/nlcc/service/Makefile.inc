
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER = VS2008
PLATFORMSDK=7.0

SUBDIRS = 

SUBTARGET = nlcc_dispatcher

SRC = \
  src/nlcc_dispatch.cpp

INCPATH = \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/common/nlcc/service/java \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include/win32

LIBPATH += \
  $(NLBUILDROOT)/prod/common/nlcc/ulib/$(BINDIR)

LIBS += \
  nlcc_ulib_static$(SUBTARGET_SUFFIX)
