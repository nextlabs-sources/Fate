WDF redistributable binaries
=============================

The sub-folders contain the free and checked version of the KMDF and 
UMDF coinstallers. 

The checked version of coinstaller can be used only on Windows 2000, Windows XP, 
and Windows Server 2003 operating systems. They cannot be used on Windows Vista, 
Windows Vista SP1 and Windows Server 2008 operating systems.

The checked version of the Coinstaller is provided only to aid in debugging 
during your driver develompent and testing phases. Please DO NOT package the 
checked version of the coinstaller while shipping your binaries.

Note that you cannot install a checked build of the KMDF or UMDF runtime on a 
free build of the OS. Similarly, a free build of the KMDF or UMDF runtime cannot 
be installed on a checked build of the OS.



