#ifndef __FTPCLI_H__
#define __FTPCLI_H__
//#ifdef __cplusplus
//extern "C"{
//#endif
 bool /*_stdcall*/ FtpUpload(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile);
//#ifdef __cplusplus
//}
//#endif


#endif //__FTPCLI_H__