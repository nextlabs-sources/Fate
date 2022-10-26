/*********************************************************************************
 *
 * NextLabs Device Enforcer (NLDevEnf)
 *
 * Evaluation Front-end.  Abstraction for SDK.
 *
 ********************************************************************************/

#ifndef __NL_DEVENF_EVAL_H__
#define __NL_DEVENF_EVAL_H__

typedef struct
{
  wchar_t bus_name[64];            /* Bus */
  wchar_t class_name[64];          /* Class */
  int id_vendor;                   /* Vendor (ID) */
  wchar_t id_vendor_string[64];    /* Vendor (string) */
  int id_product;                  /* Product (ID) */
  wchar_t id_product_string[64];   /* Product (string) */
  wchar_t serial_number[64];       /* Serial Number */
} RdeDevice;

/** PolicyInit
 *
 *  \brief Initialize the policy evaluation interface.
 *  \return true on success, otherwise false.
 */
bool PolicyInit(void);

/** PolicyEval
 *
 *  \brief Perform policy evaluation for a device resource.
 *
 *  \param resource_from (in) (Device) resource.
 *  \param device (in)        Device
 *  \param user_name (in)     User name for evaluation.
 *  \param user_sid (in)      User SID for evaluation.
 *  \param deny (out)         Result of evaluation.
 *
 *  \return false on error.
 */
bool PolicyEval( _In_ const wchar_t* resource_from ,
		 _In_ const RdeDevice* device ,
		 _In_ const wchar_t* user_name ,
		 _In_ const wchar_t* user_sid ,
		 bool& deny );

#endif /* __NL_DEVENF_EVAL_H__ */
