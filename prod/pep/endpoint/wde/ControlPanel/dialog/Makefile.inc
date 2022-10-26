
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = edpmdlg

SRC = \
  src/dllmain.cpp \
  src/EDPMgrDlg.cpp \
  src/emgrdlg.cpp \
  src/HelpDlg.cpp \
  src/IniFile.cpp \
  src/NL_ListView.cpp \
  src/NL_NotificationsList.cpp \
  src/NotifyDlg.cpp \
  src/SettingDlg.cpp \
  src/stdafx.cpp \
  src/SummaryDlg.cpp \
  src/tabcfg.cpp \
  src/StopEnfDlg.cpp \
  src/WarningPCPWDDlg.cpp \
  src/WarningPCUnexpectDlg.cpp \
  src/ProgressDlg.cpp \
  src/NL_TabCtrl.cpp

RCSRC = src/emgrdlg.rc

INCPATH += \
  $(NLEXTERNALDIR)/wtl/wtl80/include \
  ../ \
  ./ \
  include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/common/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/utility/include \
  $(NLBUILDROOT)/prod/pc/IPCproxy/include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLEXTERNALDIR)/wtl/wtl80/include \
  $(NLBUILDROOT)/prods/WDE/build.output/dependencies

LIBPATH += \
  $(NLBUILDROOT)/prod/pc/IPCstub/$(BINDIR)	\
  $(NLBUILDROOT)/prod/pc/IPCproxy/$(BINDIR)

LIBS += comctl32 \
  ipcstub$(SUBTARGET_SUFFIX) \
  ipcproxy_static$(SUBTARGET_SUFFIX)

DFLAGS += -DEMGRDLG_EXPORTS
