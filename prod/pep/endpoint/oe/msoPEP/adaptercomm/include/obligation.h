#ifndef __OBLIGATION_ADAPTERCOMM_H__
#define __OBLIGATION_ADAPTERCOMM_H__

#include <string>
#include <vector>
#include <algorithm>

namespace AdapterCommon
{
	class Attribute
	{
	public:
		Attribute(const WCHAR* wzName,const WCHAR* wzValue):wstrName(wzName),wstrValue(wzValue){};
		void SetAttr(WCHAR* wzName,WCHAR* wzValue)
		{
			wstrName=wzName;
			wstrValue=wzValue;
		}
		void GetAttr(std::wstring& strName,std::wstring& strValue)
		{
			strName=wstrName;
			strValue=wstrValue;
		}
		std::wstring GetValue(){return wstrValue;}
		std::wstring GetName(){return wstrName;}
		bool operator == (const Attribute& attr)
		{
			return wstrName==attr.wstrName;
		}
			
	private:
		//int				cbSize;
		std::wstring	wstrName;
		std::wstring	wstrValue;
	};
	class Obligation
	{
	public:
		~Obligation(){vecAttributes.clear();};
		void SetName(const WCHAR*wzName)
		{
			wstrObligationName=wzName;
		}
		std::wstring GetName(){return wstrObligationName;}
		void GetName(std::wstring&strName){strName=wstrObligationName;}
		void AddAttribute(Attribute attr)
		{
			vecAttributes.push_back(attr);
		}
		bool FindAttribute(const WCHAR*wzName,std::wstring& strValue)
		{
			Attribute attr(wzName,L"");
			std::vector<Attribute>::iterator it=std::find(vecAttributes.begin(),vecAttributes.end(),attr);
			if(it!=vecAttributes.end())
			{
				strValue=it->GetValue();
				return true;
			}
			else
				return false;
		}
	private:
		//int									cbSize;
		std::wstring						wstrObligationName;
		std::vector<Attribute>	vecAttributes;
	};
}
#endif
