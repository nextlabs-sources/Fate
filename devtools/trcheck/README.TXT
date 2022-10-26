
History:

In the past is was identified through observation that Tamper Resistance would not
be stopped by the time the stop PC API has returned to its caller.  A change was
made in cepdpman.exe (PDPMan.cpp) to force the unloading of PC plug-ins around
end of 2009.  The plug-ins will be unloaded before the RPC call is returned from
the PDP context.

Summary:

This tool checks this by testing the read of a protected file while stopping the
PC.  If the file is *not* readable after the stop PC API has returned, then there
is a synchronization problem with Tamper Resistance.
