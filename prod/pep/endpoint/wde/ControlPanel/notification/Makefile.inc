
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = notification

SRC = \
  src/dllmain.cpp \
  src/nl_notification.cpp \
  src/NTXLBS_Bubble.cpp \
  src/stdafx.cpp \
  src/NL_ListView.cpp \
  src/NL_NotificationsList.cpp \
  src/NotifyDlg.cpp \
  src/WndShadow.cpp

RCSRC = src/nl_notification.rc

INCPATH += \
  ./ \
  ./include \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/common/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/utility/include \
  $(NLEXTERNALDIR)/wtl/WTL80/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

DFLAGS += -DNL_NOTIFICATION_EXPORTS
