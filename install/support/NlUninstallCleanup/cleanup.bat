
echo *********************************
echo **                             **
echo **   NEXTLABS Cleanup Script   **
echo **                             **
echo *********************************
echo.
echo   by Roni Tan
echo   copyright NextLabs Inc
echo.
echo.

setlocal

echo Delete Endpoint services
sc delete nltamper
sc delete nlcc
sc delete nlinjection
sc delete procdetect
sc delete nlSysEncryption
sc delete nlSysEncryptionFW
sc delete nl_devenf
sc delete complianceenforcerservice

echo Delete drivers
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Compliant Enterprise\Policy Controller"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v InstallDir') DO set ValueValue=%%A
echo %ValueValue%
set pcode=%ValueValue:~24%
"%pcode%policy controller\driver\nldevcon.exe" remove root\nlcc

echo Delete temp file
del %temp%\installercommon.dll
del %temp%\PDPStop.dll

echo Delete control set
reg delete "hklm\system\currentcontrolset\services\eventlog\application\compliant enterprise" /f
reg delete "HKLM\SYSTEM\CurrentControlSet\Services\ProcDetect" /f
reg delete "HKLM\SYSTEM\CurrentControlSet\services\ComplianceEnforcerService" /f
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\SafeBoot\Minimal\ComplianceEnforcerService" /f
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\SafeBoot\Network\ComplianceEnforcerService" /f
reg delete "HKLM\SYSTEM\ControlSet001\services\eventlog\Application\Compliant Enterprise" /f
reg delete "HKLM\SYSTEM\ControlSet002\services\eventlog\Application\Compliant Enterprise" /f
reg delete "HKLM\SYSTEM\ControlSet003\services\eventlog\Application\Compliant Enterprise" /f

echo Delete plug-ins
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Browser Helper Objects\{FB159F40-0C40-4480-9A72-71C1D07606B7}" /f
reg delete "HKLM\SOFTWARE\Classes\AppID\{2A9EAF67-12C2-4655-A2DD-E7D988A3AE59}" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\Word\Addins\NLVLOfficeEnforcer.1" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\Excel\Addins\NLVLOfficeEnforcer.1" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\PowerPoint\Addins\NLVLOfficeEnforcer.1" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\Outlook\Addins\msoPEP.msoObj.1" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\Excel\Addins\CEOffice.Office" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\PowerPoint\Addins\CEOffice.Office" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\Word\Addins\CEOffice.Office" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\Word\Addins\OutlookAddin.1" /f
reg delete "HKLM\SOFTWARE\Microsoft\Office\Word\Addins\NlPortableEncryptionCtx.1" /f

reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Windows\CurrentVersion\Explorer\Browser Helper Objects\{FB159F40-0C40-4480-9A72-71C1D07606B7}" /f
reg delete "HKLM\SOFTWARE\wow6432node\Classes\AppID\{2A9EAF67-12C2-4655-A2DD-E7D988A3AE59}" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\Word\Addins\NLVLOfficeEnforcer.1" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\Excel\Addins\NLVLOfficeEnforcer.1" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\PowerPoint\Addins\NLVLOfficeEnforcer.1" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\Outlook\Addins\msoPEP.msoObj.1" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\Excel\Addins\CEOffice.Office" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\PowerPoint\Addins\CEOffice.Office" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\Word\Addins\CEOffice.Office" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\Word\Addins\OutlookAddin.1" /f
reg delete "HKLM\SOFTWARE\wow6432node\Microsoft\Office\Word\Addins\NlPortableEncryptionCtx.1" /f


echo Delete file extensions
reg delete "HKCR\.nxl" /f
reg delete "HKCR\nxlfile" /f
reg delete "HKLM\SOFTWARE\Classes\.nxl" /f
reg delete "HKLM\SOFTWARE\Classes\nxlfile" /f

