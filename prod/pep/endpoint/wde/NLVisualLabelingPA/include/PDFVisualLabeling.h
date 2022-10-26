#pragma once

class CPDFVL
{
public:
	static bool DoVisualLabeling(const wstring& strFilePath, const PABase::ACTION& theAction,const NM_VLObligation::VisualLabelingInfo& newInfo,HWND hView = NULL);
};
