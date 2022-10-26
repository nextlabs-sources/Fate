// Key Management Utility
//

#include "stdafx.h"

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#define WIN32_LEAN_AND_MEAN
//#define VC_EXTRALEAN

// C++ standard include
#include <iostream>
#include <vector>
#include <fstream> // for key ring import / export
#include <string>
// for cryptography e.g. AES
#include <windows.h>
#include <Wincrypt.h>

using namespace std;

// NextLabs SDK
#include <cetype.h>
#include <cesdk.h>
#include <ceservice.h>

// Key Management SDK
#include "KeyManagementConsumer.h"
#include "KeyManagementConsumerPrivate.h"

// global variables
DWORD mCurrentProcessID = 0;
CEHandle mHandlePC;
int mTimeout = 30000; // 30 seconds
bool mAlwaysYes = false;
HCRYPTPROV ghCryptProv = NULL;
HCRYPTKEY ghKey = NULL;

#pragma warning( push )
#pragma warning( disable : 4245 )
static const unsigned char kmcPassword[] = {7, -117, 34, -79, -74, 85, -10, -63, -99, -120, 103, 15, -48, -46, -8, -88};
#pragma warning( pop )
#define KMCPASSWORD_LEN ((sizeof (kmcPassword))/(sizeof (unsigned char)))

// create new key ring named keyRingName
CEResult_t createKeyRing(wchar_t *keyRingName)
{
	CEString sKeyRingName = CEM_AllocateString(keyRingName);
	CEResult_t res = CEKey_CreateKeyRing(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, mCurrentProcessID, mTimeout);

	// free sKeyRingName
	CEM_FreeString(sKeyRingName);

	// print result
	if (res == CE_RESULT_SUCCESS) {
		wcout << L"Succeeded creating key ring: " << keyRingName << L"\n";
	}

	return res;
}

// generate key in the key ring specified by keyRingName
// keylen is in bytes
CEResult_t generateKey(wchar_t *keyRingName, int keylen)
{
	CEKeyID keyID;
	CEString sKeyRingName = CEM_AllocateString(keyRingName);
	CEResult_t res = CEKey_GenerateKey(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, keylen, &keyID, mCurrentProcessID, mTimeout);

	// print result
	if (res == CE_RESULT_SUCCESS) {
			// hexify hash
            CEString hash = hexifyHash(&keyID);
			// hexify timestamp
			CEString sTimestamp = hexify((unsigned char *)&keyID.timestamp, 4);
            wcout << L"Successfully generated new key: " << CEM_GetString(hash) << CEM_GetString(sTimestamp) << std::endl;
            CEM_FreeString(hash);
			CEM_FreeString(sTimestamp);
	}

	// clean up
	CEM_FreeString(sKeyRingName);

	return res;
}

// list key rings
CEResult_t listKeyRing()
{
	CEResult_t res = CE_RESULT_SUCCESS;
	CEKeyRing *keyRings = NULL;
	CEint32 size = 0;

	// first call to ListKeyRings.  NULL in key ring output parameter to get the size.
	res = CEKey_ListKeyRings(mHandlePC, kmcPassword, KMCPASSWORD_LEN, NULL, &size, mCurrentProcessID, mTimeout);
	// I expect insufficient buffer error because buffer hasn't been allocated. 
	// if I get SUCCESS as a result, that means there's no data.  Return immediately.
	// if I get error, return immediately, too.
	if (res != CE_RESULT_INSUFFICIENT_BUFFER)
		goto end;

	// allocate buffer
	keyRings = new CEKeyRing[size];
	res = CEKey_ListKeyRings(mHandlePC, kmcPassword, KMCPASSWORD_LEN, keyRings, &size, mCurrentProcessID, mTimeout);

	// print result
	if (res == CE_RESULT_SUCCESS) {
		wcout << L"Succeeded in obtaining key ring list\n";
		for (int i=0; i<size; i++) {
			wcout << CEM_GetString(keyRings[i].name) << L"\n";
		}
	}

end:
	// clean up
	if (keyRings != NULL) 
		delete[] keyRings;

	return res;
}

