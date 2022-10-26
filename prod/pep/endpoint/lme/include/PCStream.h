#pragma once

//////////////////////////////////////////////////////////////////////////
//class CHookedStream: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedStream );
//    //INpwRC_Stream
//
//public:
//
//    void Hook( void* pStream );
//
//public:
//
//    typedef HRESULT (__stdcall* Old_Stream_read) (INpwRC_Stream*,unsigned char * buf,unsigned long n );
//    typedef HRESULT (__stdcall* Old_Stream_write) ( INpwRC_Stream*,unsigned char * buf,unsigned long n ) ;
//    typedef HRESULT (__stdcall* Old_Stream_reset) ( INpwRC_Stream*);
//
//    static HRESULT __stdcall New_Stream_read (INpwRC_Stream*,unsigned char * buf,unsigned long n );
//    static HRESULT __stdcall New_Stream_write ( INpwRC_Stream*,unsigned char * buf,unsigned long n ) ;
//    static HRESULT __stdcall New_Stream_reset (INpwRC_Stream* );
//};
//////////////////////////////////////////////////////////////////////////