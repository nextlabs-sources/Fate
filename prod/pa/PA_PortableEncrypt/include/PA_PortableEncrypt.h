#pragma once

typedef enum
{
	DEFAULT= 0,
	PE_AUTOWRAP = 1 
} PE_ObligationType;

typedef struct _PE_Obligation
{
	PE_ObligationType obType ;
	std::wstring		wstrLogId;
} PE_Obligation, *LPE_Obligation;


PA_STATUS ParseObligationList(OBJECTINFO &file, PE_Obligation &obligation) ;
PA_STATUS WINAPI DoAutoWrap ( PE_Obligation &objData ) ;