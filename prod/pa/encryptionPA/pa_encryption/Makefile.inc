# Makefile inc file for the brain module
# The source files are OS dependent

VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

# Define the compiler to be used. At present it can be VS2008/VS2008
COMPILER = VS2008
PLATFORMSDK=7.0

# Define the target
SUBTARGET = pa_encrypt
MANIFEST = $(BINDIR)/pa_encrypt$(SUBTARGET_SUFFIX).dll.manifest

# On Windows, add MD
# Fix Bug 11440 by sharing dynamic linked CRT with zip_adapter.dll
DYNAMICCRT=yes

DFLAGS+=-D_USRDLL -DPA_ENCRYPTION_EXPORTS -DATL_STATIC_REGISTRY 

# Define the objects of the target module
SRC +=  \
	src/stdafx.cpp \
	src/MyListView.cpp \
	src/MsgDlg.cpp \
	src/AutoEncryptDlg.cpp \
	src/EncryptionWrapper.cpp \
	src/EncryptProgressDlg2.cpp \
	src/EncryptProgressDlg.cpp \
	src/log.cpp \
	src/PasswordDlg.cpp \
	src/pa_encryption.cpp \

RCSRC =	src/pa_encryption.rc \
  
# Define the custom header include path
INCPATH += \
  include  \
  ../include \
  $(NLENFORCERSDIR)/framework/paf/trunk/source/PAF/include \
  ../../paf/include \
  UI/ \
  $(NLBUILDROOT)/prod/common/include \
  $(NLEXTERNALDIR)/boost/boost_1_43_0 \
   
  

LIBPATH += \
  $(BUILDOUTPUTDIR)/$(LIBDIR) \
  
LIBS += gdiplus comctl32 comdlg32

# Define the custom library to link
# eg. mylib myother_lib

DFLAGS +=

