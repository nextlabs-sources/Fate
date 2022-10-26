#include "LogMgr.h"
#include "pcstatusmgr.h"
#include "InstalledComptMgr.h"



/*

exported functions of this dll,

correspond to .def file.

implementation is in .cpp file

*/

BOOL GetLogMgr(CLogMgr** ppILogMgr);

BOOL GetPCStatusMgr(CPCStatusMgr** ppIPCStatusMgr);

BOOL GetInstalledComptMgr(CInstalledComptMgr** ppInstalledComptMgr);

BOOL GetNotifyMgr(CNotifyMgr** ppNotifyMgr);

BOOL GetDlgMgr(CDlgMgr** ppDlgMgr);
