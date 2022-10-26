
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = edpmgrutility

SRC = \
  src/dllmain.cpp \
  src/EdpMgrApi.cpp \
  src/edpmgrutility.cpp \
  src/InstalledComptMgrImp.cpp \
  src/LocalInfo.cpp \
  src/LogMgrImp.cpp \
  src/NotifyMgrImp.cpp \
  src/PCStatusMgrImp.cpp \
  src/stdafx.cpp \
  src/NoPermissionDlg.cpp \
  src/RegUtility.cpp \
  src/DlgMgrImp.cpp

RCSRC = src/edpmgrutility.rc

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0/eframework/platform \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/enhancement \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBPATH += 

LIBS += 

DFLAGS += -DEDPMGRUTILITY_EXPORTS
