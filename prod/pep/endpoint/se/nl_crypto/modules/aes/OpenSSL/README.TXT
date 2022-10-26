###############################################################################
#
# NextLabs AES Crypto Module Implementation
#
###############################################################################

This is the NextLabs crypto module that uses AES.  It uses the AES
implementaion in OpenSSL 1.0.0d.

Files imported from OpenSSL
---------------------------
Below are the files that were imported from OpenSSL 1.0.0d:
- include/openssl/opensslconf.h
- include/openssl/aes.h
- include/openssl/e_os2.h
- include/openssl/modes.h
- include/aes_locl.h
- aes_x86core.c
- aes_cbc.c
- cbc128.c

Import procedure
----------------
Below is the procedure used to import the files from OpenSSL 1.0.0d:

1. Download openssl-1.0.0d.tar.gz from http://www.openssl.org/source/.
   Extract the files.
2. Copy crypto\aes\aes_locl.h to nl_crypto/modules/aes/OpenSSL/include
   in Perforce.
3. Copy crypto\aes\aes_x86core.c to nl_crypto/modules/aes/OpenSSL in
   Perforce.
4. Copy crypto\aes\aes_cbc.c to nl_crypto/modules/aes/OpenSSL in
   Perforce.
5. Copy crypto\modes\cbc128.c to nl_crypto/modules/aes/OpenSSL in
   Perforce.
6. Copy crypto\aes\aes.h to nl_crypto/modules/aes/OpenSSL/include/
   openssl in Perforce.
7. Copy e_os2.h to nl_crypto/modules/aes/OpenSSL/include/openssl in
   Perforce.
8. Copy crypto\modes\modes.h to nl_crypto/modules/aes/OpenSSL/include/
   openssl in Perforce.
9. Run "perl Configure VC-WIN32".
10. Copy crypto\opensslconf.h to nl_crypto/modules/aes/OpenSSL/include/
    openssl in Perforce.
