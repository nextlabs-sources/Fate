
#include <windows.h>
#include <stdio.h>

#include "nldevcon.hpp"
#include "install.hpp"
#include "enum.hpp"

static int Usage();

int wmain(int argc, wchar_t** argv)
{
    int nRet = NLDEVCON_SUCCESS;
    int i    = 0;

    if(argc < 2) return Usage();

    if(0 == _wcsicmp(argv[1], L"install"))
    {
        if(4 != argc) return Usage();
        return cmdInstall(argv[2], argv[3]);

    }
    else if(0 == _wcsicmp(argv[1], L"remove"))
    {
        if(3 != argc) return Usage();
        return cmdUninstall(argv[2]);
    }
    else if(0 == _wcsicmp(argv[1], L"query"))
    {
        NLDEVCON_ENUM_RESTRICTIONS Restrictions;
        Restrictions.ClassName = NULL;
        Restrictions.ClassGuid = NULL;
        Restrictions.HwId      = NULL;
        for(i=2; i<argc; i++)
        {
            if(0 == _wcsnicmp(argv[i], L"Class=", 6))
            {
                Restrictions.ClassName = (argv[i])+6;
            }
            else if(0 == _wcsnicmp(argv[i], L"ClassGuid=", 10))
            {
                Restrictions.ClassGuid = (argv[i])+10;
            }
            else if(0 == _wcsnicmp(argv[i], L"HwId=", 5))
            {
                Restrictions.HwId = (argv[i])+5;
            }
            else
            {
                ;
            }
        }

        return cmdQuery(&Restrictions);
    }
    else
    {
        nRet = Usage();
    }

    return nRet;
}

int Usage()
{
    printf("NLDEVCON Usage:\n");
    printf("  nldevcon [/? | -? | -h | --help]\n");
    printf("  nldevcon install <inf> <hwId>\n");
    printf("  nldevcon remove <hwId>\n");
    printf("  nldevcon query [Class=<Class Name>] [ClassGuid=<Class GUID>] [HwId=<Hardware Id>]\n");
    return NLDEVCON_HELP;
}