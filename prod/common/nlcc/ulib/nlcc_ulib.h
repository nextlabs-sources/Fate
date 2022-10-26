
#ifndef __NLCC_ULIB_H__
#define __NLCC_ULIB_H__

#include <windows.h>
#include "nlcc.h"

/** NLCC_HANDLE
 *
 *  \brief NLCC handle object.
 */
typedef struct
{
  HANDLE h;  /* Handle to NLCC PEP device */
} NLCC_HANDLE, *PNLCC_HANDLE;

/** NLCC_Open
 *
 *  \brief Initialize a handle to NLCC.
 *  \return 0 on success, otherwise an -1.
 *
 *  \notes After a successful call to NLCC_Initialize the function NLCC_Close must be
 *         called.
 *
 *  \sa NLCC_Close
 */
_Check_return_
int NLCC_UOpen( _Out_ PNLCC_HANDLE in_handle );

/** NLCC_Close
 *
 *  \brief Close a handle to NLCC.
 *
 *  \sa NLCC_Initialize
 */
int NLCC_UClose( _In_ PNLCC_HANDLE in_handle );

/** NLCC_UInitializeQuery
 *
 *  \brief Initialize the given query object.
 */
void NLCC_UInitializeQuery( _Out_ PNLCC_QUERY in_query );

/** NLCC_Query
 *
 *  \brief Query the PDP for a decision and/or response.
 *
 *  \param in_request (in)     Request for policy decision.  This parameter must have been
 *                             initialized by calling NLCC_UInitializeQuery.
 *  \param out_response (out)  Response structure.
 *  \param in_timeout (in-opt) Timeout in milliseconds.  See WaitForSingleObject for details.
 *                             Wait semantics are identical to that function.  A value of 0
 *                             (zero) indicates that the call should be synchronous.
 *
 *  \return Status of the query.  A query can fail or be cancelled as a result of the timeout.
 *          If the timeout specified by in_timeout expires before a response (decision) is
 *          provided, then the return value will non-zero.  If the PDP (Policy Controller)
 *          is down, then the return value is also non-zero.
 *
 *          Additional error information may be retrieved by calling GetLastError.  Specifically,
 *          error conditions such as timeout (WAIT_TIMEOUT) and resource allocation
 *          (STATUS_INSUFFICIENT_RESOURCES) are included in addition to invalid parameter detection
 *          (ERROR_INVALID_PARAMETER).  An error value of ERROR_NOT_ENOUGH_MEMORY indicates there
 *          is not sufficient space in the query structure to pack the attribute.
 *
 *  \sa NLCC_UInitializeQuery
 */
int NLCC_UQuery( _In_ PNLCC_HANDLE in_handle ,
		 _In_ PNLCC_QUERY in_request ,
		 _Out_ PNLCC_QUERY out_response ,
		 _In_ size_t in_timeout );

/** NLCC_AddAttribute
 *
 *  \brief Add an attribute to a query.
 *
 *  \param in_query (in) Query object to add an attribute to.  This parameter must have been
 *                       initialized by calling NLCC_UInitializeQuery.
 *  \param in_key (in)   Key.  Must be non-NULL and non-empty.
 *  \param in_value (in) Value.  Must be non-NULL and non-empty.
 *
 *  \return 0 on success, otherwise non-zero.
 *
 *  \sa NLCC_UInitializeQuery, NLCC_UGetAttributeByIndex
 */
_Check_return_
int NLCC_UAddAttribute( _In_ PNLCC_QUERY in_query ,
			_In_ const wchar_t* in_key ,
			_In_ const wchar_t* in_value );

/** NLCC_UGetAttributeByIndex
 *
 *  \brief Retrieve an attribute's key and value given the index.
 *
 *  \param in_query (in)   Query object to get attribute from.  This parameter must have been
 *                         initialized by calling NLCC_UInitializeQuery.
 *  \param in_index (in)   Index to location.
 *  \param out_key (out)   Attribute's key.  A pointer to the address which contains a null
 *                         terminated string.
 *  \param out_value (out) Attribute's value.  A pointer to the address which contains a null
 *                         terminated string.
 *
 *  \return 0 on success, otherwise non-zero.  On failure the last error value will be one of
 *          the following:
 *            ERROR_INVALID_PARAMETER  - A given parameter is invalid.
 *            ERROR_NOT_FOUND          - There is no item at the given index.
 *
 *  \sa NLCC_UAddAttribute, NLCC_UInitializeQuery
 */
_Check_return_
int NLCC_UGetAttributeByIndex( _In_ const PNLCC_QUERY in_query ,
			       _In_ size_t in_index ,
			       _Inout_ const wchar_t** in_key ,
			       _Inout_ const wchar_t** in_value );

#endif /* __NLCC_ULIB_H__ */