echo Delete RDE registry
reg add hklm\system\currentcontrolset\control\class\{e0cbf06c-cd8b-4647-bb8a-263b43f0f974} /v UpperFilters /f
reg add hklm\system\currentcontrolset\control\class\{d48179be-ec20-11d1-b6b8-00c04fa372a7} /v UpperFilters /f
reg add hklm\system\currentcontrolset\control\class\{c06ff265-ae09-48f0-812c-16753d7cba83} /v UpperFilters /f
reg add hklm\system\currentcontrolset\control\class\{7ebefbc0-3200-11d2-b4c2-00a0c9697d07} /v UpperFilters /f
reg add hklm\system\currentcontrolset\control\class\{6bdd1fc6-810f-11d0-bec7-08002be2092f} /v UpperFilters /f
reg add hklm\system\currentcontrolset\control\class\{4d36e972-e325-11ce-bfc1-08002be10318} /v UpperFilters /f
reg add hklm\system\currentcontrolset\control\class\{36fc9e60-c465-11cf-8056-444553540000} /v UpperFilters /f

reg add hklm\system\controlset001\control\class\{e0cbf06c-cd8b-4647-bb8a-263b43f0f974} /v UpperFilters /f
reg add hklm\system\controlset001\control\class\{d48179be-ec20-11d1-b6b8-00c04fa372a7} /v UpperFilters /f
reg add hklm\system\controlset001\control\class\{c06ff265-ae09-48f0-812c-16753d7cba83} /v UpperFilters /f
reg add hklm\system\controlset001\control\class\{7ebefbc0-3200-11d2-b4c2-00a0c9697d07} /v UpperFilters /f
reg add hklm\system\controlset001\control\class\{6bdd1fc6-810f-11d0-bec7-08002be2092f} /v UpperFilters /f
reg add hklm\system\controlset001\control\class\{4d36e972-e325-11ce-bfc1-08002be10318} /v UpperFilters /f
reg add hklm\system\controlset001\control\class\{36fc9e60-c465-11cf-8056-444553540000} /v UpperFilters /f

reg add hklm\system\controlset002\control\class\{e0cbf06c-cd8b-4647-bb8a-263b43f0f974} /v UpperFilters /f
reg add hklm\system\controlset002\control\class\{d48179be-ec20-11d1-b6b8-00c04fa372a7} /v UpperFilters /f
reg add hklm\system\controlset002\control\class\{c06ff265-ae09-48f0-812c-16753d7cba83} /v UpperFilters /f
reg add hklm\system\controlset002\control\class\{7ebefbc0-3200-11d2-b4c2-00a0c9697d07} /v UpperFilters /f
reg add hklm\system\controlset002\control\class\{6bdd1fc6-810f-11d0-bec7-08002be2092f} /v UpperFilters /f
reg add hklm\system\controlset002\control\class\{4d36e972-e325-11ce-bfc1-08002be10318} /v UpperFilters /f
reg add hklm\system\controlset002\control\class\{36fc9e60-c465-11cf-8056-444553540000} /v UpperFilters /f

reg add hklm\system\controlset003\control\class\{e0cbf06c-cd8b-4647-bb8a-263b43f0f974} /v UpperFilters /f
reg add hklm\system\controlset003\control\class\{d48179be-ec20-11d1-b6b8-00c04fa372a7} /v UpperFilters /f
reg add hklm\system\controlset003\control\class\{c06ff265-ae09-48f0-812c-16753d7cba83} /v UpperFilters /f
reg add hklm\system\controlset003\control\class\{7ebefbc0-3200-11d2-b4c2-00a0c9697d07} /v UpperFilters /f
reg add hklm\system\controlset003\control\class\{6bdd1fc6-810f-11d0-bec7-08002be2092f} /v UpperFilters /f
reg add hklm\system\controlset003\control\class\{4d36e972-e325-11ce-bfc1-08002be10318} /v UpperFilters /f
reg add hklm\system\controlset003\control\class\{36fc9e60-c465-11cf-8056-444553540000} /v UpperFilters /f

reg add hklm\system\controlset004\control\class\{e0cbf06c-cd8b-4647-bb8a-263b43f0f974} /v UpperFilters /f
reg add hklm\system\controlset004\control\class\{d48179be-ec20-11d1-b6b8-00c04fa372a7} /v UpperFilters /f
reg add hklm\system\controlset004\control\class\{c06ff265-ae09-48f0-812c-16753d7cba83} /v UpperFilters /f
reg add hklm\system\controlset004\control\class\{7ebefbc0-3200-11d2-b4c2-00a0c9697d07} /v UpperFilters /f
reg add hklm\system\controlset004\control\class\{6bdd1fc6-810f-11d0-bec7-08002be2092f} /v UpperFilters /f
reg add hklm\system\controlset004\control\class\{4d36e972-e325-11ce-bfc1-08002be10318} /v UpperFilters /f
reg add hklm\system\controlset004\control\class\{36fc9e60-c465-11cf-8056-444553540000} /v UpperFilters /f

