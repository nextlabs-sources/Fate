
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <msi.h>
#include <msiquery.h>
#include "GPOTransformGenerator.h"

using namespace std;

GPOTransformGenerator::GPOTransformGenerator(wchar_t* pathToMsi, CustomizationParameters* customizationParameters) {
	if (pathToMsi == NULL) {
		throw invalid_argument("pathToMsi must not be NULL");
	}

	if (customizationParameters == NULL) {
		throw invalid_argument("customizationParameters must not be NULL");
	}

	this->pathToMsi = pathToMsi;
	wcout << "pathto MSI:"<< pathToMsi<<"\n";
	this->customizationParameters = customizationParameters;
	this->pathToMsiCopy = NULL;

}

GPOTransformGenerator::~GPOTransformGenerator() {
	delete this->pathToMsiCopy;
}

void GPOTransformGenerator::generateTransform() {
    wcout << "Copy Original MSI" <<"\n";
	this->copyOriginalMsi();
	wcout << "Modify Original MSI" <<"\n";
	this->modifyCopiedMsi();
	wcout << "Create Transform" <<"\n";
	this->createTransorm();
	wcout << "Delete Msi Copy" <<"\n";
	this->deleteMsiCopy();
}

void GPOTransformGenerator::copyOriginalMsi() {
	ifstream inputStream(this->pathToMsi, ios::binary); 
	ofstream outputStream(this->getPathToMsiCopy(), ios::binary);

	outputStream << inputStream.rdbuf();

	outputStream.flush();
	outputStream.close();
	inputStream.close();

}

void GPOTransformGenerator::modifyCopiedMsi() {
	int result;
	wchar_t* pathToMsiCopy;
	wchar_t* EnableService;
	MSIHANDLE msiDatabase;
	MSIHANDLE propertyTableView;
	MSIHANDLE EnableServiceRecord;

	pathToMsiCopy = this->getPathToMsiCopy();
	msiDatabase = 0;
	wcout << "Open MSI file" <<"\n";
	result = MsiOpenDatabase(pathToMsiCopy, MSIDBOPEN_TRANSACT, &msiDatabase);
	if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to open msi database copy.  Result: " + result);
	}

	// Modify dabs location property
	// icenet property format <machineName>:<port> (ports can be at most 5 digits)
	
	EnableService = new wchar_t[2];
	wcscat(EnableService, L"0");
	wcout << "Create new property value" <<"\n";
	EnableServiceRecord = MsiCreateRecord(1);
	result = MsiRecordSetString(EnableServiceRecord, 1, EnableService);
	if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to create icenet location database record.  Result: " + result);
	}
    wcout << "Generate SQL command" <<"\n";
	/*result = MsiDatabaseOpenView(msiDatabase, 
			 L"INSERT INTO Property (Value, Property) VALUES (?, 'ENABLE_SERVICE_START_ON_INSTALL')", 
			 &propertyTableView); 
			 */
	result = MsiDatabaseOpenView(msiDatabase, 
			 L"UPDATE Property set Value='0' where Property='ENABLE_SERVICE_START_ON_INSTALL'", 
			 &propertyTableView);	
    if (result == ERROR_BAD_QUERY_SYNTAX) {
         wcout << "Incorrect SQL Syntax" <<"\n";
	}	
    if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to open Property table in copied msi database.  Result: " + result);
	}
	wcout << "Execute SQL COMMAND" <<"\n";
	result = MsiViewExecute(propertyTableView, EnableServiceRecord);
	if (result != ERROR_SUCCESS) {
	    wcout << "Execute property" <<"\n";
		throw runtime_error("Unable to insert icenet location property record.  Result: " + result);
	}
	wcout << "Commit change to copy" <<"\n";
	result = MsiDatabaseCommit(msiDatabase);
	if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to commit msi database changes.  Result: " + result);
	}

	delete EnableService;
	

	MsiCloseHandle(msiDatabase);
	MsiCloseHandle(propertyTableView);
	MsiCloseHandle(EnableServiceRecord);
}

void GPOTransformGenerator::createTransorm() {
	int result;
	MSIHANDLE originalMsiDatabase;
	MSIHANDLE copiedMsiDatabase;
	wchar_t* pathToMsiCopy;

	originalMsiDatabase = 0;
	result = MsiOpenDatabase(this->pathToMsi, MSIDBOPEN_TRANSACT, &originalMsiDatabase);
	if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to open original msi database.  Result: " + result);
	}

	pathToMsiCopy = this->getPathToMsiCopy();
	copiedMsiDatabase = 0;
	result = MsiOpenDatabase(pathToMsiCopy, MSIDBOPEN_TRANSACT, &copiedMsiDatabase);
	if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to open msi database copy.  Result: " + result);
	}

	result = MsiDatabaseGenerateTransform(copiedMsiDatabase, originalMsiDatabase, L"GPOTransform.mst", 0, 0);
	if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to create msi transform.  Result: " + result);
	}

	result = MsiCreateTransformSummaryInfo(copiedMsiDatabase, originalMsiDatabase, L"GPOTransform.mst", 0, MSITRANSFORM_VALIDATE_UPDATEVERSION);
	if (result != ERROR_SUCCESS) {
		throw runtime_error("Unable to create msi transform summary information.  Result: " + result);
	}

	MsiCloseHandle(originalMsiDatabase);
	MsiCloseHandle(copiedMsiDatabase);
}

void GPOTransformGenerator::deleteMsiCopy() {
	_wremove(this->getPathToMsiCopy());
}

wchar_t* GPOTransformGenerator::getPathToMsiCopy() {
	size_t pathToMsiLength;
	wchar_t* startOfNameModification;

	if (this->pathToMsiCopy == NULL) {
		pathToMsiLength = wcslen(this->pathToMsi);
		this->pathToMsiCopy = new wchar_t[pathToMsiLength + 5 + 1];
		wcscpy(this->pathToMsiCopy, this->pathToMsi);
		startOfNameModification = this->pathToMsiCopy + pathToMsiLength - 4;
		wcscpy(startOfNameModification, L"-copy.msi");
	}

	return this->pathToMsiCopy;
}