################################################################################
#
# NextLabs Crypto Module Implementation
#
################################################################################

Creating a Module
-----------------
A crypto module must implement the following interfaces defined in nl_crypto.h
These interface are used by the framework to communicate with the module to
perform encryption and generate keys.

Required:

  Implementation
  --------------
  create_key   Create a key.
  encrypt      Encrypt a buffer.
  decrypt      Decrypt a buffer.

  Exports
  -------
  nl_crypto_module_entry    Load and initialize the crypto module.
  nl_crypto_module_unload   Unload and uninitialize the crypto module.