// list keys
CEResult_t listKey(wchar_t *keyRingName)
{
	CEResult_t res = CE_RESULT_SUCCESS;
	CEKeyRing keyRing;
	CEString sKeyRingName = CEM_AllocateString(keyRingName);

	// first call to ListKeys.  In keyRing, list is empty and size is 0.  This is to get the size of the list.
	keyRing.size = 0;
	keyRing.keyIDs = NULL;
	keyRing.name = sKeyRingName;

	res = CEKey_ListKeys(mHandlePC, kmcPassword, KMCPASSWORD_LEN, &keyRing, mCurrentProcessID, mTimeout);
	// I expect insufficient buffer error because buffer hasn't been allocated. 
	// if I get SUCCESS as a result, that means there's no data.  Return immediately.
	// if I get error, return immediately, too.
	if (res != CE_RESULT_INSUFFICIENT_BUFFER)
		goto end;

	// allocate buffer
	keyRing.keyIDs = new CEKeyID[keyRing.size];

	// second call to get the list
	res = CEKey_ListKeys(mHandlePC, kmcPassword, KMCPASSWORD_LEN, &keyRing, mCurrentProcessID, mTimeout);

	// print result
	if (res == CE_RESULT_SUCCESS) {
		wcout << L"Succeeded in obtaining key list for ring: " << keyRingName << L"\n";
		
		for (int i=0; i<keyRing.size; i++) {
					// hexify hash
                    CEString hash = hexifyHash(&keyRing.keyIDs[i]);
					// hexify timestamp
					CEString sTimestamp = hexify((unsigned char *)&keyRing.keyIDs[i].timestamp, 4);
                    wcout << L"Key " << i << L" ID: " << CEM_GetString(hash) << CEM_GetString(sTimestamp) << endl;
                    CEM_FreeString(hash);
					CEM_FreeString(sTimestamp);
		}
		
	}

end:
	// clean up
	if (keyRing.keyIDs != NULL) 
		delete[] keyRing.keyIDs;
	if (sKeyRingName != NULL)
		CEM_FreeString(sKeyRingName);

	return res;
}

bool areYouSure()
{
	wchar_t yesOrNo;
	wcin >> yesOrNo;
	if (yesOrNo == L'y' || yesOrNo == L'Y') 
		return true;
	else 
		return false;
}

// delete key ring
CEResult_t deleteKeyRing(wchar_t *keyRingName)
{
	CEResult_t res = CE_RESULT_SUCCESS;
	CEString sKeyRingName = CEM_AllocateString(keyRingName);

	if (mAlwaysYes == false) {
	    // are you sure?
	    wcout << L"Are you sure you want to delete all keys in key ring " << keyRingName << L"?  All data encrypted with these keys will be undecryptable.  Enter y/n ?\n"; 
	    if (areYouSure() == true) {
		wcout << "Delete\n";
	    } else {
		wcout << "Don't delete\n";
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	    }
	}

	res = CEKey_DeleteKeyRing(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, mCurrentProcessID, mTimeout);
	// print result
	if (res == CE_RESULT_SUCCESS) {
		wcout << L"Succeeded in deleting key ring: " << keyRingName << L"\n";		
	}

end:
	if (sKeyRingName != NULL)
		CEM_FreeString(sKeyRingName);

	return res;
}

