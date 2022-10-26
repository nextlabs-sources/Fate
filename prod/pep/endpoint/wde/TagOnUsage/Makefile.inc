
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
TARGET = WdeAddTags

SRC = \
  src/stdafx.cpp \
  src/addTags.cpp \
  src/AddTagsDlg.cpp \
  src/AddTagsMgr.cpp \
  src/NL_ListView_ComboBox.cpp \
  src/CriSectionMgr.cpp \
  src/NXTLBS_ListView.cpp \
  src/PromptDlg.cpp \
  src/Tagging.cpp \
  src/Utils.cpp

RCSRC = src/addTags.rc

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLEXTERNALDIR)/wtl/WTL80/include \

LIBPATH += 

LIBS += \
  comctl32

