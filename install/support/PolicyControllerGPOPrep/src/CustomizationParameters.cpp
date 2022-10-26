
#include "stdafx.h"
#include "CustomizationParameters.h"

void CustomizationParameters::setIcenetHostname(wchar_t* hostname) {
	this->icenetHostname = new wchar_t[wcslen(hostname) + 1];
	wcscpy_s(this->icenetHostname, wcslen(hostname) + 1, hostname);
}

wchar_t* CustomizationParameters::getIcenetHostname() {
	return this->icenetHostname;
}

void CustomizationParameters::setIcenetPort(unsigned int port) {
	this->icenetPort = port;
}

unsigned int CustomizationParameters::getIcenetPort() {
	return this->icenetPort;
}