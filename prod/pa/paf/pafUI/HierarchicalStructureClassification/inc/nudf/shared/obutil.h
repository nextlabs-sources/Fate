

#ifndef __NUDF_SHARE_OBLIGATION_UTIL_H__
#define __NUDF_SHARE_OBLIGATION_UTIL_H__

#include <Windows.h>

#include <string>
#include <map>
#include <memory>

#include <nudf\shared\obdef.h>

namespace nudf {
namespace util {


typedef std::map<std::wstring, std::wstring>    OBPARAMS;

class CObligation
{
public:
    CObligation() : _id(0) {}
    CObligation(_In_ USHORT id) : _id(id) {}
    virtual ~CObligation() {}

    inline bool IsValid() const throw() {return (0 != _id);}
    inline USHORT GetId() const throw() {return _id;}
    inline void SetId(_In_ USHORT id) throw() {_id = id;}
    inline const OBPARAMS& GetParams() const throw() {return _params;}
    inline OBPARAMS& GetParams() throw() {return _params;}

    CObligation& operator = (const CObligation& ob) throw()
    {
        if(this != &ob) {
            _id = ob.GetId();
            _params = ob.GetParams();
        }
        return *this;
    }

    bool operator == (const CObligation& ob) throw()
    {
        return (_id == ob.GetId());
    }

    virtual void Clear() throw()
    {
        _id = 0;
        _params.clear();
    }

    ULONG GetBlobSize() const throw() {

        ULONG paramsize = 0;

        for(OBPARAMS::const_iterator it=_params.begin(); it!=_params.end(); ++it) {
            if((*it).first.empty() || (*it).second.empty()) {
                continue;
            }
            paramsize += (ULONG)((*it).first.length()*sizeof(WCHAR) + (*it).second.length()*sizeof(WCHAR));
        }

        return ((ULONG)(sizeof(NXRM_OBLIGATION) + paramsize));
    }

    bool ToBlob(_Out_ PUCHAR blob, _Inout_ PULONG size) const throw()
    {
        ULONG desired_size = GetBlobSize();
        PNXRM_OBLIGATION ob = (PNXRM_OBLIGATION)blob;
        PWCHAR params = NULL;

        if(desired_size > *size) {
            SetLastError(ERROR_BUFFER_OVERFLOW);
            *size = 0;
            return false;
        }

        memset(blob, 0, *size);
        ob->NextOffset = 0;
        ob->Id = _id;
        params = ob->Params;
        for(OBPARAMS::const_iterator it=_params.begin(); it!=_params.end(); ++it) {
            if((*it).first.empty() || (*it).second.empty()) {
                continue;
            }
            memcpy(params, (*it).first.c_str(), (*it).first.length()*sizeof(WCHAR));
            params += ((*it).first.length() + 1);
            memcpy(params, (*it).second.c_str(), (*it).second.length()*sizeof(WCHAR));
            params += ((*it).second.length() + 1);
        }

        return true;
    }

    void FromBlob(_In_ const NXRM_OBLIGATION* ob)
    {
        PCWCHAR params = ob->Params;
        _id = ob->Id;
        while(L'\0' != params[0]) {

            std::wstring wspair(params);
            params += (wspair.length() + 1);    // move to next

            //
            //  Each PARAMETER is in following format:
            //       Name=Value
            //
            std::wstring::size_type pos = wspair.find_first_of(L"=");
            if(pos == std::wstring::npos) {
                continue;
            }
            std::wstring name = wspair.substr(0, pos);
            std::wstring value = wspair.substr(pos+1);
            if(name.empty() || value.empty()) {
                continue;
            }

            _params[name] = value;
        }
    }

private:
    USHORT          _id;
    OBPARAMS        _params;
};

typedef std::map<USHORT, CObligation>   OBS;

class CObligations
{
public:
    CObligations() {}
    virtual ~CObligations() {}

    inline bool IsEmpty() const throw() {return _obs.empty();}
    inline const OBS& GetObligations() const throw() {return _obs;}
    inline OBS& GetObligations() throw() {return _obs;}
    inline CObligations& operator = (const CObligations& obs) throw()
    {
        if(this != &obs) {
            _obs = obs.GetObligations();
        }
        return *this;
    }
    inline CObligations& Merge(const CObligations& obs) throw()
    {
        if(this != &obs) {
            const OBS& v = obs.GetObligations();
            for(OBS::const_iterator it=v.begin(); it!=v.end(); ++it) {
                _obs[(*it).first] = (*it).second;
            }
        }
        return *this;
    }

    bool Find(_In_ USHORT id, _Out_ CObligation& ob) const throw()
    {
        OBS::const_iterator it = _obs.find(id);
        if(it != _obs.end()) {
            ob = (*it).second;
            return true;
        }
        return false;
    }

    void Clear() throw()
    {
        _obs.clear();
    }
    
    ULONG GetBlobSize() const throw()
    {
        ULONG size = 0;

        for(OBS::const_iterator it=_obs.begin(); it!=_obs.end(); ++it) {
            size += (*it).second.GetBlobSize();
        }

        return size;
    }
    
    bool ToBlob(_Out_ PUCHAR blob, _Inout_ PULONG size) const throw()
    {
        ULONG valid_size = 0;


        if(NULL == size) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }

        // Calculate size only
        if(NULL == blob) {
            *size = GetBlobSize();
            return true;
        }

        // Fill data
        memset(blob, 0, *size);
        valid_size = *size;
        *size = 0;
        
        OBS::const_iterator it=_obs.begin();
        while(it != _obs.end()) {

            ULONG obsize = valid_size;

            // Set Obligation
            if(!(*it).second.ToBlob(blob, &obsize)) {
                // failed
                return false;
            }

            // Move to next item
            ++it;
            // Update Offset Info
            ((PNXRM_OBLIGATION)blob)->NextOffset = (it != _obs.end()) ? obsize : 0;
            *size += obsize;        // Total returned data size
            valid_size -= obsize;   // Reduce valid size
            blob += obsize;         // Move to next section
        }

        return true;
    }

    void FromBlob(_In_ const UCHAR* blob, _In_ ULONG size)
    {
        while(size >= sizeof(NXRM_OBLIGATION)) {
            CObligation ob;
            ob.FromBlob((const NXRM_OBLIGATION*)blob);
            _obs[ob.GetId()] = ob;

            // No more entry
            if(0 == ((const NXRM_OBLIGATION*)blob)->NextOffset) {
                break;
            }

            // Move to next entry
            blob += ((const NXRM_OBLIGATION*)blob)->NextOffset;
        }
    }

private:
    OBS     _obs;
};


}   // namespace nudf::util
}   // namespace nudf



#endif  // #ifndef __NUDF_SHARE_OBLIGATION_UTIL_H__