#ifndef NLSTRFUNCS_H
#define NLSTRFUNCS_H

/* Generic NextLabs strings interface to unify Window / Linux strings */
#include <string.h>
#include <string>


nlchar    *  nlstrcat   (nlchar *dest, const nlchar *src);
bool         nlstrcat_s (_Inout_z_cap_(len) nlchar *dest, _In_ size_t len, _In_z_ const nlchar *src);
nlchar    *  nlstrncat  (nlchar *dest, const nlchar *src, size_t len);
bool         nlstrncat_s(_Inout_z_cap_(destLen) nlchar *dest, size_t destLen, _In_z_ const nlchar *src, _In_ size_t len);
_Check_return_ _Ret_opt_z_ nlchar    *  nlstrchr   (_In_z_ const nlchar * str, _In_ nlint32 c);
_Check_return_ _Ret_opt_z_ nlchar    *  nlstrrchr  (_In_z_ const nlchar * str, _In_ nlint32 c);
_Check_return_ nlint32      nlstrcmp   (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2);
_Check_return_ nlint32      nlstrncmp  (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2, _In_ size_t len);
_Check_return_ nlint32      nlstricmp  (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2);
_Check_return_ nlint32      nlstrnicmp (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2, _In_ size_t len);
nlchar    *  nlstrcpy   (nlchar *dest, const nlchar *src);
bool         nlstrcpy_s (_Out_z_cap_(len) nlchar *dest, _In_ size_t len, _In_z_ const nlchar *src);
nlchar    *  nlstrncpy  (nlchar *dest, const nlchar *src, size_t len);
bool         nlstrncpy_s(_Out_z_cap_(destLen) nlchar *dest, _In_ size_t destLen, _In_z_ const nlchar *src, _In_ size_t len);
_Check_return_ _Ret_opt_z_ nlchar    *  nlstrdup   (_In_z_ const nlchar * str);
_Check_return_ _Ret_opt_z_ nlchar    *  nlstrstr   (_In_z_ const nlchar * str, _In_z_ const nlchar *substr);
bool         nlstrtoascii(_In_z_ const nlchar *nl_str, _Out_z_bytecap_(ascii_buf_size) char *asci_buf, _In_ unsigned int ascii_buf_size);
bool         nlstrplatformcpy(_Out_z_cap_(des_len) nlchar *dest, _In_z_count_(src_len) const nlchar *source, 
			      _In_ int &src_len, _In_ int src_byte_len, _In_ int des_len); 

/* Returning the length of the string in CHARACTER, not including*/
/* the terminating character                                     */
_Check_return_ size_t     nlstrlen   (_In_z_ const nlchar * str);

/* Returning the length of the string in BYTE, not including the */
/* terminating character                                         */
_Check_return_ size_t     nlstrblen  (_In_z_ const nlchar * str);

/* Convert a string to lower case. The destLength is the length of*/
/* destination string buffer; the lenght of source string doesn't */
/* include the terminal NULL.                                     */
void nlstrntolow(_Out_z_cap_(destLength) nlchar *dest, _In_ size_t destLength,
		 _In_z_count_(srcLength) const nlchar *src, _In_ size_t srcLength);

#endif //NLSTRFUNCS_H
