
.SUFFIXES: .obj .cpp

WINDDK = c:/winddk/6000

LIBPATH  = \
	   /LIBPATH:"C:/Dev/tools/Microsoft Visual Studio 8/VC/lib" \
	   /LIBPATH:"C:/Dev/tools/Microsoft Visual Studio 8/VC/PlatformSDK/Lib" \
	   /LIBPATH:"C:/dev/tools/Microsoft Visual Studio 8/SDK/v2.0/lib" \
           /LIBPATH:$(WINDDK)/lib/wnet/i386 \
	   /LIBPATH:$(WINDDK)/lib/crt/i386

LIBS = libcpmt.lib

DEFINES  = \
	   -D_CRT_SECURE_NO_WARNINGS \
           -D_CRT_NON_CONFORMING_SWPRINTFS

INCLUDES = \
           /I$(WINDDK)/inc/api \
	   /I"C:/Dev/tools/Microsoft Visual Studio 8/VC/include" \
	   /I"C:/Dev/tools/Microsoft Visual Studio 8/VC/PlatformSDK/include"

CC     = cl
LD     = "C:/Dev/tools/Microsoft Visual Studio 8/VC/bin/link"
CFLAGS = /Zi /EHsc /MT /W3
LFLAGS = /DEBUG

PLUGIN_DRIVER_OBJS  = plugin_driver.obj
PLUGIN_LOGSERVER_OBJS = plugin_logserver.obj

all: plugin_logserver

plugin_logserver: $(PLUGIN_LOGSERVER_OBJS)
	$(LD) $(PLUGIN_LOGSERVER_OBJS) $(LIBS) $(LIBPATH) $(LFLAGS) /DLL /OUT:plugin_logserver.dll ws2_32.lib

.cpp.obj:
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $<

clean:
	@rm -f *.obj *.dll *.lib *.ilk *.exe *.exp *.pdb
