###########################################################################
#
# Policy Controller Service RPC
#
###########################################################################

Methods
-------
@idl method [method_name]( [param] ... )

@idl method TEST_Add( @idl param [in] (int*) x ,
                      @idl param [in] (int*) y ,
	              @idl param [out] (int) result )

Parameters
----------

@idl param [options] (type) [variable_name]

Parameter options:

in    - Input parameter
out   - Output parameter
alloc - Output parameter allocated by the callee

Examples:

@idl method TEST_Add( @idl param [in] (int*) x ,
                      @idl param [in] (int*) y ,
	              @idl param [out] (int) result )
