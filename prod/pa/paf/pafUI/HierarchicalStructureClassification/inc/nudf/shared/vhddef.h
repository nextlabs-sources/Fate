

#ifndef __NXRM_VHD_DEF_H__
#define __NXRM_VHD_DEF_H__



#ifdef __cplusplus
extern "C" {
#endif


#define NXRMVHD_WIN32_DEVICE_NAME_A	    "\\\\.\\NxRmVhd"
#define NXRMVHD_WIN32_DEVICE_NAME_W	    L"\\\\.\\NxRmVhd"
#define NXRMVHD_DEVICE_NAME_A           "\\Device\\NxRmVhd"
#define NXRMVHD_DEVICE_NAME_W           L"\\Device\\NxRmVhd"
#define NXRMVHD_DOS_DEVICE_NAME_A       "\\DosDevices\\NxRmVhd"
#define NXRMVHD_DOS_DEVICE_NAME_W       L"\\DosDevices\\NxRmVhd"

#define DOS_MOUNT_PREFIX	            L"\\DosDevices\\"
#define NT_MOUNT_PREFIX		            L"\\Device\\NsVolume"


#define NXRMVHD_UNIQUE_ID_PREFIX_A      "NsVolume"
#define NXRMVHD_UNIQUE_ID_PREFIX_W      L"NsVolume"
#define NXRMVHD_UNIQUE_ID_PREFIX_LENGTH ((sizeof(NXRMVHD_UNIQUE_ID_PREFIX_A) / sizeof(CHAR)) - 1)

#define FILE_DEVICE_NXRMVHD             0xC000
#define NXRMVHD_IOCTL_BASE              0x0C00
#define IOCTL_NXRMVHD_MOUNT_DISK        ((ULONG)CTL_CODE(FILE_DEVICE_NXRMVHD, NXRMVHD_IOCTL_BASE+7, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define IOCTL_NXRMVHD_UNMOUNT_DISK      ((ULONG)CTL_CODE(FILE_DEVICE_NXRMVHD, NXRMVHD_IOCTL_BASE+17, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define IOCTL_NXRMVHD_QUERY_DISK        ((ULONG)CTL_CODE(FILE_DEVICE_NXRMVHD, NXRMVHD_IOCTL_BASE+20, METHOD_BUFFERED, FILE_READ_ACCESS))


#define BYTES_PER_KB                    1024LL
#define BYTES_PER_MB                    1048576LL
#define BYTES_PER_GB                    1073741824LL
#define BYTES_PER_TB                    1099511627776LL
#define BYTES_PER_PB                    1125899906842624LL

#define NXRMVHD_MAX_PATH			    268
#define NXRMVHD_SECTOR_SIZE_LEGACY	    512
#define NXRMVHD_MIN_VOLUME_SIZE         (37 * NXRMVHD_SECTOR_SIZE_LEGACY)
#define NXRMVHD_MAX_VOLUME_SIZE         BYTES_PER_PB

#define NXRMVHD_UNMOUNTALL			    0xFFFFFFFF


#define MAX_DOSDISK                     24  // Drive Letter C ==> Z ("CDEFGHIJKLMNOPQRSTUVWXYZ")
#define MAX_ACTIVEDISK                  64  // At most we can have 64 active VHD at the same time

#define NXRMVHDTAG                      'dhVN'
#define NXRMVHITAG                      'IhVN'
#define NXRMTMPTAG                      '**VN'



#define VHDFMT_FILE_MAGIC           'fdVN'
#define VHDFMT_FILE_VERSION         0x00010000

#define VHDFMT_BLOCK_SIZE           ((ULONG)0x200000)           //  2 MB
#define VHDFMT_SECTOR_SIZE          0x1000                      //  4 KB

#define VHDFMT_MIN_DISK_SIZE        ((LONGLONG)0x200000)        //  2MB
#define VHDFMT_MAX_DISK_SIZE_FAT    ((LONGLONG)0x7FF00000)      //  2046 MB
#define VHDFMT_MAX_DISK_SIZE        ((LONGLONG)0x780000000)     //  30 GB
#define VHDFMT_MAX_BLOCK_COUNT_FAT  1023                        //  1023 (0x3FF) Blocks, up to 2046 MB
#define VHDFMT_MAX_BLOCK_COUNT      15360                       //  15K (0x3C00) Blocks, up to 30 GB

#define VHDFMT_BLOCKTABLE_START     0x1000                      //  4KB
#define VHDFMT_BLOCK_START          ((ULONG)0x200000)           //  2 MB


#define BLOCK_STATE_ALLOCING        0x00000001
#define BLOCK_STATE_ALLOCED         0x00000002
#define BLOCK_STATE_INITING         0x00000004
#define BLOCK_STATE_INITED          0x00000008


#pragma pack(push, 4)

typedef struct _NXRMVHDMOUNTDRIVE {
	ULONG		BytesPerSector;
	BOOLEAN		Removable;
	BOOLEAN		Visible;
	WCHAR		PreferredDriveLetter;
    UCHAR       Key[32];
	WCHAR		HostFileName[NXRMVHD_MAX_PATH];
} NXRMVHDMOUNTDRIVE, *PNXRMVHDMOUNTDRIVE;

typedef struct _NXRMVHDUNMOUNTDRIVE {
	ULONG		DiskId;
} NXRMVHDUNMOUNTDRIVE,*PNXRMVHDUNMOUNTDRIVE;

typedef struct _NXRMVHDINFO {
    ULONG       DiskId;
    WCHAR       DriveLetter;
    ULONG       Removable;
    ULONG       Visible;
    WCHAR       VolumeName[128];
	WCHAR		HostFileName[NXRMVHD_MAX_PATH];
} NXRMVHDINFO, *PNXRMVHDINFO;

typedef struct _NXRMVHDINFOS {
    ULONG       Count;
    NXRMVHDINFO Infs[1];
} NXRMVHDINFOS, *PNXRMVHDINFOS;


//
//  VHD File Format
//

//      [Offset]        [Size]          [Data]
//      0               512             Header
//      512             512             Reserved
//      1024            512             Copy of Header
//      1536            512             Reserved
//      2048            2048            Reserved
//      4096            61440 (60KB)    BlockTable (At most 15360 <15K> Blocks, 32G)
//      65536 (64KB)    ?               Blocks
//

typedef struct _VHDFILEHEADER {
    ULONG           Magic;
    ULONG           Version;
    UCHAR           UniqueId[16];
    LONGLONG        DiskSpace;
    UCHAR           KeyId[64];      // Id of KEK (SHA1/SHA256/NextLabs)
    UCHAR           KeyBlob[32];    // Encrypted CEK (AES256)
} VHDFILEHEADER, *PVHDFILEHEADER;
typedef const VHDFILEHEADER* PCVHDFILEHEADER;

typedef union _VHDFILEHEADER_BLOB {
    VHDFILEHEADER   Header;
    UCHAR           Buffer[512];
} VHDFILEHEADER_BLOB;

typedef struct _VHDBAT {
    ULONG           BlockInfo[VHDFMT_MAX_BLOCK_COUNT];
} VHDBAT, *PVHDBAT;


#pragma pack(pop)



__forceinline
VOID
PositionToBlockInfo(
                    _In_ LONGLONG Position,
                    _Out_ PULONG BlockId,
                    _Out_ PULONG OffsetInBlock
                    )
{
    *BlockId = (ULONG)(Position / VHDFMT_BLOCK_SIZE);
    *OffsetInBlock = (ULONG)(Position % VHDFMT_BLOCK_SIZE);
}

__forceinline
LONGLONG
SeqIdToBlockOffset(
                   _In_ ULONG SeqId
                   )
{
    LARGE_INTEGER Offset = {0, 0};
    Offset.LowPart   = SeqId;
    Offset.QuadPart *= VHDFMT_BLOCK_SIZE;
    Offset.QuadPart += VHDFMT_BLOCK_START;
    return Offset.QuadPart;
}

__forceinline
ULONG
VhdGetBlockState(
                 _In_ PVHDBAT Bat,
                 _In_ ULONG Id
                 )
{
    ASSERT(Id < VHDFMT_MAX_BLOCK_COUNT);
    return ((Bat->BlockInfo[Id] >> 16) & 0x0000FFFF);
}

__forceinline
ULONG
VhdGetBlockSeqId(
                 _In_ PVHDBAT Bat,
                 _In_ ULONG Id
                 )
{
    return (Bat->BlockInfo[Id] & 0x0000FFFF);
}

__forceinline
ULONG
VhdSetBlockState(
                 _In_ PVHDBAT Bat,
                 _In_ ULONG Id,
                 _In_ ULONG State
                 )
{
    ASSERT(Id < VHDFMT_MAX_BLOCK_COUNT);
    Bat->BlockInfo[Id]  |= (State << 16);
    return ((Bat->BlockInfo[Id] >> 16) & 0x0000FFFF);
}

__forceinline
ULONG
VhdUnsetBlockState(
                 _In_ PVHDBAT Bat,
                 _In_ ULONG Id,
                 _In_ ULONG State
                 )
{
    ASSERT(Id < VHDFMT_MAX_BLOCK_COUNT);
    Bat->BlockInfo[Id]  &= (~(State << 16));
    return ((Bat->BlockInfo[Id] >> 16) & 0x0000FFFF);
}

__forceinline
VOID
VhdSetBlockSeqId(
                 _In_ PVHDBAT Bat,
                 _In_ ULONG Id,
                 _In_ ULONG SeqId
                 )
{
    ASSERT(Id < VHDFMT_MAX_BLOCK_COUNT);
    ASSERT(SeqId < VHDFMT_MAX_BLOCK_COUNT);
    Bat->BlockInfo[Id] &= 0xFFFF0000;
    Bat->BlockInfo[Id] |= (SeqId & 0x0000FFFF);
}




#ifdef __cplusplus
}
#endif


#endif