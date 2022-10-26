
#ifndef __NLCC_KLIB_H__
#define __NLCC_KLIB_H__

#include <nlcc.h>

/** NLCC_HANDLE
 *
 *  \brief Handle to an NLCC connection.
 */
typedef struct
{
  PDEVICE_OBJECT dev_obj;
  PFILE_OBJECT fo_obj;
} NLCC_HANDLE, *PNLCC_HANDLE;

/** NLCC_KOpen
 *
 *  \brief Initialize a handle to NLCC.
 *  \return STATUS_SUCCESS on success, otherwise an error value.
 *
 *  \notes After a successful call to NLCC_Initialize the function NLCC_Close must be
 *         called.
 *
 *  \sa NLCC_Close
 */
__drv_maxIRQL(PASSIVE_LEVEL)
__checkReturn
NTSTATUS NLCC_KOpen( __in PNLCC_HANDLE in_handle );

/** NLCC_KClose
 *
 *  \brief Close a handle to NLCC.
 *
 *  \sa NLCC_Initialize
 */
__drv_maxIRQL(APC_LEVEL)
NTSTATUS NLCC_KClose( __in PNLCC_HANDLE in_handle );

/** NLCC_KQuery
 *
 *  \brief Query the PDP for a decision and/or response.
 *
 *  \param in_request (in)     Request for policy decision.
 *  \param out_response (out)  Response structure.
 *  \param in_timeout (in-opt) Timeout in 100-nanosecond units.  See KeWaitForSingleObject for
 *                             details.  Wait semantics are identical to that function.
 *
 *  \return Status of the query.  A query can fail or be cancelled as a result of the timeout.
 *          If the timeout specified by in_timeout expires before a response (decision) is
 *          provided, then the return value will be STATUS_TIMEOUT.  If the PDP (Policy Controller)
 *          is down, then the return value is STATUS_INSUFFICIENT_RESOURCES. 
 */
__drv_maxIRQL(APC_LEVEL)
NTSTATUS NLCC_KQuery( __in PNLCC_HANDLE in_handle ,
		      __in PNLCC_QUERY in_request ,
		      __in PNLCC_QUERY out_response ,
		      __in_opt PLARGE_INTEGER in_timeout );

/** NLCC_KInitializeQuery
 *
 *  \brief Initialize an instance of a NLCC_QUERY structure.
 */
VOID NLCC_KInitializeQuery( __in PNLCC_QUERY in_query );

/** NLCC_KAddAttribute
 *
 *  \brief Add an attribute to a query object.
 *
 *  \param in_query (in) Query object.
 *  \param in_key (in)   Key name.
 *  \param in_value (in) Key value.
 *
 *  \return STATUS_SUCCESS on success, otherwise some error value.
 *
 *  \notes This may be called at any IRQL level provided the memory used for storage of
 *         all the parameters is non-pageable.
 */
NTSTATUS NLCC_KAddAttribute( __in PNLCC_QUERY in_query ,
			     __in const PUNICODE_STRING in_key ,
			     __in const PUNICODE_STRING in_value );

#endif /* __NLCC_KLIB_H__ */
