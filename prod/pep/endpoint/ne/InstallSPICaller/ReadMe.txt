How to use InstallSPICaller*.exe
2011/04/21


How to check if a WinSock Service Provider is installed
-----------------------------------------------------------
On x64, use SPIDemo.exe checked into //depot/dev/D_SiriusR2/devtools/nlspidemo.
SPIDemo.exe is a commandline tool.
	> spidemo -show -s
	> spidemo -show -d
	
On x86, use SPOrder.exe (GUI)


Showing what WinSock Service Providers are installed
-----------------------------------------------------
C:\tmp>SPIDemo.exe -show -s

Winsock 32-bit Catalog:
=======================
1001 - MSAFD Tcpip [TCP/IP]
1002 - MSAFD Tcpip [UDP/IP]
1003 - MSAFD Tcpip [RAW/IP]
1004 - MSAFD Tcpip [TCP/IPv6]
1005 - MSAFD Tcpip [UDP/IPv6]
1006 - MSAFD Tcpip [RAW/IPv6]
1007 - RSVP TCPv6 Service Provider
1008 - RSVP TCP Service Provider
1009 - RSVP UDPv6 Service Provider
1010 - RSVP UDP Service Provider

Winsock 64-bit Catalog:
=======================
1001 - MSAFD Tcpip [TCP/IP]
1002 - MSAFD Tcpip [UDP/IP]
1003 - MSAFD Tcpip [RAW/IP]
1004 - MSAFD Tcpip [TCP/IPv6]
1005 - MSAFD Tcpip [UDP/IPv6]
1006 - MSAFD Tcpip [RAW/IPv6]
1007 - RSVP TCPv6 Service Provider
1008 - RSVP TCP Service Provider
1009 - RSVP UDPv6 Service Provider
1010 - RSVP UDP Service Provider


Installing on x86 Host
---------------------------
In order to install ftpe32.dll, httpe32.dll and hpe32.dll on a x86 host (i.e., WinXP and Win7-32),
you need to run InstallSPICaller32.exe.

The installation steps are as follow:
	1. Right click on "DOS Prompt" and select "Run as Administrator"
	2. Run "InstallSPICaller32.exe install c:\tmp\ftpe32.dll" to install FTPE.
		Note: Must run as Admin and specify full path to DLL.
	3. Run SPOrder to check if ftpe is on the top of WinSock stack.
	
	
Installing on x64 Host
------------------------
x64 host needs both 32- and 64-bit service providers. We must run InstallSPICaller32.exe to 
install 32-bit SPs and InstallSPICaller.exe to install 64-bit SPs.

The installation steps are as follow:
	1. Right click on "DOS Prompt" and select "Run as Administrator"
	2. Run "InstallSPICaller32.exe install c:\tmp\ftpe32.dll" to install FTPE (32-bit).
		Note: Must run as Admin and specify full path to DLL.
	3. Run "SPIDemo -show -s" to check if ftpe is on the top of WinSock stack.
	4. Run "InstallSPICaller.exe install c:\tmp\ftpe.dll" to install FTPE (64-bit).
	5. Run "SPIDemo -show -s" to check if ftpe is on the top of WinSock stack.


	C:\tmp>SPIDemo.exe -show -s

	Winsock 32-bit Catalog:
	=======================
	1013 - Nextlabs FTPE Provider    <-- FTPE32 is installed
	1001 - MSAFD Tcpip [TCP/IP]
	1002 - MSAFD Tcpip [UDP/IP]
	1003 - MSAFD Tcpip [RAW/IP]
	1004 - MSAFD Tcpip [TCP/IPv6]
	1005 - MSAFD Tcpip [UDP/IPv6]
	1006 - MSAFD Tcpip [RAW/IPv6]
	1007 - RSVP TCPv6 Service Provider
	1008 - RSVP TCP Service Provider
	1009 - RSVP UDPv6 Service Provider
	1010 - RSVP UDP Service Provider

	Winsock 64-bit Catalog:
	=======================
	1001 - MSAFD Tcpip [TCP/IP]
	1002 - MSAFD Tcpip [UDP/IP]
	1003 - MSAFD Tcpip [RAW/IP]
	1004 - MSAFD Tcpip [TCP/IPv6]
	1005 - MSAFD Tcpip [UDP/IPv6]
	1006 - MSAFD Tcpip [RAW/IPv6]
	1007 - RSVP TCPv6 Service Provider
	1008 - RSVP TCP Service Provider
	1009 - RSVP UDPv6 Service Provider
	1010 - RSVP UDP Service Provider