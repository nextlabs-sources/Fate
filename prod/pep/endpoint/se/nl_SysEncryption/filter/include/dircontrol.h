

#pragma once
#ifndef __NL_DIR_CONTROL_H__
#define __NL_DIR_CONTROL_H__

VOID
CheckFileBothDirectoryInformation(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in PFILE_BOTH_DIR_INFORMATION Info,
                                  __in BOOLEAN ProgramDir,
                                  __inout PNLDINFO DInfo
                                  );


VOID
CheckFileDirectoryInformation(
                              __in PFLT_FILTER Filter,
                              __in PFLT_INSTANCE Instance,
                              __in PFILE_DIRECTORY_INFORMATION Info,
                              __in BOOLEAN ProgramDir,
                              __inout PNLDINFO DInfo
                              );


VOID
CheckFileFullDirectoryInformation(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in PFILE_FULL_DIR_INFORMATION Info,
                                  __in BOOLEAN ProgramDir,
                                  __inout PNLDINFO DInfo
                                  );


VOID
CheckFileIdBothDirectoryInformation(
                                    __in PFLT_FILTER Filter,
                                    __in PFLT_INSTANCE Instance,
                                    __in PFILE_ID_BOTH_DIR_INFORMATION Info,
                                    __in BOOLEAN ProgramDir,
                                    __inout PNLDINFO DInfo
                                    );


VOID
CheckFileIdFullDirectoryInformation(
                                    __in PFLT_FILTER Filter,
                                    __in PFLT_INSTANCE Instance,
                                    __in PFILE_ID_FULL_DIR_INFORMATION Info,
                                    __in BOOLEAN ProgramDir,
                                    __inout PNLDINFO DInfo
                                    );


#endif