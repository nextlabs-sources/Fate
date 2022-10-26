#ifndef __ATTACHMENT_ADAPTERCOMMON_H__
#define __ATTACHMENT_ADAPTERCOMMON_H__

#include "obligation.h"
#include <vector>
#include <string>
#include <atlcomcli.h>
namespace AdapterCommon
{
	class Attachment
	{
	public:
		Attachment():bNeedStrip(false),bRemoved(false) {vecObligations.clear();};
		~Attachment() {vecObligations.clear();};
		void SetSrcPath(WCHAR * wzSrcPath) {wstrSrcPath=wzSrcPath;};
		std::wstring GetSrcPath(){return wstrSrcPath;};

		void SetTempPath(WCHAR * wzTempPath) {wstrTempPath=wzTempPath;};
		std::wstring GetTempPath(){return wstrTempPath;};

		void SetReturnPath(WCHAR * wzReturnPath) {wstrReturnPath=wzReturnPath;};

		Obligation Item(int i){return vecObligations[i];};
		size_t Count(){return vecObligations.size();};

		bool	SetStripFlag(bool newValue){bool oldValue=bNeedStrip;bNeedStrip=newValue;return oldValue;};
		bool	GetStripFlag(){return bNeedStrip;};

		bool	SetRemovedFlag(bool newValue){bool oldValue=bRemoved;bRemoved=newValue;return oldValue;};
		bool	GetRemovedFlag(){return bRemoved;};
		void AddObligation(Obligation ob)
		{
			vecObligations.push_back(ob);
		}
	private:
		std::wstring	wstrSrcPath;
		std::wstring	wstrTempPath;
		std::wstring	wstrReturnPath;
		bool			bNeedStrip;
		bool			bRemoved;
		std::vector<Obligation>		vecObligations;
	};

	class Attachments
	{
	public:
		Attachments(){vecAttachments.clear();};
		~Attachments(){vecAttachments.clear();};

		void FreeAttachments(){vecAttachments.clear();}
		void AddAttachment(Attachment att){	vecAttachments.push_back(att);};
		size_t Count(){return vecAttachments.size();};
		Attachment& Item(size_t i)
		{
			//Attachment att=vecAttachments[i];
			return vecAttachments[i];
		}
	private:
		std::vector<Attachment> vecAttachments;
	};
}



#endif
