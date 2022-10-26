#############################################################################################
#
# This is documentation on how to use UMDH.  It provides a basic introduction on how WinDbg
# can be installed, a program configured for use with UMDH and the analysis of a sample
# program.
#
#############################################################################################
Before Starting
---------------
(1) Install WinDbg from Microsoft.
    'umdh' and 'gflags' tools (used below) are part of that package.

    Usually WinDbg installs in a location like:

      C:\Program Files\Debugging Tools for Windows

Building the Sample
-------------------
#cl docleak.cpp /Zi /MTd /EHsc

Setup Symbols
-------------
(1) Setup symbol path environment.
#set _NT_SYMBOL_PATH=c:\websymbols;SRV*c:\websymbols*http://msdl.microsoft.com/download/symbols

(2) Pull symbols from Microsoft.
#symchk /r c:\windows\system32 /s SRV*c:\websymbols\*http://msdl.microsoft.com/download/symbols

Setup Program to Analyze
------------------------
(1) Enable global flags for user-mode stack trace (ust).  Remember to disable this when
    finished using '-ust' option.
#gflags /i myprogram.exe +ust

Analyze Program
---------------
(1) Run the program past initialization and take a 'before snapshot.
#umdh -p:[pid] -f:c:\before

(2) Run the program to produce a suspected leak.

(3) Take a snapshot after (2).
#umdh -p:[pid] -f:c:\after

(4) Perform a diff of the snapshots
#umdh -v -d c:\before c:\after > c:\diff

(5) View the diff and investigate.

Sample Output
-------------

The leak statistics are provided in the header of a stack trace (lines 1 and 2).  The leak, or
call into the heap, occurs at line 13 in symbol DocumentHTML::DocumentHTML.

01>+    1640 (   2788 -   1148)     17 allocs	BackTrace26
02>+      10 (     17 -      7)	BackTrace26	allocations
03>
04>	ntdll!RtlDebugAllocateHeap+000000E1
05>	ntdll!RtlAllocateHeapSlowly+00000044
06>	ntdll!RtlAllocateHeap+00000E64
07>	docleak!_heap_alloc_base+0000005C (f:\sp\vctools\crt_bld\self_x86\crt\src\malloc.c, 105)
08>	docleak!_heap_alloc_dbg+000001F5 (f:\sp\vctools\crt_bld\self_x86\crt\src\dbgheap.c, 411)
09>	docleak!_nh_malloc_dbg+00000019 (f:\sp\vctools\crt_bld\self_x86\crt\src\dbgheap.c, 268)
10>	docleak!malloc+00000019 (f:\sp\vctools\crt_bld\self_x86\crt\src\dbgheap.c, 154)
11>	docleak!operator new+0000000F (f:\sp\vctools\crt_bld\self_x86\crt\src\new.cpp, 59)
12>	docleak!operator new[]+0000000C (f:\sp\vctools\crt_bld\self_x86\crt\src\newaop.cpp, 7)
13>	docleak!DocumentHTML::DocumentHTML+0000004A (c:\umdh\docleak.cpp, 95)
14>	docleak!DocumentFactory+0000009A (c:\umdh\docleak.cpp, 120)
15>	docleak!main+0000002C (c:\umdh\docleak.cpp, 134)
16>	docleak!__tmainCRTStartup+00000233 (f:\sp\vctools\crt_bld\self_x86\crt\src\crt0.c, 327)
17>	docleak!mainCRTStartup+0000000D (f:\sp\vctools\crt_bld\self_x86\crt\src\crt0.c, 196)
18>	kernel32!BaseProcessStart+00000023
