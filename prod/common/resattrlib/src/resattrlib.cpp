/*************************************************************************
 *
 * Compliant Enterprise Resource Attribute Library
 *
 *
 ************************************************************************/



#include "../include/resattrlib.h"
#ifdef TAGLIB_WINDOWS
//#include <winsock2.h>
#ifdef BUILDING_RESATTRLIB
#undef BUILDING_RESATTRLIB
#endif
#define BUILDING_RESATTRLIB
#endif

#include <string>
#include <cassert>
#include <vector>

class NameValue
{
public:
    
#ifdef TAGLIB_WINDOWS
	NameValue(const WCHAR *name, const WCHAR *value) : m_name(name), m_value(value) { }
	std::wstring m_name;
    std::wstring m_value;
#else
	NameValue(const char *name, const char *value) : m_name(name), m_value(value) { }
    std::string m_name;
    std::string m_value;
#endif
};

struct ResourceAttributes
{
private:
    std::vector <NameValue> data;

public:  
#ifdef TAGLIB_WINDOWS
	void add(const WCHAR *name, const WCHAR *value)
    {
        data.push_back(NameValue(name, trimBeginEndSpace(value).c_str()));
        return;
    }
#else
    void add(const char *name, const char *value)
    {
        data.push_back(NameValue(name, value));
        return;
    }
#endif

#ifdef TAGLIB_WINDOWS
    void add(const char *name, const char *value)
    {
        size_t size = 0;
		mbstowcs_s(&size, NULL, 0, name, 0);

        WCHAR *namebuf = new WCHAR[size];
        mbstowcs_s(&size, namebuf, size, name, _TRUNCATE);

		mbstowcs_s(&size, NULL, 0, value, 0);
        WCHAR *valuebuf = new WCHAR[size];
        mbstowcs_s(&size, valuebuf, size, value, _TRUNCATE);

        data.push_back(NameValue(namebuf, trimBeginEndSpace(valuebuf).c_str()));
        delete[] namebuf;
        delete[] valuebuf;

        return;
    }
#endif

const
#ifdef TAGLIB_WINDOWS
	WCHAR*
#else
    char *
#endif
	getName(size_t index) const
    {
        if (index >= data.size())
        {
            return NULL;
        }
        else
        {
            return data[index].m_name.c_str();
        }
    }
const
#ifdef TAGLIB_WINDOWS
	WCHAR*
#else
    char *
#endif
	getValue(size_t index) const
    {
        if (index >= data.size())
        {
            return NULL;
        }
        else
        {
            return data[index].m_value.c_str();
        }
    }
	

#ifdef TAGLIB_WINDOWS
	void setValue(size_t index,const WCHAR*value)
	{
		if(index>=data.size())
			return;
		else
		{
			data[index].m_value=trimBeginEndSpace(value);
		}
	}
#endif
	void setValue(size_t index,const char*value)
	{
#ifdef TAGLIB_WINDOWS
		size_t size = 0;
		mbstowcs_s(&size, NULL, 0, value, 0);
		WCHAR *valueIn = new WCHAR[size];
		mbstowcs_s(&size, valueIn, size, value, _TRUNCATE);
#else
		const char* valueIn=value;
#endif
		if(index>=data.size())
			return;
		else
		{
			data[index].m_value=trimBeginEndSpace(valueIn);
		}
	}

    size_t size() const
    {
        return data.size();
    }
    /*
    Return 0: didn't find the attribute name
    1: found
    */
#ifdef TAGLIB_WINDOWS
	int findAttr(const WCHAR*name,int&idx)
    {
    	for(unsigned int i=0;i<data.size();i++)
    	{
    		if(wcscmp(data[i].m_name.c_str(),name)==0)
    		{
    			idx=i;
    			return 1;
    		}
    	}
    	return 0;	
    }
#else
    int findAttr(const char*name,int&idx)
    {
    	for(unsigned int i=0;i<data.size();i++)
    	{
    		if(strcmp(data[i].m_name.c_str(),name)==0)
    		{
    			idx=i;
    			return 1;
    		}
    	}
    	return 0;	
    }
#endif

	static std::wstring trimBeginEndSpace(const std::wstring& s)
	{
		const std::wstring drop = L" \f\n\r\t\v";
		std::wstring temp = s;
		temp.erase(temp.find_last_not_of(drop) + 1);
		return temp.erase(0, temp.find_first_not_of(drop));
	}

};


int AllocAttributes(ResourceAttributes **attrs)
{
    *attrs = new ResourceAttributes();
    return 0;
}

void AddAttributeA(ResourceAttributes *attrs, const char *name, const char *value)
{
    if (attrs == NULL || name == NULL || value == NULL)
    {
        return;
    }

    attrs->add(name, value);
}
#ifdef TAGLIB_WINDOWS
void AddAttributeW(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value)
{
    if (attrs == NULL || name == NULL || value == NULL)
    {
        return;
    }

    attrs->add(name, value);
}
#endif

const
#ifdef TAGLIB_WINDOWS
WCHAR*
#else
char *
#endif
GetAttributeName(const ResourceAttributes *attrs, int index)
{
    assert(attrs != NULL);
    assert(index >= 0);

    if (attrs == NULL || index < 0)
    {
        return NULL;
    }

    return attrs->getName(index);
}

const
#ifdef TAGLIB_WINDOWS
WCHAR*
#else
char *
#endif
GetAttributeValue(const ResourceAttributes *attrs, int index)
{
    assert(attrs != NULL);
    assert(index >= 0);

    if (attrs == NULL || index < 0)
    {
        return NULL;
    }

    return attrs->getValue(index);
}


void SetAttributeValueA(ResourceAttributes *attrs, int index,const char* value)
{
    assert(attrs != NULL);
    assert(index >= 0);

    if (attrs == NULL || index < 0)
    	return;

    attrs->setValue(index,value);
}

#ifdef TAGLIB_WINDOWS
void SetAttributeValueW(ResourceAttributes *attrs, int index,const WCHAR* value)
{
    assert(attrs != NULL);
    assert(index >= 0);

    if (attrs == NULL || index < 0)
    	return;

    attrs->setValue(index,value);
}
#endif
int GetAttributeCount(const ResourceAttributes *attrs)
{
	assert(attrs != NULL);
    return (int)attrs->size();
}

void FreeAttributes(ResourceAttributes *attrs)
{
	assert(attrs != NULL);
    delete attrs;
    return;
}

int FindAttributeA(ResourceAttributes *attrs, const char* name,int& idx)
{
	assert(attrs != NULL);
    assert(name != NULL);
#ifdef TAGLIB_WINDOWS
	size_t size = 0;
	mbstowcs_s(&size, NULL, 0, name, 0);
	WCHAR *nameIn = new WCHAR[size];
	mbstowcs_s(&size, nameIn, size, name, _TRUNCATE);
#else
	const char*nameIn=name;
#endif
    return attrs->findAttr(nameIn,idx);
}
#ifdef TAGLIB_WINDOWS
int FindAttributeW(ResourceAttributes *attrs, const WCHAR* name,int& idx)
{
	assert(attrs != NULL);
    assert(name != NULL);
    return attrs->findAttr(name,idx);
}
#endif
