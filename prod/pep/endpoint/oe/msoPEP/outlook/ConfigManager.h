#pragma once

class CConfigSection
{
public:
	void LoadConfigSection(const wchar_t* wszFileName, const wchar_t* wszSectionName);
	const std::wstring& operator[](const wchar_t* wszKeyName) const;

protected:
	std::wstring GetIniString(const wchar_t* wszFileName, const wchar_t* wszSectionName, const wchar_t* wszKeyName);

private:
	std::map<std::wstring, std::wstring> m_mapCfgValue;
};

class CConfigManager
{
public:
	CConfigManager();
	~CConfigManager();
	BOOL LoadConfig();
	const CConfigSection& operator[](const wchar_t* wszSectionName) const;
	int QueryPCTimeout();
	std::wstring GetXHeaderKeyPrefix();
private:
	std::map<std::wstring, CConfigSection> m_mapCfgSections;
};

extern CConfigManager theCfg;

