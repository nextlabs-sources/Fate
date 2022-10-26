#!/bin/bash
mkdir -p build_install/release_win_x86
mkdir -p build_install/release_win_x64
cp -R bin/java build_install/jars
cp -R bin/release_win_x86/* build_install/release_win_x86
cp -R bin/release_win_x64/* build_install/release_win_x64
mkdir -p build_install/main/resource
mkdir -p build_install/main/resource64
mkdir -p build_install/main/resourceXP
cp bin/release_win_x86/nl_klog.sys build_install/main/resource/nl_klog.dll
cp bin/release_win_x64/nl_klog.sys  build_install/main/resource64/nl_klog.dll
cp bin/release_win_x86/nl_crypto.sys build_install/main/resource/nl_crypto.dll
cp bin/release_win_x64/nl_crypto.sys build_install/main/resource64/nl_crypto.dll
cp bin/release_win_x86/nl_SysEncryption.sys build_install/main/resource
cp bin/release_win_x64/nl_SysEncryption.sys build_install/main/resource64
cp prod/pep/endpoint/se/configuration/nlse_plugin32.cfg build_install/main/resource
cp prod/pep/endpoint/se/configuration/nlse_plugin.cfg build_install/main/resource64
cp prod/pep/endpoint/se/configuration/TamperResistance.cfg build_install/main/resource/SystemEncryption_TamperResistance.cfg
cp prod/pep/endpoint/se/nl_SysEncryption/filter/NLSE.inf build_install/main/resource
cp prod/pep/endpoint/se/configuration/SystemEncryptionService.properties build_install/main/resource
mkdir -p build_install/main/installer/obl
cp install/se/Script\ Files/include/CommonInstallScript.obl build_install/main/installer/obl
cp install/se/Script\ Files/include/PluginInstallerSDK.obl build_install/main/installer/obl
mkdir -p build_install/main/image
cp install/se/src/resource/ce-32.ico build_install/main/image/
