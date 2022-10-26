
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

SUBTARGET = NLVLViewPrint

SRC = \
      src/dllmain.cpp \
      src/Auxiliary.cpp\
      src/stdafx.cpp \
      src/NLVLViewPrint.cpp \
      src/OvelayWndMgr.cpp \
      src/OverlayWnd.cpp \
      src/ManageThread.cpp

INCPATH += \
  ./include \
  ../../include \
  $(NLEXTERNALDIR)/prod/pa/paf/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0

LIBS += gdi32 Ws2_32 gdiplus Comdlg32 Msimg32

DFLAGS += -DDOWATERMARK_EXPORTS
