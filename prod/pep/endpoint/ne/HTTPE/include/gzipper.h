#pragma once

#include <string>
using namespace std;

class CGZipper
{
public:
	static CGZipper& Instance();

	/*
	return value:
	true, success;
	false, fatal error;
	*/
	BOOL UNZipData(const string& input, string& output);

private:
	CGZipper(void);
	CGZipper(const CGZipper&);
	void operator = (const CGZipper&);
	~CGZipper(void);
};