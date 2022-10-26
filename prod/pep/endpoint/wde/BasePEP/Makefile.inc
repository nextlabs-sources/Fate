
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0


SUBTARGET = basepep



SRC += \
	src/dllmain.cpp \
	src/basepep.cpp \
	src/runtimecontext.cpp \
	src/baseeventprovidercontext.cpp \
	src/shellexplorercontext.cpp \
	src/genericcontext.cpp \
	src/commonutils.cpp \
	src/eventhander.cpp \
	src/eventparser.cpp \
	src/contentstorage.cpp \
	src/fileoperationdlg.cpp \
	src/droptargetproxy.cpp \
	src/GenericSaveAsObligation.cpp \
	src/cbcontext.cpp \
	src/ScreenCaptureAuxiliary.cpp \
	src/iecontext.cpp \
	src/disablescreencapturecontext.cpp \
	src/dllhostcontext.cpp \
	src/gsexplorercontext.cpp \
	src/chromecontext.cpp

INCPATH += \
  include \
  $(NLBUILDROOT)/prod/pep/endpoint/wde/Common \
  $(NLBUILDROOT)/prod/pc/cesdk/include \
  $(NLBUILDROOT)/prod/common/include \
  $(NLBUILDROOT)/prod/common/brain/include \
  $(NLBUILDROOT)/prod/common/resattrmgr/include \
  $(NLBUILDROOT)/prod/common/resattrlib/include \
  $(NLBUILDROOT)/prod/common/feature_manager \
  $(NLBUILDROOT)/prod/common/celog2/include \
  $(NLBUILDROOT)/prod/pc/cesdk/client/stub/service/include \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/inc \
  $(NLBUILDROOT)/prod/common/lib/detours/4.0.1/inc \
  $(NLBUILDROOT)/prod/pep/common/eframework/eframework.R4.0 \
  $(NLBUILDROOT)/prod/pa/paf/include \
  $(MSVC_ROOT)/vc/atlmfc/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 \
  $(NLBUILDROOT)/prod/common/nlcc \
  $(NLBUILDROOT)/prod/common/nlcc/ulib \
  $(NLBUILDROOT)/prod/common/nlcommonlib/include \

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  $(NLBUILDROOT)/prod/common/lib/mch/3.1.13/lib/free \
  $(NLBUILDROOT)/prod/common/lib/detours/4.0.1/lib

ifeq ($(TARGETENVARCH), x86)
LIBPATH += \
  $(MSVC_ROOT)/VC/atlmfc/lib \
  $(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x86/lib
else
LIBPATH += \
	$(MSVC_ROOT)/VC/atlmfc/lib/amd64 \
	$(NLEXTERNALDIR)/boost/boost_1_43_0/stage-x64/lib
endif

# We need to specify the full file path of fltlib in LIBS below.
# Otherwise, if we specify $(WINDDK)/lib/wnet/xxx in LIBPATH above,
# the linker complains about unresolved external symbols for
# _IID_IFileOperation, _CLSID_FileOperation and _IID_IShellItemArray
# for some reason.
ifeq ($(TARGETENVARCH), x86)
LIBS +=         \
  madCHook32 libdetoursx86 
else
LIBS +=         \
  madCHook64 libdetoursx64 
endif

LIBS += \
	Wininet \
	CELog2$(SUBTARGET_SUFFIX) \
	nlcc_ulib_static$(SUBTARGET_SUFFIX) \
	iepmapi


