// ***************************************************************
//  IpcTests.cpp              version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  tests IPC functionality
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#include "SystemsTest.h"
#include <Systems.h>

void WINAPI IpcTestCallback(LPCSTR, LPCVOID messageBuffer, DWORD messageLength, LPCVOID answerBuffer, DWORD answerLength)
{
  bool b1 = true;
  int i1 = 0;
  for (i1 = 0; i1 < (int) messageLength / 4; i1++)
  {
    if ((((DWORD*) messageBuffer)[i1]) != ((DWORD) i1 ^ 0x77777777))
    {
      b1 = false;
      break;
    }
  }
  if (b1)
    for (i1 = 0; i1 < (int) answerLength / 4; i1++)
      ((DWORD*) answerBuffer)[i1] = (DWORD) i1 ^ 0x88888888;
}

DWORD arrIn[1024], arrOut[1024];

bool CheckIpc(int count)
{
  for (int i1 = 0; i1 < 1024; i1++)
  {
    arrIn[i1] = (DWORD) i1 ^ 0x77777777;
    arrOut[i1] = 0;
  }

  if (!SendIpcMessage("IpcTest", &arrIn, count * 4, &arrOut, count * 4))
    return false;

  for (int i1 = 0; i1 < count; i1++)
    if (arrOut[i1] != ((DWORD) i1 ^ 0x88888888))
      return false;

  return true;
}

bool TestIpc(void)
{
   bool result = false;
   wprintf(L"%S: Testing IPC functions\n", __FUNCTION__);

   if (CreateIpcQueue("IpcTest", IpcTestCallback))
   {
      if (CheckIpc(0))
      {
         if (CheckIpc(20))
         {
            if (DestroyIpcQueue("IpcTest"))
            {
               SetMadCHookOption(USE_NEW_IPC_LOGIC, NULL);
               if (CreateIpcQueue("IpcTest", IpcTestCallback))
               {
                  if (CheckIpc(0))
                  {
                     if (CheckIpc(1))
                     {
                        if (CheckIpc(60))
                        {
                           if (CheckIpc(1024))
                           {
                              if (DestroyIpcQueue("IpcTest"))
                              {
                                 wprintf(L"  SUCCESS\n\n");
                                 result = true;
                              }
                              else
                                 wprintf(L"  FAILURE - DestroyIpcQueue(2): %x\n\n", GetLastError());
                           }
                           else
                              wprintf(L"  FAILURE - Lpc(1024)\n\n");
                        }
                        else
                           wprintf(L"  FAILURE - Lpc(60)\n\n");
                     }
                     else
                        wprintf(L"  FAILURE - Lpc(1)\n\n");
                  }
                  else
                     wprintf(L"  FAILURE - Lpc(0)\n\n");
               }
               else
                  wprintf(L"  FAILURE - CreateIpcQueue(2): %x\n\n", GetLastError());
            }
            else
               wprintf(L"  FAILURE - DestroyIpcQueue(1): %x\n\n", GetLastError());
         }
         else
            wprintf(L"  FAILURE - AutoChoose(20)\n\n");
      }
      else
         wprintf(L"  FAILURE - AutoChoose(0)\n\n");
   }
   else
      wprintf(L"  FAILURE - CreateIpcQueue(1): %x\n\n", GetLastError());

   return result;
}