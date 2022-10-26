#ifndef CUSTOMIZATIONPARAMETERS_H
#define CUSTOMIZATIONPARAMETERS_H

class CustomizationParameters {
	wchar_t* icenetHostname;
	unsigned int icenetPort;
public:
	void setIcenetHostname(wchar_t*);
	wchar_t* getIcenetHostname();
	void setIcenetPort(unsigned int);
	unsigned int getIcenetPort();
};

#endif