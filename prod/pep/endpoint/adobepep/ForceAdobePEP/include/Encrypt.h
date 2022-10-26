#pragma once


#include <string>
using namespace std;

enum EncryptRetValueType
{
	emEncryptSuccess	=0,
	emIsEncrytFile		=1,
	emNotEncrytFile		=2,
	emEncryptError		=3
};

class CEncrypt
{
public:
	CEncrypt(void);
	virtual ~CEncrypt(void);

	//bMarkBeforeCreate如果是false，则是直接的加密
	//如果是true，则不是直接的加密，而是标记为加密，往往在文件被生成之前标记。当文件被生成后，文件会自动被加密。
	//如果bQueryOnly是true，不做任何加密，只是查询strPath是不是已经被加密了
	static EncryptRetValueType Encrypt(const wstring& strPath, bool bMarkBeforeCreate, bool bQueryOnly = false);

};
