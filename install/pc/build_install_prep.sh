#!/bin/bash
#
# DESCRIPTION
#	This script is used to put PC binaries in place for building installers.


#
# Check for errors
#

if [ "$NLBUILDROOT" == "" ]; then
	echo "### ERROR: Missing \$NLBUILDROOT directory"
	exit 1
fi

if [ ! -d $NLBUILDROOT ] && [ ! -d $NLBUILDROOT/ ]; then
	echo "### ERROR: Build root $NLBUILDROOT does not exist"
	exit 1
fi


#
# Prepare environment
#

PROD_DIR=$NLBUILDROOT/prod
DIST_BIN_DIR=$NLBUILDROOT/bin
BUILD_INSTALL_DIR=$NLBUILDROOT/build_install

if [ -d /cygdrive/f/ ] && [ -e /cygdrive/f/version.jar ]; then
	cp -f /cygdrive/f/version.jar $BUILD_INSTALL_DIR
fi

mkdir -p $BUILD_INSTALL_DIR/main/resource
mkdir -p $BUILD_INSTALL_DIR/main/image
mkdir -p $BUILD_INSTALL_DIR/main/resource64
mkdir -p $BUILD_INSTALL_DIR/main/dependencies/main/installer
mkdir -p $BUILD_INSTALL_DIR/main/dependencies/main/lib
mkdir -p $BUILD_INSTALL_DIR/release_win_x86
mkdir -p $BUILD_INSTALL_DIR/release_win_x64