echo Delete the main Nextlabs registry
reg delete hklm\software\nextlabs /f
reg delete hklm\software\wow6432node\nextlabs /f

echo Stop EDP Manager
taskkill /im edpmanager.exe /t

echo Delete folders
rmdir "%pcode%policy controller" /s /q
rmdir "%pcode%desktop enforcer" /s /q
rmdir "%pcode%live meeting enforcer" /s /q
rmdir "%pcode%network enforcer" /s /q
rmdir "%pcode%office communicator enforcer" /s /q
rmdir "%pcode%outlook enforcer" /s /q
rmdir "%pcode%removable device enforcer" /s /q
rmdir "%pcode%system encryption" /s /q
rmdir "%pcode%common" /s /q

rmdir "%pcode%" /s /q

rmdir "c:\program files(x86)\nextlabs\outlook enforcer" /s /q
rmdir "c:\program files(x86)\nextlabs\policy controller" /s /q

echo Delete shared dll indication
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\nextlabs.cscinvoke.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\nextlabstagginglib32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\podofolib.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\tagviewmenu32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\cebrain32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\cecem32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\ceconn32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\ceeval32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\celog32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\celogging32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\cemarshal5032.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\cepepman32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\ceprivate32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\cesdk32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\cesec32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\ceservice32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\cetransport32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\config\\encryption_adapters32.conf" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\freetype6.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\libtiff.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\nl_sysenc_lib32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\nlcc_ulib32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\pa_encrypt32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\pa_filetagging32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\pa_pe32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\pafui32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\pdflib32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\pgp_adapter32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\resattrlib32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\resattrmgr32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\tag_office2k732.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\zip_adapter32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\zlib1.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\nlca_tamper.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\help\\images\\greyinfo.gif" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jlib\\dnsjava.jar" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\cepdpman.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\amd64\\nlcc.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\nlcc.inf" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\jar\\nlcc\\nlcc_dispatcher.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\mfc90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\mfc90u.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\mfcm90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\mfcm90u.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\msvcm90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\msvcp90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\msvcr90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\microsoft.vc90.atl.manifest" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\common\\bin32\\vcomp90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\x86\\wdfcoinstaller01009.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\podofolib.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\cetamperproof32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\cepdpgeneric32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\nltamperproofconfig32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\nl_tamper_plugin32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\nlca_client32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\nlca_framework32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\nlca_plugin32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\nlca_service.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\policycontroller_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\nl_tamper\\objfre_wxp_x86\\i386\\nl_tamper.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nlca_plugin32.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\nlcc.inf" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\nlcc.x86.cat" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\nldevcon.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\microsoft.vc90.atl\\atl90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\microsoft.vc90.crt\\msvcm90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\microsoft.vc90.mfc\\mfc90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\microsoft.vc90.openmp\\vcomp90.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\mch_install_test.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\config\\keymanagementservice.properties" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\bin\\cesdk32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\bin\\ceservice32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\jar\\keymanagementservice.jar" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\bin\\keymanagementconsumer32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\bin\\keyutil.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\basepep32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\ce_deny.gif" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\ce_deny.html" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\enhancement32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\iepep32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlscekeeper32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlregisterplugins.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\windowsdesktopenforcer_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nl_screencap_plugin.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\diagnostic32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\edpmanager.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\edpmgrutility32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\edpmdlg32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\ipcproxy32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\ipcstub32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlsce.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\notification32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\pcstatus32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\wdeaddtags.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\wde_base_plugin.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlvlofficeenforcer200332.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlvlofficeenforcer200732.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlvlviewprint32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlvisuallabelingpa200332.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlvisuallabelingpa200732.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\dialog.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\notify.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\status_plugin.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\menu_l.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\installspicaller32.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\networkenforcer_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\ftpe32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\config\\ftpe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\service\\injection\\ftpte.exe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\service\\injection\\smartftp.exe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\httpe32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\config\\httpe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\hpe32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\config\\hpe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\installoex64.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\approvaladapter.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\mso2k3pep32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\mso2k7pep32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\odhd2k332.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\odhd2k732.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\outlookenforcer_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\readme.txt" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\ce_explorer.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\injectexp.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\oeservice.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\ceoffice32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\ce_explorer32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\injectexp32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\nlregisterplugins.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\oeservice32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\windowblocker32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\adaptercomm32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\adaptermanager32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\approvaladapter32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\boost_regex-vc90-mt-1_43.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\mso2010pep32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\odhd201032.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nl_oe_plugin32.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\live meeting enforcer\\bin\\nllme32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\livemeetingenforcer_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\service\\injection\\pwconsole.exe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\removabledeviceenforcer_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\driver\\i386\\wdfcoinstaller01009.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\bin\\nextlabscredentialprovider.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\bin\\logon_detection_win7.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\driver\\nl_devenf.x86.cat" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\driver\\nldevcon.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\rde_plugin32.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\office communicator enforcer\\bin\\ipcproxy.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\office communicator enforcer\\readme.txt" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\service\\injection\\communicator.exe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\officecommunicatorenforcer_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\windows\\system32\\drivers\\nl_crypto.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\iconbadging32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\driver\\nlsefw.inf" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\systemencryption\\systemencryptionservice.jar" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\systemencryption_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlportableencryption.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlportableencryptionctx32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlsysencryption.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlsysencryptionobligation.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\driver\\objfre_wxp_x86\\i386\\nl_sysencryption.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\driver\\objfre_wxp_x86\\i386\\nl_sysencryptionfw.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nl_autounwrapper.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\windows\\system32\\drivers\\nl_klog.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlregisterplugins.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlse_plugin32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlsefw_plugin32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nlse_plugin32.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nlsefw_plugin32.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\systemencryption\\pcs_server32.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\edpmanager.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\network enforcer\\bin\\installspicaller.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\cepdpman.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\nlcc.inf" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\amd64\\nlcc.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\jar\\nlcc\\nlcc_dispatcher.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\service\\injection\\taskkill.exe.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\windows\\system32\\drivers\\nlinjection64.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\basepep.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\bin\\cesdk.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\bin\\ceservice.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\keymanagement\\bin\\keymanagementconsumer.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlregisterplugins.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\pcstatus.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\wdeaddtags.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\basepepplugin.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\outlookaddin.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\diagnostic.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\edpmdlg.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\edpmgrutility.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\enhancement.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\iepep.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\ipcproxy.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\ipcstub.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlsce.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\nlscekeeper.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\notification.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\wde_base_plugin.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\dialog.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\menu_l.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\menu_r.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\notify.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\desktop enforcer\\bin\\status_plugin.ini" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\tamper_resistance\\windowsdesktopenforcer_tamperresistance.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nl_screencap_plugin.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\iconbadging.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlportableencryption.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlportableencryptionctx.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlsysencryptionobligation.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\driver\\objfre_win7_amd64\\amd64\\nl_sysencryption.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\driver\\objfre_win7_amd64\\amd64\\nl_sysencryptionfw.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nl_autounwrapper.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\windows\\system32\\drivers\\nl_crypto.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\windows\\system32\\drivers\\nl_klog.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlse_plugin.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlsefw_plugin.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\system encryption\\bin\\nlsysencryption.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nlse_plugin.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\nlsefw_plugin.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\systemencryption\\pcs_server.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\ceoffice.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\nlregisterplugins.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\windowblocker.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\adaptercomm.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\adaptermanager.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\approvaladapter.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\boost_regex-vc90-mt-1_43.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\mso2010pep.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\outlook enforcer\\bin\\odhd2010.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\bin\\nextlabscredentialprovider.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\bin\\logon_detection_win7.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\bin\\nl_devenf_plugin.dll" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\removable device enforcer\\driver\\nldevcon.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\config\\plugin\\rde_plugin.cfg" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\bin\\cepdpman.exe" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\nlcc.inf" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\driver\\amd64\\nlcc.sys" /f
reg delete hklm\software\microsoft\windows\currentversion\shareddlls /v "c:\\program files\\nextlabs\\policy controller\\jservice\\jar\\nlcc\\nlcc_dispatcher.dll" /f
