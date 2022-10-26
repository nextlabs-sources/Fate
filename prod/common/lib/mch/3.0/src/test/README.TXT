
MCH TEST

mchtest32.exe:	    32 bit test program
mchtest64.exe:	    64 bit test program
mchtestlib32.dll:   32 bit test library which will be injected into 32 bit processes
mchtestlib64.dll:   64 bit test program which will be injected into 64 bit processes
nlinjection32.sys:  32 bit NextLabs injection driver (Only valid in 32 bit OS)
nlinjection64.sys:  64 bit NextLabs injection driver (Only valid in 64 bit OS)

Usage:
    mchtest32/64.exe --install: install the injection driver
    mchtest32/64.exe --uninstall: uninstall the injection driver
    mchtest32/64.exe -i: inject dll
    mchtest32/64.exe -u: unload dll