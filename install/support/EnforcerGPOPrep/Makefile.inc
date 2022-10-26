
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

	
SUBDIRS   = 
TARGET = EnforcerGPOPrep

SRC = \
  CustomizationParameters.cpp \
  GPOTransformGenerator.cpp \
  EnforcerGPOPrep.cpp \
  XGetopt.cpp \
  stdafx.cpp

INCPATH += \
  include 

LIBPATH += \
  T:\bintool\Winsdk\PlatformSDK\Lib 

LIBS += msi


CFLAGS += \
  -DPEP_USE_LEGACY_IGNORE_CHECK

CXXFLAGS += \
  -DPEP_USE_LEGACY_IGNORE_CHECK