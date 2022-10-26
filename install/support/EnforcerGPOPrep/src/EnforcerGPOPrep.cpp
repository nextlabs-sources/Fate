// EnforcerGPOPrep.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GPOTransformGenerator.h"
#include "XGetopt.h"
#include <iostream>

#define USAGE "Usage: EnforcerGPOPrep.exe -i <Path To MSI file>"

using namespace std;

class command_option_exception : public exception {
	TCHAR option;
protected:
	command_option_exception(TCHAR option)
	{
		this->option = option;
	}
public:
	TCHAR getOptionCharacter()
	{
		return this->option;
	}
};

class missing_option : public command_option_exception {
public:
	missing_option(TCHAR option) : command_option_exception(option) { }
};

class unrecognized_option : public command_option_exception {
public:
	unrecognized_option(TCHAR option) : command_option_exception(option) { }
};

class invalid_option_argument : public command_option_exception {
	wchar_t* reason;
public:
	invalid_option_argument(TCHAR option, wchar_t* reason) : command_option_exception(option) 
	{
		this->reason = new wchar_t[wcslen(reason) + 1];
		wcsncpy_s(this->reason, wcslen(reason) + 1, reason, _TRUNCATE);
	}

	wchar_t* getReason() 
	{
		return this->reason;
	}
};

class EnforcerGPOPrep 
{
	wchar_t* pathToMsi;
	CustomizationParameters* customizationParameters;

public:
	void parseCommandLineArgs(int, _TCHAR*[]);
	void execute();

	EnforcerGPOPrep();
	~EnforcerGPOPrep();
};

EnforcerGPOPrep::EnforcerGPOPrep() 
{
	this->pathToMsi = NULL;
	this->customizationParameters = NULL;
}

void EnforcerGPOPrep::parseCommandLineArgs(int argc, _TCHAR* argv[])
{
	int c;
	int argIndex;
	TCHAR specifiedParameters[4];

	this->customizationParameters = new CustomizationParameters();
	argIndex = 0;
    wcout << "Parsing arguments" <<"\n";
	while ((c = getopt(argc, argv, _T("i:s:p:"))) != EOF)
	{
		specifiedParameters[argIndex] = static_cast<TCHAR>(c);
		argIndex++;
		switch (c)
		{
			case _T('i'):
				this->pathToMsi = new wchar_t[wcslen(optarg) + 1];
				wcsncpy_s(this->pathToMsi, wcslen(optarg) + 1, optarg, _TRUNCATE);
				break;
				
			
			default:
				throw unrecognized_option(*(argv[optind-1]+1));
				break;
		}
	}

	specifiedParameters[argIndex] = '\0';
	
	if (_tcschr(specifiedParameters, 'i') == NULL) {
			throw missing_option('i');
	}
	wcout << "Parsing arguments complete" <<"\n";
}

void EnforcerGPOPrep::execute() 
{	
	GPOTransformGenerator* transformGenerator = new GPOTransformGenerator(this->pathToMsi, this->customizationParameters);

	transformGenerator->generateTransform();

	delete transformGenerator;
}

EnforcerGPOPrep::~EnforcerGPOPrep() 
{
	delete pathToMsi;
	delete customizationParameters;
}

int _tmain(int argc, _TCHAR* argv[])
{
	EnforcerGPOPrep *app = new EnforcerGPOPrep();

	try 
	{
		app->parseCommandLineArgs(argc, argv);
		app->execute();
	} 
	catch (missing_option exception) 
	{
		wcout << "\nThe option, " << exception.getOptionCharacter() << ", must be specified";
		wcout << "\n\n" << USAGE << "\n\n";
	}
	catch (unrecognized_option exception) 
	{
		wcout << "\nThe option, " << exception.getOptionCharacter() << ", is either not a valid option or is missing an argument.";
		wcout << "\n\n" << USAGE << "\n\n";
	}
	catch (invalid_option_argument exception) 
	{
		wcout << "\nA value specified for the option, " << exception.getOptionCharacter() << ", is invalid.  " << exception.getReason();
		wcout << "\n\n" << USAGE << "\n\n";
	}
	catch (exception exception)
	{
		wcout << "\nAn unexpected error occured.  " << exception.what(); 
	}
	delete app;
}

