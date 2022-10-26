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

	//bMarkBeforeCreate�����false������ֱ�ӵļ���
	//�����true������ֱ�ӵļ��ܣ����Ǳ��Ϊ���ܣ��������ļ�������֮ǰ��ǡ����ļ������ɺ��ļ����Զ������ܡ�
	//���bQueryOnly��true�������κμ��ܣ�ֻ�ǲ�ѯstrPath�ǲ����Ѿ���������
	static EncryptRetValueType Encrypt(const wstring& strPath, bool bMarkBeforeCreate, bool bQueryOnly = false);

};
