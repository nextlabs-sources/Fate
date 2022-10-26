#ifndef GPOTRANSFORMGENERATOR_H
#define GPOTRANSFORMGENERATOR_H

#include "CustomizationParameters.h"

class GPOTransformGenerator {
	wchar_t* pathToMsi;
	CustomizationParameters* customizationParameters;
	wchar_t* pathToMsiCopy;

	void copyOriginalMsi();
	void modifyCopiedMsi();
	void createTransorm();
	void deleteMsiCopy();
	wchar_t* getPathToMsiCopy();
public:
	GPOTransformGenerator(wchar_t*, CustomizationParameters*);
	~GPOTransformGenerator();
	void generateTransform();
};

#endif