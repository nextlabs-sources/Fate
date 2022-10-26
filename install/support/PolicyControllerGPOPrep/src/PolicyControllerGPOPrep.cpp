// PolicyControllerGPOPrep.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GPOTransformGenerator.h"
#include "XGetopt.h"
#include <iostream>

#define USAGE "Usage: PolicyControllerGPOPrep.exe -s <IcenetServerHost> -p <IcenetServerPort> [-i <Path To Policy Controller MSI>]"

#define EDLP_POLICY_CONTROLLER_FILE_NAME L"EDLP-PolicyController-setup.msi"
#define CE_POLICY_CONTROLLER_FILE_NAME L"CE-PolicyController-setup.msi"

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
		wcscpy_s(this->reason, wcslen(reason) + 1, reason);
	}

	wchar_t* getReason() 
	{
		return this->reason;
	}
};

class PolicyControllerGPOPrep 
{
	wchar_t* pathToMsi;
	CustomizationParameters* customizationParameters;

public:
	void parseCommandLineArgs(int, _TCHAR*[]);
	void execute();

	PolicyControllerGPOPrep();
	~PolicyControllerGPOPrep();
};

PolicyControllerGPOPrep::PolicyControllerGPOPrep() 
{
	this->pathToMsi = NULL;
	this->customizationParameters = NULL;
}

void PolicyControllerGPOPrep::parseCommandLineArgs(int argc, _TCHAR* argv[])
{
	int c;
	int portArg;
	int argIndex;
	TCHAR specifiedParameters[4];

	this->customizationParameters = new CustomizationParameters();
	argIndex = 0;

	while ((c = getopt(argc, argv, _T("i:s:p:"))) != EOF)
	{
		specifiedParameters[argIndex] = static_cast<TCHAR>(c);
		argIndex++;
		switch (c)
		{
			case _T('i'):
				this->pathToMsi = new wchar_t[wcslen(optarg) + 1];
				wcscpy_s(this->pathToMsi, wcslen(optarg) + 1, optarg);
				break;
				
			case _T('s'):
				this->customizationParameters->setIcenetHostname(optarg);
				break;

			case _T('p'):
				portArg = _wtoi(optarg);
				if (portArg <= 0) 
				{
					throw invalid_option_argument('p', L"Port specified is not a number.");
				}
				this->customizationParameters->setIcenetPort(portArg);
				break;

			default:
				throw unrecognized_option(*(argv[optind-1]+1));
				break;
		}
	}

	specifiedParameters[argIndex] = '\0';
	if (_tcschr(specifiedParameters, 'p') == NULL) 
	{
		throw missing_option('p');
	}

	if (_tcschr(specifiedParameters, 's') == NULL) {
		throw missing_option('s');
	}

	if (_tcschr(specifiedParameters, 'i') == NULL) {
	    FILE* fp = NULL;
		if (_wfopen_s(&fp, EDLP_POLICY_CONTROLLER_FILE_NAME, L"r") == 0)
		{
			this->pathToMsi = new wchar_t[wcslen(EDLP_POLICY_CONTROLLER_FILE_NAME)];
			wcscpy_s(this->pathToMsi, wcslen(EDLP_POLICY_CONTROLLER_FILE_NAME), EDLP_POLICY_CONTROLLER_FILE_NAME);
		} 
		else if (_wfopen_s(&fp, CE_POLICY_CONTROLLER_FILE_NAME, L"r") == 0)
		{
			this->pathToMsi = new wchar_t[wcslen(CE_POLICY_CONTROLLER_FILE_NAME)];
			wcscpy_s(this->pathToMsi, wcslen(CE_POLICY_CONTROLLER_FILE_NAME), CE_POLICY_CONTROLLER_FILE_NAME);
		} 
		else 
		{
			throw missing_option('i');
		}
	}
}

void PolicyControllerGPOPrep::execute() 
{	
	GPOTransformGenerator* transformGenerator = new GPOTransformGenerator(this->pathToMsi, this->customizationParameters);

	transformGenerator->generateTransform();

	delete transformGenerator;
}

PolicyControllerGPOPrep::~PolicyControllerGPOPrep() 
{
	delete pathToMsi;
	delete customizationParameters;
}

int _tmain(int argc, _TCHAR* argv[])
{
	PolicyControllerGPOPrep *app = new PolicyControllerGPOPrep();

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