set -x
cp -pR $NLBUILDROOT/install/pc/resource/* $BUILD_INSTALL_DIR/main/resource
cp -f $NLBUILDROOT/install/KMService/Script\ Files/include/PluginInstallerSDK.obl $BUILD_INSTALL_DIR/main/dependencies/main/installer 

cp -f $BUILD_INSTALL_DIR/main/resource/stop.ico $BUILD_INSTALL_DIR/main/image 

cp -f $DIST_BIN_DIR/release_win_x86/*.dll $BUILD_INSTALL_DIR/release_win_x86/
cp -f $DIST_BIN_DIR/release_win_x86/*.exe $BUILD_INSTALL_DIR/release_win_x86/
cp -f $DIST_BIN_DIR/release_win_x86/*.sys $BUILD_INSTALL_DIR/release_win_x86/
cp -f $DIST_BIN_DIR/release_win_x86/*.cat $BUILD_INSTALL_DIR/main/resource/
cp -f $DIST_BIN_DIR/release_win_x64/*.dll $BUILD_INSTALL_DIR/release_win_x64/
cp -f $DIST_BIN_DIR/release_win_x64/*.exe $BUILD_INSTALL_DIR/release_win_x64/
cp -f $DIST_BIN_DIR/release_win_x64/*.sys $BUILD_INSTALL_DIR/release_win_x64/
cp -f $DIST_BIN_DIR/release_win_x64/*.cat $BUILD_INSTALL_DIR/main/resource64/
cp -f $DIST_BIN_DIR/java/NLCCService.jar $BUILD_INSTALL_DIR/release_win_x86
cp -f $DIST_BIN_DIR/release_dotnet2/Nextlabs.CSCinvoke.dll $BUILD_INSTALL_DIR/release_win_x64
cp -f $DIST_BIN_DIR/release_dotnet2/Nextlabs.CSCinvoke.dll $BUILD_INSTALL_DIR/release_win_x86

cp -f $PROD_DIR/common/nlcc/service/java/NLCCService.properties $BUILD_INSTALL_DIR/main/resource
cp -f $PROD_DIR/common/nlcc/driver/nlcc.inf $BUILD_INSTALL_DIR/main/resource
cp -f $PROD_DIR/common/nlcc/service/java/NLCCService.properties $BUILD_INSTALL_DIR/release_win_x86
cp -f $PROD_DIR/common/lib/mch/3.0/lib/free/nlinjection32.sys $BUILD_INSTALL_DIR/release_win_x86
cp -f $PROD_DIR/common/lib/mch/3.0/lib/free/nlinjection64.sys $BUILD_INSTALL_DIR/release_win_x64

cp -f $PROD_DIR/pc/configuration/TamperResistance.cfg $BUILD_INSTALL_DIR/main/resource64/PolicyController_TamperResistance.cfg
cp -f $PROD_DIR/pc/configuration/TamperResistance32.cfg $BUILD_INSTALL_DIR/main/resource/PolicyController_TamperResistance.cfg

cp -f $PROD_DIR/pep/common/nlcontextmanager/configuration/nl_contextmgr_plugin.cfg $BUILD_INSTALL_DIR/main/resource64/nl_contextmgr_plugin.cfg
cp -f $PROD_DIR/pep/common/nlcontextmanager/configuration/nl_contextmgr_plugin32.cfg $BUILD_INSTALL_DIR/main/resource/nl_contextmgr_plugin32.cfg
cp -f $PROD_DIR/pep/common/nlcontextmanager/configuration/nlcontextmgr_tamper.cfg $BUILD_INSTALL_DIR/main/resource/nlcontextmgr_tamper.cfg

cp -f $PROD_DIR/pc/content/configuration/nlca_plugin.cfg $BUILD_INSTALL_DIR/main/resource64
cp -f $PROD_DIR/pc/content/configuration/nlca_plugin32.cfg $BUILD_INSTALL_DIR/main/resource
cp -f $PROD_DIR/pc/content/configuration/nlca_tamper.cfg $BUILD_INSTALL_DIR/main/resource
cp -f $PROD_DIR/pc/cesdk/client/service/tamperproof/injection/procexp.exe32.ini $BUILD_INSTALL_DIR/main/resource/procexp.exe.ini
cp -f $PROD_DIR/pc/cesdk/client/service/tamperproof/injection/taskkill.exe32.ini $BUILD_INSTALL_DIR/main/resource/taskkill.exe.ini
cp -f $PROD_DIR/pc/cesdk/client/service/tamperproof/injection/taskmgr.exe32.ini $BUILD_INSTALL_DIR/main/resource/taskmgr.exe.ini
cp -f $PROD_DIR/pc/cesdk/client/service/tamperproof/injection/*.ini $BUILD_INSTALL_DIR/main/resource64

cp -f $PROD_DIR/pa/encryptionPA/configuration/encryption_adapters.conf $BUILD_INSTALL_DIR/release_win_x64
cp -f $PROD_DIR/pa/encryptionPA/configuration/encryption_adapters32.conf $BUILD_INSTALL_DIR/release_win_x86

cp -f $PROD_DIR/pep/endpoint/wde/configuration/license.cfg $BUILD_INSTALL_DIR/main/resource

cp -f $NLEXTERNALDIR/zlib/zlib1.dll $BUILD_INSTALL_DIR/main/dependencies/main/lib
cp -f $NLEXTERNALDIR/microsoft/wdfCoInstaller/1.9/x86/WdfCoInstaller01009.dll $BUILD_INSTALL_DIR/main/resource
cp -f $NLEXTERNALDIR/microsoft/wdfCoInstaller/1.9/amd64/WdfCoInstaller01009.dll $BUILD_INSTALL_DIR/main/resource64

# TBF: Should remove these when they are removed from installer
cp -f $BUILD_INSTALL_DIR/release_win_x64/nldevcon.exe build_install/main/resource64/devcon.exe
cp -f $BUILD_INSTALL_DIR/release_win_x86/nldevcon.exe build_install/main/resource/devcon.exe

# Replace Podofo v0.8.1 with Podofo v0.8.1 containing OLE compound document deadlock fix
cp -f $NLEXTERNALDIR/podofo-0.8.1-NextLabs-WFSE/release_lib_win32_vs2008/PoDoFoLib.dll $NLBUILDROOT/build_install/main/dependencies/main/lib/release_win_x86/VS2008/
cp -f $NLEXTERNALDIR/podofo-0.8.1-NextLabs-WFSE/release_lib_x64_vs2008/PoDoFoLib.dll $NLBUILDROOT/build_install/main/dependencies/main/lib/release_win_x64/VS2008/
set +x
