#pragma once

#include "NLProcess.h"

#define NLACTIONINSTANCE (CNLAction::NLGetInstance())

class CNLAction
{
private:
	CNLAction(void);
	~CNLAction(void);
public:
	static CNLAction& NLGetInstance();
	void Initialize( );
	void UnInitialize( );
	
public:
	ActionResult NLSetEventForXMLRibbon( _Inout_ STUNLOFFICE_RIBBONEVENT*	pribbonevent );
	ActionResult NLSetEventForCOMNotify( _Inout_ NotifyEvent*	pnotifyevent );
	ActionResult NLSetEventForHookEvent( _Inout_ HookEvent*	phookevent );
private:
	/*
	*\ Brief: this function is a common interface
	*\ Parameter
	*		[INOUT]		pEventStruct: it is a structure include the event information and the specific struct is decided by the second parameter emEventTriggerPoint
	*		[IN]				emEventTriggerPoint: this is a flag to tell us who triggered this event: XML ribbon, office COM notify or hook API
	*							this parameter can be set as the following values:
	*							1. emNLOfficeEventTriggerByXMLRibbon: if we set this flag, pEventStruct type is STUNLOFFICE_RIBBONEVENT   
	*							2. emNLOfficeEventTriggerByCOMNotify: if we set this flag, pEventStruct type is STUNLOFFICE_COMNOTIFYEVENT   
	*							3. emNLOfficeEventTriggerByHookEvent:      if we set this flag, pEventStruct type is STUNLOFFICE_HOOKEVENT
	*	Return value:	to see the instruction about EMNLOFFICE_RESULT
	*/		
	ActionResult NLSetEventCommon( _Inout_ void* pEventStruct, _In_ const EventType& emEventTriggerPoint );

	/* 
	*\ Brief:  this function used for XML ribbon button. It will get the action and set some flags from ribbon event 
	*\ Parameter
	*		[IN]		emRibbonEvent: notify which ribbon button clicked
	*		[IN]		emDesFileType: the destination file  default type, only used for office 2007 save as action, others we should use the default value: emOfficeUnknownType 
	*\	Return value: to see STUNLOFFICE_RESULT description
	*/
	ProcessResult NLSetEventFromXMLRibbonButton( _In_ const RibbonEvent& emRibbonEvent, _In_ const UNNLOFFICE_EVENTEXTRAFLAG& unOfficeEventExtraType );
	
private:
	CNLProcess* m_process;		// used to process the actions
};
