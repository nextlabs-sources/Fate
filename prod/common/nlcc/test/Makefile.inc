
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

WINDDK = C:/WinDDK/7600.16385.1

TARGET = nlcc_test

SRC = src/nlcc_test.cpp

INCPATH += \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += \
  $(NLBUILDROOT)/prod/common/nlcc/ulib/$(BINDIR)

LIBS += \
  nlcc_ulib_static$(SUBTARGET_SUFFIX)