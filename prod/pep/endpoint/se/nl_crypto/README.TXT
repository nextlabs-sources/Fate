################################################################################
#
# NextLabs Crypto Module Interface
#
################################################################################

The crypto module provides an abstraction for symmetric encryption such that
the implementation of the encryption may be changed without compilation of the
interace user.  For example, a signed file system driver may use this interface
and change the encryption implementation without compilation of that driver
which may force it to be resigned.

    File System driver
           |
           |
      NL Crypto API

The above figure illustrates how a module can be interfaced behind the interface
of Crypto API.

File System Driver
------------------

nl_crypto_initialize   - Initialize the Crypto API interface and load a specific
                         module.

nl_crypto_shutdown     - Shutdown the Crypto API inteface and unload the specific
                         module loaded in nl_crypto_initialize.

A driver should call nl_crypto_initialize to instantiate an instance of the crypto
module it wishes to use.  The context of the initialize provides an interface to
call that module.  An example is given here:

DriverEntry(...)
{
  ...
  /* Initialize the crypto module */
  nl_crypto_context crypto;
  nl_crypto_initialize(&crypto,0);
  ...
}

PreWrite(...)
{
  ...
  /* Encrypt the callers buffer before commit to disk */
  crypto.encrypt(&cc,key,key_size,ivec,(unsigned char*)buf,buf_size);
  ...
}

Notes
-----
The build target is nl_crypto.sys.  This file needs to be renamed to nl_crypto.dll
and placed in system32/drivers and the nl_crypto.reg imported to define that driver.

(1) Build driver nl_crypt.sys
(2) Rename nl_crypto.sys to nl_crypto.dll
(3) Copy nl_crypto.dll to system32\drivers\
(4) Run nl_crypto.reg on the target machine.
