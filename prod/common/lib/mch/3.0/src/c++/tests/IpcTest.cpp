// ***************************************************************
//  IpcTest.cpp               version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  tool to test IPC functionality
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// Tests the pipe or LPC interprocess communication functions.  To test,
// start IPCTest.exe with no parameters in one command window, then
// start it in another command window with parameter 'm' (for master).
// The master will start sending messages using the selected IPC
// transport to the other IPCTest process.  As soon as the first
// process receives a message from the master, it spawns a thread
// to send 100 messages back to the master.

// The function SendTestMessage constructs a message to send and
// sends it using _SendMessage.  _SendMessage sends the message
// then checks the integrity of the reply by confirming certain
// fields are set in the answer.

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Systems.h>
#include <crtdbg.h>

#include "SString.h"

const int TEST_MESSAGE_SIGNATURE = 0xDEADBEEF;
const int ANSWER_MESSAGE_SIGNATURE = 0xABABABAB;
const LPCH TEST_MESSAGE_TEXT = "TEST_MESSAGE";
const LPCH ANSWER_MESSAGE_TEXT = "ANSWER";

BOOL gMaster = TRUE;
BOOL gSlaveThreadCreated = FALSE;
char me[MAX_PATH];
char target[MAX_PATH];

// test message 128 long
typedef struct tagStressTestMessage
{
  DWORD Signature;
  int ID;
  LPCH Message[120];
} STRESS_TEST_MESSAGE, *PSTRESS_TEST_MESSAGE;

// reply to test message 32 long
typedef struct tagStressTestAnswer
{
  DWORD Signature;
  int ReplyID;
  LPCH ReplyMessage[24];
} STRESS_TEST_ANSWER, *PSTRESS_TEST_ANSWER;

BOOL SendTestMessage(LPCSTR target, int id);
BOOL _SendMessage(LPCSTR target, STRESS_TEST_MESSAGE *pMsg);

int SlaveThreadProc(LPVOID o)
{
  printf("--Slave write thread starting--\n");
  for (int i = 0; i < 100; i++)
  {
    if (!SendTestMessage(target, rand()))
    {
      printf("SendTestMessage failed on iteration %d\n", i);
      printf("--Slave write thread ended after error--\n");
      return 0;
    }
    else
      printf("Slave sending message %d to master\n", i);
  }
  printf("--Slave write thread ended normally--\n");
  return 0;
}

void WINAPI MasterCallback(LPCSTR name, LPCVOID messageBuffer, DWORD messageLength, LPCVOID answerBuffer, DWORD answerLength)
{
  printf("Callback: Message received from slave\n");
  PSTRESS_TEST_MESSAGE msg = (PSTRESS_TEST_MESSAGE) messageBuffer;
  PSTRESS_TEST_ANSWER answer = (PSTRESS_TEST_ANSWER) answerBuffer;

  _ASSERT(msg->Signature == TEST_MESSAGE_SIGNATURE);
  _ASSERT(strcmp((char *) msg->Message, TEST_MESSAGE_TEXT) == 0);

  ZeroMemory(answer, sizeof(*answer));
  answer->ReplyID = msg->ID + 1;
  answer->Signature = ANSWER_MESSAGE_SIGNATURE;
  strcpy((char *) answer->ReplyMessage, ANSWER_MESSAGE_TEXT);
}

void WINAPI SlaveCallback(LPCSTR name, LPCVOID messageBuffer, DWORD messageLength, LPCVOID answerBuffer, DWORD answerLength)
{
//  printf("Callback: Message received from master\n");

  PSTRESS_TEST_MESSAGE msg = (PSTRESS_TEST_MESSAGE) messageBuffer;
  PSTRESS_TEST_ANSWER answer = (PSTRESS_TEST_ANSWER) answerBuffer;

  _ASSERT(msg->Signature == TEST_MESSAGE_SIGNATURE);
  _ASSERT(strcmp((char *) msg->Message, TEST_MESSAGE_TEXT) == 0);

  ZeroMemory(answer, sizeof(*answer));
  answer->ReplyID = msg->ID + 1;
  answer->Signature = ANSWER_MESSAGE_SIGNATURE;
  strcpy((char *) answer->ReplyMessage, ANSWER_MESSAGE_TEXT);


  if (!gSlaveThreadCreated)
  {
    DWORD newThreadId;
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &SlaveThreadProc, NULL, 0, &newThreadId);

    gSlaveThreadCreated = TRUE;
  }
}

