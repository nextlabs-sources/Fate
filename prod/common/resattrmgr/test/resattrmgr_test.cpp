#if defined (WIN32) || (_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include "resattrmgr.h"

void printAllAttributes(ResourceAttributeManager *mgr, const char *filename)
{
    ResourceAttributes *attrs;
    AllocAttributes(&attrs);
    ReadResourceAttributesA(mgr, filename, attrs);

    int size = GetAttributeCount(attrs);

    std::wcerr << "There are " << size << " attribute(s)" << std::endl;
    for (int i = 0; i < size; ++i)
    {
        std::wcerr << "Name:  " << GetAttributeName(attrs, i) << std::endl;
        std::wcerr << "Value: " << GetAttributeValue(attrs, i) << std::endl;
    }

    FreeAttributes(attrs);
}

void setAttribute(ResourceAttributeManager *mgr, const char *filename, const char *key, const char *value)
{
    ResourceAttributes *attrs;
    AllocAttributes(&attrs);
    AddAttributeA(attrs, key, value);

    WriteResourceAttributesA(mgr, filename, attrs);
    FreeAttributes(attrs);
}

void setAttributeForFiles(ResourceAttributeManager *mgr, const char *infile, const char *key, const char *value)
{
    std::ifstream input(infile, std::ifstream::in);

    std::string l;
    while(std::getline(input, l))
    {
        setAttribute(mgr, l.c_str(), key, value);
    }
    input.close();
}

void usage()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "  attrs.exe <file>                     ; print all attributes of file \'file\'" << std::endl;
    std::cout << "  attrs.exe <file> <key> <value>       ; set key=value in file \'file\'" << std::endl;
    std::cout << "  attrs.exe -list <file> <key> <value> ; set key=value for every file listed in \'file\'" << std::endl;
}

int main(int argc, char *argv[])
{
    ResourceAttributeManager *mgr;
    
    CreateAttributeManager(&mgr);

    switch(argc)
    {
        case 2:
            printAllAttributes(mgr, argv[1]);
            break;
        case 4:
            setAttribute(mgr, argv[1], argv[2], argv[3]);
            printAllAttributes(mgr, argv[1]);
            break;
        case 5:
            setAttributeForFiles(mgr, argv[2], argv[3], argv[4]);
            break;
        default:
            usage();
    }
}

