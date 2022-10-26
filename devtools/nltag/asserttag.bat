@echo OFF
:: This script takes the 1st parameter as the folder to run assert tags on.
:: This script assumes that nltag.exe is located in the current folder where
::      the script is being run from

:: First delete the given tag on all the files in the directory
for /R %1 %%G in (*) do ("nltag.exe" "%%G" --del --name=testname --value=testvalue)

:: Assert the given tag for every file in the directory recursively (/R)
for /R %1 %%G in (*) do ("nltag.exe" "%%G" --assert --name=testname --value=testvalue)

:: Output the time required to read the attributes on the files
for /R %1 %%G in (*) do ("nltag.exe" "%%G" --view --time)

:: Again delete the given tag on all the files in the directory
for /R %1 %%G in (*) do ("nltag.exe" "%%G" --del --name=testname --value=testvalue)
