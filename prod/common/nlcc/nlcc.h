/******************************************************************************************
 *
 * NextLabs Communication Channel
 *
 * Common definitions - device names and data types
 *
 *****************************************************************************************/

#ifndef __NLCC_H__
#define __NLCC_H__

#define NLCC_VERSION 1

#define NLCC_QUERY_DEVICE  L"\\\\.\\NLCC_QUERY"
#define NLCC_QUERY_KDEVICE L"\\Device\\NLCC_QUERY"
#define NLCC_QUERY_SYMBOLIC_LINK_NAME  L"\\DosDevices\\NLCC_QUERY"
#define NLCC_QUERY_KSYMBOLIC_LINK_NAME L"\\DosDevices\\NLCC_QUERY"

#define NLCC_PDP_DEVICE  L"\\\\.\\NLCC_PDP"
#define NLCC_PDP_KDEVICE L"\\Device\\NLCC_PDP"
#define NLCC_PDP_DEVICE_SYMBOLIC_LINK_NAME  L"\\DosDevices\\NLCC_PDP"
#define NLCC_PDP_KDEVICE_SYMBOLIC_LINK_NAME L"\\DosDevices\\NLCC_PDP"

/* Attribute payload size (in bytes) */
#define NLCC_ATTRIBUTE_PAYLOAD_SIZE 8192

#define NLCC_POLICY_RESULT_DENY  0   /* Deny (explicit) */
#define NLCC_POLICY_RESULT_ALLOW 1   /* Allow (explicit) */
#define NLCC_POLICY_RESULT_NA    2   /* Not Applicable (nothing applies) */

/* Packing on 8-byte boundary */
#pragma pack(push,8)

/* NLCC_QUERY */
typedef struct
{
  int ip;                   /* IP address */
  long pid;                 /* Process ID */
  unsigned int event_level; /* Noise/Event Level */
} NLCC_QUERY_REQUEST, *PNLCC_QUERY_REQUEST;

/* NLCC_QUERY_RESPONSE */
typedef struct
{
  int allow;                /* allow */
} NLCC_QUERY_RESPONSE, *PNLCC_QUERY_RESPONSE;

typedef struct
{
  unsigned int size;        /* size (in bytes) of this structure */

  /* Request/Response*/
  union
  {
    NLCC_QUERY_REQUEST  request;
    NLCC_QUERY_RESPONSE response; 
  } info;

  /* Common */
  volatile LONGLONG tx_id;  /* Transaction ID */
  wchar_t attributes[NLCC_ATTRIBUTE_PAYLOAD_SIZE]; /* attributes */
} NLCC_QUERY, *PNLCC_QUERY;

/* Packing back to default */
#pragma pack(pop,8)

#endif /* __NLCC_H__ */