// delete key 
CEResult_t deleteKey(wchar_t *keyRingName, wchar_t *keyIDInput)
{
	CEResult_t res = CE_RESULT_SUCCESS;
	CEString sKeyRingName = CEM_AllocateString(keyRingName);
	CEKeyRing keyRing;
	keyRing.keyIDs = NULL;
	wchar_t keyIDFromNum[KM_HASH_LEN*2 + 4*2 + 1], *keyID = keyIDInput;
	
	// if it's key number (e.g. 0, 1, 2, ...), convert it to key ID (n bytes)
	unsigned int keyNum = 0;
	if (wcslen(keyIDInput) < 15) {// it's a number
	    keyNum = _wtoi(keyIDInput);

	    // get list of keys
	    // first call to ListKeys.  In keyRing, list is empty and size is 0.  This is to get the size of the list.
	    keyRing.size = 0;
	    keyRing.keyIDs = NULL;
	    keyRing.name = sKeyRingName;
	    res = CEKey_ListKeys(mHandlePC, kmcPassword, KMCPASSWORD_LEN, &keyRing, mCurrentProcessID, mTimeout);
	    if (res != CE_RESULT_INSUFFICIENT_BUFFER)
		goto end;

	    if (keyNum >= static_cast<unsigned int>(keyRing.size)) {
		wcerr << L"Wrong key number: " << keyNum << L", which is larger than or equal to key ring size: " << keyRing.size << endl;
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	    }
	    
	    // allocate buffer
	    keyRing.keyIDs = new CEKeyID[keyRing.size];
	    
	    // second call to get the list
	    res = CEKey_ListKeys(mHandlePC, kmcPassword, KMCPASSWORD_LEN, &keyRing, mCurrentProcessID, mTimeout);
	    // print result
	    if (res != CE_RESULT_SUCCESS) 
		goto end;

	    // hexify hash
	    CEString hash = hexifyHash(&keyRing.keyIDs[keyNum]);
	    // hexify timestamp
	    CEString sTimestamp = hexify((unsigned char *)&keyRing.keyIDs[keyNum].timestamp, 4);
	    // copy hash and sTimestamp into keyIDFromNum and keyID
	    _snwprintf_s(keyIDFromNum, KM_HASH_LEN*2 + 4*2 + 1, _TRUNCATE, 
		       L"%s%s", CEM_GetString(hash), CEM_GetString(sTimestamp));
	    CEM_FreeString(hash);
	    CEM_FreeString(sTimestamp);
	    keyID = keyIDFromNum;
	}

	if (mAlwaysYes == false) {
	    // are you sure?
	    wcout << L"Are you sure you want to delete the key " << keyID << L" from key ring " << keyRingName << L"?  All data encrypted with this key will be undecryptable.  Enter y/n ?\n"; 
	    if (areYouSure() == true) {
		wcout << L"Delete\n";
	    } else {
		wcout << L"Don't delete\n";
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	    }
	}

	// convert keyID string to CEKeyID type
	CEKeyID ceKeyID;
	for (int i=0; i<KM_HASH_LEN; i++) {
		swscanf_s (keyID + i*2, L"%02x", &ceKeyID.hash[i]);
	}
	// decode timestamp
	// wcout << L"timestamp to be decoded: " << (keyID + KM_HASH_LEN*2) << endl;
	unsigned char buf[4];
	CEString sTimestamp = CEM_AllocateString(keyID + KM_HASH_LEN*2);
	unhexify(sTimestamp, buf, 4);
	ceKeyID.timestamp = (int)*(int*)buf;
	CEM_FreeString(sTimestamp);

	wprintf (L"key hash to delete: ");
	for (int i=0; i<KM_HASH_LEN; i++) {
		wprintf (L"%02x", ceKeyID.hash[i]);
	}
	wcout << L"\n";
	wcout << L"\ntimestamp: " << ceKeyID.timestamp << L"\n";

	res = CEKey_DeleteKey(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, ceKeyID, mCurrentProcessID, mTimeout);

	// print result
	if (res == CE_RESULT_SUCCESS) {
		wcout << L"Succeeded in deleting key ring: " << keyRingName << L"\n";		
	}

end:
	if (sKeyRingName != NULL)
		CEM_FreeString(sKeyRingName);
	if (keyRing.keyIDs != NULL)
	    delete keyRing.keyIDs;

	return res;
}


// ask for password interactively
CEResult_t askPassword(wstring &password)
{
    wchar_t ch;
    const char ENTER = 13;

    while((ch = _getwch()) != ENTER)
    {
        password += ch;
        std::cout << '*';
    }

    wcout << endl;

    return CE_RESULT_SUCCESS;
}

