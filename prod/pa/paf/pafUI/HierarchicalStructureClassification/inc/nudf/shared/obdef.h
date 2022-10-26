

#ifndef __NUDF_SHARE_OBLIGATION_DEF_H__
#define __NUDF_SHARE_OBLIGATION_DEF_H__


#ifdef __cplusplus
extern "C" {
#endif


//
//  Obligation Id
//
#define OB_ID_OVERLAY       0x0001
#define OB_ID_CLASSIFY      0x0002

//
//  Obligation Name
//
#define OB_NAME_OVERLAY     L"OB_OVERLAY"
#define OB_NAME_CLASSIFY    L"OB_CLASSIFY"

//
//  Obligation Parameter Name
//
#define OB_OVERLAY_PARAM_IMAGE          L"image"
#define OB_OVERLAY_PARAM_TRANSPARENCY   L"transparency"
#define OB_OVERLAY_PARAM_PLACEMENT      L"placement"
#define OB_CLASSIFY_PARAM_GROUP         L"group"


#pragma pack(push, 8)

typedef struct _NXRM_OBLIGATION {
    ULONG   NextOffset;
    USHORT  Id;
    WCHAR   Params[1];
} NXRM_OBLIGATION, *PNXRM_OBLIGATION;
typedef const NXRM_OBLIGATION* PCNXRM_OBLIGATION;

#pragma pack(pop)


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_OBLIGATION_DEF_H__