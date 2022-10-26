
Compile Time Controls
---------------------

NLSE_DEBUG_FAKE_FILE_KEY		When defined will force the driver to use a fixed file key with
					a value of 'z' for all bytes of the AES-128 key.

NLSE_DEBUG_FAKE_PC_KEY			When defined will force the Policy Controller key to be defined
					with a value of 'a' for all bytes of the AES-128 key.

NLSE_DEBUG_DATA_VERIFY			When defined this will force the encryption to set all values of
					all bytes to the value 'b'.  On decryption the value will be checked
					against the value 'b'.  If any byte of data does not containt the
					value 'b', then the driver will force a 0xBEEFFEED
					bug check with the following parameters:
					    (1) Address of memory block
					    (2) Offset of mismatch into the memory block
					    (3) Offset of file for which this memory block is used
					    (4) Value of unexpected byte

NLSE_DEBUG_DATA_VERIFY_BUGCHECK         When defined the driver will force a bug check for verify failure
					in cases where NLSE_DEBUG_DATA_VERIFY has been defined.

NLSE_DEBUG_CRYPTO_PASSTHROUGH		When define encyrption is disabled.  The crypto API behaves as
					passthrough and data is not modified.
