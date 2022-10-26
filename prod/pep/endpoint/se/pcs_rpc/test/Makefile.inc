
VERSION_PRODUCT		= $(VERSION_PRODUCT_NLE)
VERSION_MAJOR		= $(VERSION_MAJOR_NLE)
VERSION_MINOR		= $(VERSION_MINOR_NLE)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_NLE)
VERSION_PATCH		= $(VERSION_PATCH_NLE)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS = 

TARGET = pcs_rpc_test

SRC = \
  src/pcs_rpc_test.cpp \
  src/test_client.cpp \
  src/test_server.cpp \
  src/test_server_implementation.cpp \
  src/fake_ceservice.cpp

INCPATH = \
  $(WINSDK_ROOT)/include \
  $(MSVC_ROOT)/VC/include \
  ../include \
  src \
  $(NLBUILDROOT)/prod/pc/cesdk/include	\
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R2.2 \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH = \
  $(MSVC_ROOT)/VC/lib \
  $(WINSDK_ROOT)/lib \
  c:/PolicyAdapterSDK

LIBS += ole32

BUILDOUTPUTDIR=build/temp