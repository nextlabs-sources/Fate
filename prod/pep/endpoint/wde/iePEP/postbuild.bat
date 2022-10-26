@echo off
if %NLENFORCERSDIR%""=="" goto end
echo Copying supporting files
if "%1"=="Release" goto release
cp --preserve=timestamp import/dhook.* %NLENFORCERSDIR%/prods/WDE/build.output/lib_win32
cp --preserve=timestamp import/ce_deny.* %NLENFORCERSDIR%/prods/WDE/build.output/lib_win32
goto end
:release
cp --preserve=timestamp import/dhook.* %NLENFORCERSDIR%/prods/WDE/build.output/release_lib_win32
cp --preserve=timestamp import/ce_deny.* %NLENFORCERSDIR%/prods/WDE/build.output/release_lib_win32
:end
