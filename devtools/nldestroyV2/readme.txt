The function of this nldestroyv2 tool is to generate workers/threads.
The number of thread is specified (default is 1).
Each worker will create a file, copy then delete that file under a specified folder.
The file size is default at 10 bytes, unless it is specified with size option.
If the source option is specified, it will copy the specified file instead of 
creating a new one. In this scenario, the size option is not applicable.
Default iteration is 100 times, unless specified from the command line.

The syntax is:
	nldestroyV2.exe --dir="c:\temp"
	nldestroyV2.exe --dir="c:\temp folder" --count=10 --thread=3
	nldestroyV2.exe --dir="\\ts01\transfer\temp" --count=10 --thread=2 --nodelete

If --nodelete option is specified, the generated files will not be delete.
Beware that there will be hundreds of generated files if --nodelete is called.

