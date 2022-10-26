
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

DYNAMICCRT = yes

COMPILER=VS2008
PLATFORMSDK=7.0

SUBDIRS   = 
SUBTARGET = diagnostic
MANIFEST = $(BINDIR)/diagnostic$(SUBTARGET_SUFFIX).dll.manifest

SRC = \
  src/stdafx.cpp \
  src/CollectProgreeDlg.cpp \
  src/CollectZipLog.cpp \
  src/CompleteDlg.cpp \
  src/diagnostic.cpp \
  src/DiagParentDlg.cpp \
  src/dllmain.cpp \
  src/ReqlogWarningDlg.cpp \
  src/StopPCDlg.cpp \
  src/ThreadHelper.cpp \
  src/ZipLocationDlg.cpp \
  src/DelLogConfirmDlg.cpp \
  src/WarningLoactionDlg.cpp \
  src/WarningPCPWDDlg.cpp \
  src/WarningPCUnexpectDlg.cpp \
  src/Decrypt.cpp \
  src/ProgressDlg.cpp

RCSRC = src/diagnostic.rc

INCPATH += \
  ./include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/celog/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/common/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/utility/include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/ControlPanel/enhancement \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLEXTERNALDIR)/boost/boost_1_45_0 \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include/ \
  $(NLEXTERNALDIR)/j2sdk/jdk1.6.0_18/include/win32/ \
  $(NLBUILDROOT)/prod/pa/encryptionPA/include

LIBS += Comdlg32

DFLAGS += -DDIAGNOSTIC_EXPORTS
