
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

ifeq ($(OFFICEVERSION), 2003)
	SUBTARGET = NLVisualLabelingPA2003
else
	SUBTARGET = NLVisualLabelingPA2007
endif

SRC = \
	src/stdafx.cpp \
	src/dllmain.cpp \
	src/ExcelDocLabeling.cpp\
	src/OfficeVisualLabeling.cpp \
	src/NLVisualLabelingPA.cpp \
	src/PDFVisualLabeling.cpp \
	src/PPTDocLabeling.cpp \
	src/WordDocLabeling.cpp

INCPATH += \
	./include \
	$(NLBUILDROOT)/prod/pa/paf/include \
	$(NLEXTERNALDIR)/boost/boost_1_45_0 \
	$(NLBUILDROOT)/prod/common/include \
	$(NLBUILDROOT)/prod/common/celog/include

DFLAGS += -DPA_LABELING_EXPORTS

ifeq ($(OFFICEVERSION), 2003)
	DFLAGS += -DMSO2K3
	FORAPP = office2003
else
	DFLAGS += -DMSO2K7
	FORAPP = office2007
endif
