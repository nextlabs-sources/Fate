
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
TARGET = edpmanager
MANIFEST = $(BINDIR)/edpmanager.exe.manifest

SRC = \
  src/stdafx.cpp \
  src/actionMenu.cpp \
  src/edlpmanager.cpp \
  src/EDPMgr.cpp \
  src/EDPMgrReqHandler.cpp \
  src/EnforcerStatusWindow.cpp \
  src/IniFile.cpp \
  src/NotifyMgr.cpp \
  src/ProductInfoDlg.cpp \
  src/StubMgr.cpp \
  src/SystrayMgr.cpp \
  src/tabcfg.cpp

RCSRC = src/edlpmanager.rc

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/pc/IPCStub \
  $(NLBUILDROOT)/prod/pc/IPCStub/include  \
  $(NLBUILDROOT)/prod/pc/IPCProxy/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/common/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/utility/include \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R3.0/eframework/platform \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLEXTERNALDIR)/wtl/wtl80/include

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/IPCstub/$(BINDIR)	\
  $(NLBUILDROOT)/prod/pc/IPCproxy/$(BINDIR)

LIBS += \
  ipcstub$(SUBTARGET_SUFFIX) \
  ipcproxy_static$(SUBTARGET_SUFFIX)