// generate AES 128 bit key based on password
// set IV to 0
CEResult_t setupKey(wchar_t *password)
{

    CEResult_t res = CE_RESULT_SUCCESS;
    HCRYPTHASH hHash = NULL;

    if (password == NULL) {
	wcerr << L"setupKey() requires password" << endl;
	res = CE_RESULT_INVALID_PARAMS;
	goto end;
    }

    // get crypto context
    if (ghCryptProv == NULL) {
	if(CryptAcquireContext(&ghCryptProv, NULL, NULL, PROV_RSA_AES, 0) == FALSE) {
	    if (GetLastError() == NTE_BAD_KEYSET) {
		if (CryptAcquireContext(&ghCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_NEWKEYSET) == FALSE) {
		    wcerr << L"CryptAcquireContext(PROV_RSA_AES, new key set) failed with " << hex << GetLastError() << endl;
		    res = CE_RESULT_GENERAL_FAILED;
		    ghCryptProv = NULL;
		    goto end;
		}
	    } else {
		wcerr << L"CryptAcquireContext(PROV_RSA_AES) failed with " << hex << GetLastError() << endl;
		res = CE_RESULT_GENERAL_FAILED;
		ghCryptProv = NULL;
		goto end;
	    }
	}
    }

    // create hash
    if(CryptCreateHash(ghCryptProv, CALG_SHA, 0, 0, &hHash) == FALSE)
    {
	wcerr << L"CryptCreateHash(SHA256) failed with " << hex << GetLastError() << endl;
	res = CE_RESULT_GENERAL_FAILED;
	goto end;
    }
    // hash password
    if (CryptHashData(hHash, (BYTE*)password, wcslen(password)*sizeof(wchar_t), 0) == FALSE)
    {
	wcerr << L"CryptHashData() failed with " << hex << GetLastError() << endl;
	res = CE_RESULT_GENERAL_FAILED;
	goto end;
    }

    if (CryptDeriveKey(ghCryptProv, CALG_AES_128, hHash, 0, &ghKey) == FALSE)
    {
	wcerr << L"CryptDeriveKey() failed with " << hex << GetLastError() << endl;
	res = CE_RESULT_GENERAL_FAILED;
	ghKey = NULL;
	goto end;
    }

    BYTE IV[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if (CryptSetKeyParam(ghKey, KP_IV, IV, 0) == FALSE) {
	wcerr << L"CryptSetKeyParam() failed with " << hex << GetLastError() << endl;
	res = CE_RESULT_GENERAL_FAILED;
	ghKey = NULL;
	goto end;
    }
    
 end:
    if(hHash != NULL) {
	CryptDestroyHash(hHash);
	hHash = NULL;
    }
    return res;
}

// export key ring 
// format:
//   Key Ring Name: 34 byte
//   Number of keys in key ring: 4 byte
//   Key 0: 76 byte
//   Key 1:
//   ...
CEResult_t exportKeyRing(wchar_t *keyRingName, wchar_t *filename, wchar_t *password)
{
	CEResult_t res = CE_RESULT_SUCCESS;
	CEKeyRing keyRing;
	CEString sKeyRingName = CEM_AllocateString(keyRingName);
	CEKey *keys = NULL;
	fstream *tmpFileStream = NULL; // read / write
	ofstream *outFileStream = NULL; // write only
	BYTE *buf = NULL;

	keyRing.keyIDs = NULL;
	res = setupKey(password);
	if (res != CE_RESULT_SUCCESS)
	    goto end;

	if (mAlwaysYes == false) {
	    // are you sure?
	    wcout << L"Are you sure you want to export key ring " << keyRingName << L" and write into file " << filename << L"?  If the file exists, it will be overwritten.\nEnter y/n ?\n"; 
	    if (areYouSure() == false) {
		wcout << L"Don't export\n";
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	    }
	}
	char tmpdir[MAX_PATH+1];
	if (GetTempPathA(MAX_PATH, tmpdir) <= 0) {
	    wcerr << L"Cannot find temp folder" << endl;
	    perror("Error Code");
	    res = CE_RESULT_GENERAL_FAILED;
	    goto end;
	}

	char tmppath[MAX_PATH + 16];
	_snprintf_s (tmppath, MAX_PATH + 16, _TRUNCATE, "%s/tmpFile", tmpdir);
	
	tmpFileStream = new fstream(tmppath, ios::in | ios::out | ios::binary | ios::trunc);
	outFileStream = new (nothrow) ofstream(filename, ios::out | ios::binary | ios::trunc);	
	if (tmpFileStream == NULL || tmpFileStream->good() == false ||
	    outFileStream == NULL || outFileStream->good() == false) {
		wcerr << L"Error opening file: " << filename << endl;
		perror("Error Code");
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	}

	// write key ring name
	wchar_t keyRingNameToWrite[KM_MAX_KEYSTORE_NAME + 1];
	memset (keyRingNameToWrite, NULL, sizeof(wchar_t) * (KM_MAX_KEYSTORE_NAME + 1));
	wcsncpy_s (keyRingNameToWrite, KM_MAX_KEYSTORE_NAME + 1, keyRingName, _TRUNCATE);
	tmpFileStream->write((const char *)keyRingNameToWrite, (KM_MAX_KEYSTORE_NAME + 1) * sizeof(wchar_t));

	// get list of keys
	// first call to ListKeys.  In keyRing, list is empty and size is 0.  This is to get the size of the list.
	keyRing.size = 0;
	keyRing.keyIDs = NULL;
	keyRing.name = sKeyRingName;
	res = CEKey_ListKeys(mHandlePC, kmcPassword, KMCPASSWORD_LEN, &keyRing, mCurrentProcessID, mTimeout);
	if (res != CE_RESULT_INSUFFICIENT_BUFFER)
		goto end;

	// allocate buffer
	keyRing.keyIDs = new CEKeyID[keyRing.size];

	// second call to get the list
	res = CEKey_ListKeys(mHandlePC, kmcPassword, KMCPASSWORD_LEN, &keyRing, mCurrentProcessID, mTimeout);

	// print result
	if (res != CE_RESULT_SUCCESS) 
		goto end;

	wcout << L"Succeeded in obtaining key list from key ring: " << keyRingName << L"\n";
	keys = new CEKey[keyRing.size];
	for (int i=0; i<keyRing.size; i++) {
		// hexify hash
        CEString hash = hexifyHash(&keyRing.keyIDs[i]);
		// hexify timestamp
		CEString sTimestamp = hexify((unsigned char *)&keyRing.keyIDs[i].timestamp, 4);
        wcout << L"Getting Key " << i << L" ID: " << CEM_GetString(hash) << CEM_GetString(sTimestamp) << endl;
        CEM_FreeString(hash);
		CEM_FreeString(sTimestamp);

		// get key
		res = CEKey_GetKey(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, keyRing.keyIDs[i], &keys[i], mCurrentProcessID, mTimeout);
		if (res != CE_RESULT_SUCCESS) 
			goto end;
	}

	// write key ring size
	tmpFileStream->write((const char *)&keyRing.size, sizeof(keyRing.size));
	// write keys
	for (int i=0; i<keyRing.size; i++) {
		tmpFileStream->write((const char *)&keys[i], sizeof(keys[i]));
	}

	// finished writing to temp file
	tmpFileStream->seekg(0, ios::end);
	DWORD size = tmpFileStream->tellg();
	tmpFileStream->seekg(0);

	buf = new BYTE[size + 16];
	tmpFileStream->read((char*)buf, size);

	// encrypt data
	if (CryptEncrypt(ghKey, NULL, TRUE, 0, buf, &size, size + 16) == FALSE) {
	    wcerr << L"CryptEncrypt failed with error code: " << hex << GetLastError() << endl;
	    res = CE_RESULT_GENERAL_FAILED;
	    goto end;
	}

	outFileStream->write((char*)buf, size);

	tmpFileStream->close();
	delete tmpFileStream;
	tmpFileStream = NULL;
	(void)remove(tmppath);
	
	wprintf (L"Succeeded exporting key ring\n");

end:
	// clean up
	delete[] keyRing.keyIDs;
	delete[] keys;
	if (tmpFileStream != NULL) {
		tmpFileStream->close();
		delete tmpFileStream;
	}
	if (outFileStream != NULL) {
		outFileStream->close();
		delete outFileStream;
	}
	if (sKeyRingName != NULL)
		CEM_FreeString(sKeyRingName);
	if (buf != NULL) 
	    delete[] buf;

	return res;
}

// import key ring 
CEResult_t importKeyRing(wchar_t *keyRingName, wchar_t *filename, wchar_t *password)
{
	CEResult_t res = CE_RESULT_SUCCESS;
	CEString sKeyRingName = NULL;
	CEKey *keys = NULL;
	ifstream *inFileStream = NULL;
	BYTE *buf = NULL;

	if (keyRingName != NULL)
	    sKeyRingName = CEM_AllocateString(keyRingName);

	res = setupKey(password);
	if (res != CE_RESULT_SUCCESS)
	    goto end;

	if (mAlwaysYes == false) {
	    // are you sure?
	    if (keyRingName != NULL) 
		wcout << L"Are you sure you want to import key ring " << keyRingName << L" from file " << filename << L"?\nEnter y/n ?\n";
	    else
		wcout << L"Are you sure you want to import key ring from file " << filename << L"?\nEnter y/n ?\n";
	    if (areYouSure() == true) {
		wcout << L"Import\n";
	    } else {
		wcout << L"Don't import\n";
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	    }
	}

	inFileStream = new ifstream(filename, ios::in | ios::binary);
	if (inFileStream == NULL || inFileStream->good() == false) {
		wcerr << L"Error opening file: " << filename << endl;
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	}

	// read file
	inFileStream->seekg(0, ios::end);
	DWORD size = inFileStream->tellg();
	inFileStream->seekg(0);
	buf = new (nothrow) BYTE[size];
	if (buf == NULL)
	{
		res = CE_RESULT_GENERAL_FAILED;
		goto end;
	}
	inFileStream->read((char*)buf, size);
	
	// decrypt data
	if (CryptDecrypt(ghKey, NULL, TRUE, 0, buf, &size) == FALSE) {
	    DWORD errorCode = GetLastError();
	    if (errorCode == NTE_BAD_DATA) { 
		wcerr << L"Cannot decrypt key ring.  Invalid password or corrupted file." << endl;
	    } else {
		wcerr << L"Cannot decrypt key ring.  Error code: " << hex << errorCode << endl;
	    }
	    res = CE_RESULT_GENERAL_FAILED;
	    goto end;
	}
	wcout << L"decrypted size: " << size << endl;

	// close input file
	inFileStream->close();
	delete inFileStream;
	inFileStream = NULL;
	
	// read key ring name
	wchar_t keyRingNameToRead[KM_MAX_KEYSTORE_NAME + 1];
	DWORD index = 0;
	memcpy((char*)keyRingNameToRead, buf, sizeof(wchar_t) * (KM_MAX_KEYSTORE_NAME + 1));
	index += sizeof(wchar_t) * (KM_MAX_KEYSTORE_NAME + 1);
	wcout << L"Reading key ring name in file: " << keyRingNameToRead << endl;

	// if the key ring name wasn't specified in input argument, use the name in the file
	if (keyRingName == NULL)
	    sKeyRingName = CEM_AllocateString(keyRingNameToRead);
	
	// create key ring
	wcout << L"Create key ring: " << CEM_GetString(sKeyRingName) << endl;
	res = CEKey_CreateKeyRing(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, mCurrentProcessID, mTimeout);

	// print result
	if (res == CE_RESULT_SUCCESS) {
	    wcout << L"Succeeded creating key ring: " << CEM_GetString(sKeyRingName) << endl;
	} else {	
		goto end;
	}

	// read number of keys
	CEint32 ringsize;
	memcpy((char*)&ringsize, buf + index, sizeof(ringsize));
	index += sizeof(ringsize);
	wcout << L"Reading key ring size: " << ringsize << endl;

	// read keys
	keys = new CEKey[ringsize];
	for (int i=0; i<ringsize; i++) {
		wprintf (L"Reading key[%d]\n", i);
		//inFileStream->read((char*)(&(keys[i])), sizeof(keys[i]));
		memcpy ((char*)(&(keys[i])), buf + index, sizeof(keys[i]));
		index += sizeof(keys[i]);
	}

	// set keys
	for (int i=0; i<ringsize; i++) {
		wprintf (L"Inserting key[%d] into key store\n", i);

		res = CEKey_SetKey(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, keys[i], mCurrentProcessID, mTimeout);
		if (res != CE_RESULT_SUCCESS) 
			goto end;
	}

	wprintf (L"Succeeded importing key ring\n");

end:
	if (keys != NULL)
		delete[] keys;
	if (inFileStream != NULL) {
		inFileStream->close();
		delete inFileStream;
	}
	if (sKeyRingName != NULL)
		CEM_FreeString(sKeyRingName);
	if (buf != NULL)
	    delete[] buf;

	return res;
}

CEResult_t getLatestKey(wchar_t *keyRingName)
{
	CEString sKeyRingName = NULL, hash = NULL, sTimestamp = NULL, keyhex = NULL;
	sKeyRingName = CEM_AllocateString(keyRingName);
    CEKeyID keyid;
    memset(&keyid, '\0', sizeof(CEKeyID));
    CEKey key;

    CEResult_t res = CEKey_GetKey(mHandlePC, kmcPassword, KMCPASSWORD_LEN, sKeyRingName, keyid, &key, mCurrentProcessID, mTimeout);

    // print result
    if (res == CE_RESULT_SUCCESS) {
        wcout << L"Succeeded in getting latest key from : " << keyRingName << endl;
    } else {	
        goto end;
    }

    wcout << L"version: " << key.struct_version << std::endl;
    hash = hexifyHash(&key.id);
    sTimestamp = hexify((unsigned char *)&key.id.timestamp, 4);
    wcout << L"id:      " << CEM_GetString(hash) << CEM_GetString(sTimestamp) << std::endl;
    keyhex = hexify((unsigned char *)&key.key, sizeof(key.key));
    wcout << L"key:     " << CEM_GetString(keyhex) << std::endl;
    wcout << L"keylen:  " << key.keylen << std::endl;
	
 end:

	
    // clean up
	if (hash != NULL)
	    CEM_FreeString(hash);
    if (keyhex != NULL)
		CEM_FreeString(keyhex);
    if (sTimestamp != NULL)
		CEM_FreeString(sTimestamp);
	if (sKeyRingName != NULL)
		CEM_FreeString(sKeyRingName);
	
    
    return res;
}

// display usage information and quit
void printUsage()
{
	wcout << L"Usage: KeyUtil.exe command args\n";
	wcout << L"Commands:\n";
	wcout << L"    create_key_ring(ckr) {key_ring_name}                  # Create key ring\n";
	wcout << L"    generate_key(gk) {key_ring_name} [keylen]                  # Generate key in specified key ring.  keylen is in bytes.  default is 16.\n";
	wcout << L"    list_key_ring(lkr)                                    # List key rings\n";
	wcout << L"    list_key(lk) {key_ring_name}                          # List keys in specified key ring\n";
	wcout << L"    import_key_ring(ikr) [key_ring_name] {ring_file_name} # import key ring from file to specified key ring\n";
	wcout << L"    export_key_ring(ekr) {key_ring_name} {ring_file_name} # export key ring from specified key ring to file\n";
	wcout << L"    delete_key(dk) {key_ring_name} {key_ID}               # delete specified key from specified key ring\n";
	wcout << L"    delete_key_ring(dkr) {key_ring_name}                  # delete specified key ring\n";
	wcout << endl;
	wcout << L"Common options:\n";
	wcout << L"    -p profile_password                                   # Profile Password to authenticate to Policy Controller\n";
	wcout << L"    -k key_ring_password                                  # Password used to encrypt or decrypt key ring file\n";
	wcout << L"    -t timeout                                            # Specify a timeout in ms (default is 30000ms)\n";
	wcout << L"    -y                                                    # Answer \"y\" to all yes/no questions\n";
    exit(-1);
}

// open connection to Policy Controller
CEResult_t connectPolicyController()
{
	CEApplication app;
    CEString appName = CEM_AllocateString(L"KeyUtil.exe");
    CEString appPath = CEM_AllocateString(L"KeyUtil.exe");
    app.appPath = appPath;
    app.appName = appName;

    CEUser user;
    user.userName = NULL;
    user.userID = NULL;

    CEResult_t res = CECONN_Initialize(app, user, NULL, &mHandlePC, 10000);

	CEM_FreeString(app.appPath);
    CEM_FreeString(app.appName);

	return res;
}

// close connection to Policy Controller
void disconnectPolicyController()
{
    CECONN_Close(mHandlePC, 10000);

    return;

}

int _tmain(int argc, _TCHAR* argv[])
{
    wchar_t *profilePassword = NULL, *keyRingPassword = NULL;
    wstring sProfilePassword, sKeyRingPassword;
    
    if (argc < 2)
	printUsage();

    // get process ID of myself
    mCurrentProcessID = GetCurrentProcessId();
    if (mCurrentProcessID == 0) {
        wcerr << L"GetCurrentProcessId() failed.  Use process ID of 0.\n";
    }

    CEResult_t res = connectPolicyController();
    if (res != CE_RESULT_SUCCESS) {
        wcerr << L"Connecting to Policy Controller failed with error code: " << res << L"\n";
        return 1;
    }
    wcout << L"Connected to Policy Controller\n";
    
    int remainingArgs = argc-1;

    // parse options, first
    DWORD index = 1;
    while (remainingArgs > 0) {
        if (wcscmp(argv[index], L"-k") == 0) {
            if (remainingArgs < 2) {
                printUsage();
            }

            keyRingPassword = argv[index+1];
            index += 2;
            remainingArgs -= 2;
            continue;
        } else if (wcscmp(argv[index], L"-p") == 0) {
            if (remainingArgs < 2) {
                printUsage();
            }
            profilePassword = argv[index+1];
            index += 2;
            remainingArgs -= 2;
            continue;
        } else if (wcscmp(argv[index], L"-t") == 0) {
            if (remainingArgs < 2) {
                printUsage();
            }
            mTimeout = _wtoi(argv[index+1]);
            index += 2;
            remainingArgs -= 2;
            continue;
        } else if (wcscmp(argv[index], L"-y") == 0) {
	    if (remainingArgs < 1) {
                printUsage();
            }
            mAlwaysYes = true;
            index++;
            remainingArgs--;
            continue;
        } else {
            break;
        }
    }

    if (remainingArgs < 1) {
        printUsage();
    }

    // make this process trusted
    if (profilePassword == NULL) {
        wcout << L"Policy Controller Administrator password not provided.  Please type it in: ";
        askPassword(sProfilePassword);
        profilePassword = (wchar_t*)sProfilePassword.c_str();
        // TODO: zeroize sProfilePassword
    }
    CEString cesProfilePassword = CEM_AllocateString(profilePassword);
    res = CESEC_MakeProcessTrusted(mHandlePC, cesProfilePassword);
    // TODO: zeroize password
    CEM_FreeString(cesProfilePassword);
    if (res != CE_RESULT_SUCCESS) {
        wcerr << L"Profile password verification failed: " << res << L"\n";
        return 1;
    }
	
    // switch on command
    if (wcscmp(argv[index], L"create_key_ring") == 0 || wcscmp(argv[index], L"ckr") == 0) {
        // crete key ring
        if (remainingArgs < 2)
            printUsage();
        res = createKeyRing(argv[index+1]);
    } else if (wcscmp(argv[index], L"generate_key") == 0 || wcscmp(argv[index], L"gk") == 0) {
        // generate key in key ring
        if (remainingArgs < 2) 
            printUsage();

        // if key len is specified, take it
        int keylen = 16; // default 16 bytes, or 128 bit
        if (remainingArgs >= 3) {
            keylen = _wtoi(argv[index+2]);
        }

        res = generateKey(argv[index+1], keylen);
    } else if (wcscmp(argv[index], L"list_key_ring") == 0 || wcscmp(argv[index], L"lkr") == 0) {
        // list key ring
        // no arguments
        res = listKeyRing();
    } else if (wcscmp(argv[index], L"list_key") == 0 || wcscmp(argv[index], L"lk") == 0) {
        // list key
        if (remainingArgs < 2) 
            printUsage();
        res = listKey(argv[index+1]);
    } else if (wcscmp(argv[index], L"delete_key_ring") == 0 || wcscmp(argv[index], L"dkr") == 0) {
        // delete key ring
        if (remainingArgs < 2) 
            printUsage();
        res = deleteKeyRing(argv[index+1]);
    } else if (wcscmp(argv[index], L"delete_key") == 0 || wcscmp(argv[index], L"dk") == 0) {
        // delete key
        if (remainingArgs < 3) 
            printUsage();
        res = deleteKey(argv[index+1], argv[index+2]);
    } else if (wcscmp(argv[index], L"export_key_ring") == 0 || wcscmp(argv[index], L"ekr") == 0) {
        // export key ring
        if (remainingArgs < 3) 
            printUsage();
        // if I don't have key ring password, ask
        if (keyRingPassword == NULL) {
	    wstring sKeyRingPasswordConfirm;
            wcout << "Key ring password not provided.  Please type it in: ";
            askPassword(sKeyRingPassword);
	    wcout << "Please confirm by typing it again: ";
	    askPassword(sKeyRingPasswordConfirm);
	    // check
	    if (sKeyRingPassword != sKeyRingPasswordConfirm) {
		wcerr << "Passwords did not match.";
		return 1;
	    }
            keyRingPassword = (wchar_t*)sKeyRingPassword.c_str();
        }
        res = exportKeyRing(argv[index+1], argv[index+2], keyRingPassword);	
    } else if (wcscmp(argv[index], L"import_key_ring") == 0 || wcscmp(argv[index], L"ikr") == 0) {
        // import key ring
        if (remainingArgs < 2) 
            printUsage();
        if (keyRingPassword == NULL) {
            wcout << L"Key ring password not provided.  Please type it in: ";
            askPassword(sKeyRingPassword);
            keyRingPassword = (wchar_t*)sKeyRingPassword.c_str();
        }
        if (remainingArgs == 2) {
            res = importKeyRing(NULL, argv[index+1], keyRingPassword);
        } else {
            res = importKeyRing(argv[index+1], argv[index+2], keyRingPassword);
        }
    } else if (wcscmp(argv[index], L"get_latest_key") == 0 || wcscmp(argv[index], L"glk") == 0) {
        if (remainingArgs < 1)
            printUsage();
        res = getLatestKey(argv[index+1]);
    } else {
        wcerr << L"Unknown command: " << argv[index] << L"\n\n";
        printUsage();
    }

    disconnectPolicyController();

    if (res != CE_RESULT_SUCCESS) {
        wcerr << L"Command failed with error code: " << res << L"\n";

        CEString lastException = CEKey_GetLastException();
        if (lastException != NULL) 
            wcerr << L"Last exception was: " << lastException->buf << endl;
    }

    return res;
}

