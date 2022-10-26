# Makefile inc file for the SDK module
# This is slightly awkward as we don't really have anything to build
# So, there is no TARGET / SUBTARGET, just a custom target

VERSION_PRODUCT		= $(VERSION_PRODUCT_PC)
VERSION_MAJOR		= $(VERSION_MAJOR_PC)
VERSION_MINOR		= $(VERSION_MINOR_PC)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_PC)
VERSION_PATCH		= $(VERSION_PATCH_PC)

COMPILER=VS2008
PLATFORMSDK=7.0

# My custom task, here we copy the CEsdk.h and the run.bat file out
# Also copy the entire sampleEnforcer out
CUSTOM:
	@if [ ! -d ${BUILDOUTPUTDIR}/$(INCDIR) ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(INCDIR) ;        \
	fi
	@cp -f  $(NLENFORCERSDIR)/prod/pc/cesdk/include/CEsdk.h ${BUILDOUTPUTDIR}/$(INCDIR) ;
	@chmod a+w ${BUILDOUTPUTDIR}/$(INCDIR)/CEsdk.h ;
	@if [ ! -d ${BUILDOUTPUTDIR}/$(BINDIR) ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(BINDIR) ;        \
	fi
	@cp -f  $(NLENFORCERSDIR)/prod/pc/cesdk/include/run.bat ${BUILDOUTPUTDIR}/$(BINDIR) ;
	@chmod a+w ${BUILDOUTPUTDIR}/$(BINDIR)/run.bat ;
	@if [ ! -d ${BUILDOUTPUTDIR}/sample ] ; then \
	  mkdir -p ${BUILDOUTPUTDIR}/sample ;        \
	fi
	@sed -e s/ADD_INCLUDE_DIR_TOKEN/\.\.\\\\\\.\.\\\\\\.\.\\\\include/ \
	     -e s/ADD_LIB_DIR_TOKEN/\.\.\\\\\.\.\\\\\.\.\\\\\.\.\\\\prods\\\\SDK\\\\build.output\\\\release_lib_win32/ \
		 -e s/ADD_DEBUG_INCLUDE_DIR_TOKEN/\.\.\\\\\\.\.\\\\\\.\.\\\\include/ \
	     -e s/ADD_DEBUG_LIB_DIR_TOKEN/\.\.\\\\\.\.\\\\\.\.\\\\\.\.\\\\prods\\\\SDK\\\\build.output\\\\lib_win32/ \
		 < $(NLENFORCERSDIR)/prod/pc/cesdk/client/applications/sampleEnforcer/SampleEnforcer_vcproj.template \
		 > $(NLENFORCERSDIR)/prod/pc/cesdk/client/applications/sampleEnforcer/SampleEnforcer.vcproj
	@cp -rf  $(NLENFORCERSDIR)/prod/pc/cesdk/client/applications/sampleEnforcer ${BUILDOUTPUTDIR}/sample ;
	@chmod -R a+w ${BUILDOUTPUTDIR}/sample/sampleEnforcer ;
