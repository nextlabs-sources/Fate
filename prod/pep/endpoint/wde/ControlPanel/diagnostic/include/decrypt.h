#pragma once

class CDecryptBundle
{
public:

	/*
	
	decrypt bundle.

	you need to stop pc first, and provide correct password in order to stop pc

	
	*/
	static BOOL Decrypt(const wstring& strPassword);


	/*

	decrypt bundle via tool.

	you need to stop pc first, and provide correct password in order to stop pc


	*/
	static BOOL DecryptViaTool(const wstring& strPassword);



	/*
	
	be used with User Account Control
	
	
	*/
	static BOOL DecryptWithEnh(const wstring& strPassword);
};