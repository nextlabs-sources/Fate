Building OE Installer
05/14/2011

1. Problem building OE installer using Hudson

If you encounter the following error when build OE installer using Hudson, it is because the 
path is too long. The solution to this problem is to map the build directory to k:/ to reduce
the path length. Run ./configure --mapBuildRoot=k to shorten the build path.

Merging modules...
Merging Visual C++ 9.0 CRT (x86) WinSXS MSM: C:\Program Files\InstallShield\2010 StandaloneBuild\Modules\i386\Microsoft_VC90_CRT_x86_x64.msm
ISDEV : error -6267: An error occurred while extracting files from the cab file c:\hudson\jobs\release-1105-1-5.5.2\workspace\1105-1-5.5.2\edp\install\oe\build\output\32bit\Interm\MergeModules\Microsoft_VC90_CRT_x64.50FC30FE_9758_3B08_B886_7BAABC047B61\MM.Cab to the location c:\hudson\jobs\release-1105-1-5.5.2\workspace\1105-1-5.5.2\edp\install\oe\build\output\32bit\Interm\MergeModules\Microsoft_VC90_CRT_x64.50FC30FE_9758_3B08_B886_7BAABC047B61
ISDEV : fatal error -5087: Stop at first error
32bit\msi - 2 error(s), 0 warning(s)
Log file has been created: <file:c:\hudson\jobs\release-1105-1-5.5.2\workspace\1105-1-5.5.2\edp\install\oe\build\output\32bit\msi\LogFiles\5-14-2011 01-48-29 AM.txt>