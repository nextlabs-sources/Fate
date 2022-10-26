#define _CRT_RAND_S
#include <winsock2.h>
#include <windows.h>
#include <wincon.h>
#include <stdlib.h>
#include <stdio.h>
#include <Iphlpapi.h>
#include <wincrypt.h>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <vector>

#include "quenchhash.h"

static char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::string hexify(unsigned char *buf, int len)
{
    std::ostringstream sb;
    for (int i = 0; i < len; i++)
    {
        sb << hexmap[buf[i] >> 4];
        sb << hexmap[buf[i] % 16];
    }

    return sb.str();
}

static std::string hashdata(const char *data, DWORD dataLen)
{
    //--------------------------------------------------------------------
    //  Declare variables.
    
    HCRYPTPROV hCryptProv;
    HCRYPTHASH hHash;
    
    //--------------------------------------------------------------------
    // Get a handle to a cryptography provider context.
    if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) 
    {
        return "";
    }
    //--------------------------------------------------------------------
    // Acquire a hash object handle.
    
    if(!CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash))
    {
        return "";
    }
    
    //--------------------------------------------------------------------
    //  This code assumes that the handle of a cryptographic context 
    //  has been acquired and that a hash object has been created 
    //  and its handle (hHash) is available.
    
    if(!CryptHashData(hHash, (const BYTE *)data, dataLen, 0))
    {
        return "";
    }

    DWORD dwHashLen = 0;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))
    {
        return "";
    }

    BYTE *hashedData = (BYTE *)malloc(dwHashLen);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hashedData, &dwHashLen, 0))
    {
        free(hashedData);
        return "";
    }
    
    std::string result = hexify(hashedData, dwHashLen);

    //--------------------------------------------------------------------
    // After processing, hCryptProv and hHash must be released.
    
    if(hHash)
    { 
        CryptDestroyHash(hHash);
    }

    if(hCryptProv) 
    {
        CryptReleaseContext(hCryptProv,0);
    }

    free(hashedData);
    return result;
}



// Fetches the MAC address and prints it
static std::string GetMACAddress(void)
{
    DWORD dwBufLen = 0;

    DWORD ret = GetAdaptersAddresses(AF_UNSPEC,
                                     0,
                                     NULL,
                                     NULL,
                                     &dwBufLen);

    if (ret != ERROR_BUFFER_OVERFLOW)
    {
        return "";
    }

    IP_ADAPTER_ADDRESSES *adapterInfo = (IP_ADAPTER_ADDRESSES *)malloc(dwBufLen);
    IP_ADAPTER_ADDRESSES *pAdapter = adapterInfo;

    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapterInfo, &dwBufLen) != ERROR_SUCCESS) {
        free (adapterInfo);
        return "";
    }

    std::list<std::string> MAClist;

    while (pAdapter != NULL) {
        std::string pa = hexify(pAdapter->PhysicalAddress, pAdapter->PhysicalAddressLength);
        MAClist.push_back(pa);
        pAdapter = pAdapter->Next;
    }

    // Sort and then convert to a single string
    MAClist.sort();
    
    std::ostringstream sb;
    sb << ":";

    for (std::list<std::string>::const_iterator i = MAClist.begin(); i != MAClist.end(); ++i)
    {
        sb << *i << ":";
    }

    free (adapterInfo);
    return sb.str();
}

// Misdirection function.  Exists just to ensure that the various bogus strings appear in the exe as a distraction
static std::string generateSecret(void)
{
    std::ostringstream secret;
    secret << QUENCH_NON_SECRET_1 << QUENCH_SHARED_SECRET;
    secret << QUENCH_NON_SECRET_3 << QUENCH_NON_SECRET_2;
    secret << QUENCH_NON_SECRET_3 << QUENCH_SHARED_SECRET;
    secret << QUENCH_NON_SECRET_2 << QUENCH_NON_SECRET_1;
    return secret.str();
}

int ce_meta_value = 1000;

template <typename X, typename Y> static std::string constructData(X randNumber, Y timestamp, const std::string &secret)
{
    std::ostringstream data;
    data << GetMACAddress() << secret << randNumber << timestamp;

    std::ostringstream result;
    result << randNumber << "," << timestamp << "," << hashdata(data.str().c_str(), (DWORD)strlen(data.str().c_str()));

    // Bogus call.  This is just to ensure that the various bytes of the non-secrets appear in the final code
    if (ce_meta_value < 100)  {
        return generateSecret();
    }

    return result.str();
}

std::string generateData(const std::string &sharedSecret)
{
    // The data is as follows:
    // random number, timestamp, hash(mac address + secret + random number + timestamp)
    unsigned int randNumber;
    rand_s(&randNumber);

    return constructData(randNumber, time(NULL), sharedSecret);
}

DWORD validateData (const std::string &inputData, time_t currentTime, const std::string &secret)
{
    const char delimiter=',';
    // Parse the input data
    std::string::size_type lastPos = 0;
    std::string::size_type pos = inputData.find_first_of(delimiter, lastPos);

    std::vector<std::string> tokens;
    while(std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(inputData.substr(lastPos, pos - lastPos));
        lastPos = inputData.find_first_not_of(delimiter, pos);
        pos = inputData.find_first_of(delimiter, lastPos);
    }

    if (tokens.size() != 3)
    {
        return ERROR_INVALID_DATA;
    }

    // Make sure that the time isn't too old
    __int64 encodedTime = _atoi64(tokens[1].c_str());

#define TIME_WINDOW (5 * 60)
    if (currentTime < encodedTime ||
        currentTime - encodedTime > TIME_WINDOW)
    {
        return ERROR_BAD_ARGUMENTS;
    }

    std::string expectedData = constructData(tokens[0], tokens[1], secret);

    if (expectedData != inputData)
    {
        return ERROR_WRONG_PASSWORD;
    }

    return ERROR_SUCCESS;
}

