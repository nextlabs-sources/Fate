cd C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\Install2015\Data\x86
copy /y msvcp71.dll C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x86\msvcp71.dll 
copy /y msvcr71.dll C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x86\msvcr71.dll 
copy /y PCStop.exe C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x86\PCStop.exe
copy /y NlRegisterPlugins32.exe C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x86\NlRegisterPlugins.exe
copy /y OEPlugin.exe C:\jenkins\jobs\release_endpoint_main\workspace\install\oe\build\data\release_win_x86

cd C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\Install2015\Data
copy /y nl_tamper.inf C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x64\nl_tamper.inf
copy /y nlinjection.inf C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x64\nlinjection.inf
copy /y nl_tamper.inf C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x86\nl_tamper.inf
copy /y nlinjection.inf C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x86\nlinjection.inf

cd C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\java\
copy /y castor.jar castor-0.9.5.4.jar 
copy /y commons-collections.jar commons-collections-2.1.1.jar 
copy /y commons-discovery.jar commons-discovery-0.2.jar 
copy /y ehcache.jar ehcache-1.1.jar
copy /y wsdl4j.jar wsdl4j-1.5.1.jar


cd \
cd C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\resource\
copy /y logging.template.properties logging.properties 
copy /y commprofile.template.xml commprofile.xml



cd C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\resource32 
copy /y nlcc.x86.cat ..\release_win_x86\nlcc.x86.cat 
copy /y nl_tamper.x86.cat ..\release_win_x86\nl_tamper.x86.cat

cd C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\resource64 
copy /y nlcc.x64.cat ..\release_win_x64\nlcc.x64.cat 
copy /y nl_tamper.x64.cat ..\release_win_x64\nl_tamper.x64.cat

cd C:\jenkins\jobs\release_endpoint_main\workspace\install\pc\build\data\release_win_x86
copy /y celog32.dll celog.dll
copy /y celog232.dll celog2.dll

cd C:\jenkins\jobs\release_endpoint_main\workspace\install\oe\build\data\release_win_x86
copy /y NlRegisterPlugins32.exe NlRegisterPlugins.exe