BOOL SendTestMessage(LPCSTR target, int id)
{
  STRESS_TEST_MESSAGE msg;
  ZeroMemory(&msg, sizeof(msg));
  msg.Signature = TEST_MESSAGE_SIGNATURE;
  msg.ID = id;
  strcpy((char *) msg.Message, TEST_MESSAGE_TEXT);
  return _SendMessage(target, &msg);
}

BOOL _SendMessage(LPCSTR target, STRESS_TEST_MESSAGE *pMsg)
{
  BOOL result = FALSE;
  STRESS_TEST_ANSWER answer;
  ZeroMemory(&answer, sizeof(answer));

  if (SendIpcMessage(target, pMsg, sizeof(*pMsg), &answer, sizeof(answer), 5000, TRUE))
  {
    if (ANSWER_MESSAGE_SIGNATURE == answer.Signature)
    {
      // answer has correct signature
      if ((answer.ReplyID != pMsg->ID + 1) || (strcmp((char *) answer.ReplyMessage, ANSWER_MESSAGE_TEXT) != 0))
      {
        printf("Answer id or text didn't match\n");
      }
      else
        result = TRUE;
    }
    else
      printf("Answer received with incorrect signature %x\n", answer.Signature);
  }
  else
    printf("SendIpcMessage returned FALSE\n");
  return result;
}

int _tmain(int argc, char *argv[])
{
  srand(GetTickCount());

  // force the IPC library to use LPC
  printf("Setting IPC transport to LPC\n");
  SetMadCHookOption(USE_NEW_IPC_LOGIC, NULL);

  if (argc > 1 && (strncmp(argv[1], "m", 1) == 0 || strncmp(argv[1], "M", 1) == 0))
  {
    printf("Running as the master.  I will start sending to the slave when it comes up\n");
    strcpy_s(me, MAX_PATH, "Session_Master");
    strcpy_s(target, MAX_PATH, "Session_Slave");
  }
  else
  {
    printf("Running as the slave.  The master should start the stress test to me when I come up\n");
    strcpy_s(me, MAX_PATH, "Session_Slave");
    strcpy_s(target, MAX_PATH, "Session_Master");
    gMaster = FALSE;
  }

  printf("Starting IPC test...\n");

  if (gMaster)
  {
    BOOL created = CreateIpcQueueEx(me, (PFN_IPC_CALLBACK) MasterCallback, 1, 0x1000);
    printf("Master queue created\n");

    printf("Trying for 10 seoonds to locate the slave...\n");
    int i = 0;

    while (i < 10)
    {
      if (!SendTestMessage(target, 0))
      {
        Sleep(1000);
        i++;
      }
      else
        break;
    }

    if (i >= 10)
    {
      printf("The slave did not come up in 10 seconds.  Terminating test\n");
      return 0;
    }

    printf("Slave located.  Starting stress test...\n");

    int count = 10000;
    i = 0;
    while (i++ < count)
    {
      if (!SendTestMessage(target, rand()))
      {
        printf("SendTestMessage failed on iteration %d\n", i);
        return 0;
      }
      printf("Sent message %d...\n", i);
    }
    printf("%d test messages sent to slave\n", count);
  }
  else
  {
    BOOL created = CreateIpcQueue(me, (PFN_IPC_CALLBACK) SlaveCallback);
    printf("Slave queue created\n");
  }

  printf("\nPress any key to exit\n");
  getchar();

  DestroyIpcQueue(me);
  printf("Queue destroyed\n");
  return 0;
}