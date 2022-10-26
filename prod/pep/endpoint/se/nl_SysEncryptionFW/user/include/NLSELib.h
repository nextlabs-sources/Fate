
#ifndef __NLSE_LIB_H__
#define __NLSE_LIB_H__

#include <windows.h>
#include <tchar.h>

/** NLSEUserCmd_EnableFilter
 *
 *  \brief Enable NLSE filter driver functionality
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_EnableFilter();

/** NLSEUserCmd_DisableFilter
 *
 *  \brief Disable NLSE filter driver functionality
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_DisableFilter();

/** NLSEUserCmd_SetIgnoredProcessByPID
 *
 *  \brief tell kernel not to handle any i/o operations of a process by PID 
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_SetIgnoreProcessByPID(ULONG pid);

/** NLSEUserCmd_UnsetIgnoredProcessByPID
 *
 *  \brief tell kernel not to ignore a process by PID 
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_UnsetIgnoreProcessByPID(ULONG pid);


#endif /* __NLSE_LIB_H__ */
