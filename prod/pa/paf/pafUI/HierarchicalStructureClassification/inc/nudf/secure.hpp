

#ifndef __NX_SECURE_HPP__
#define __NX_SECURE_HPP__


#include <Windows.h>
#include <assert.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <Sddl.h>

#include <algorithm>
#include <string>
#include <vector>



namespace NX {


class secure_mem
{
public:
    secure_mem();
    secure_mem(const std::vector<unsigned char> &data);
    ~secure_mem();

    secure_mem& operator = (const secure_mem& other) noexcept;
    void encrypt(const std::vector<unsigned char>& data);
    std::vector<unsigned char> decrypt() const;
    void clear() noexcept;


    inline size_t size() const noexcept { return m_size; }
    inline bool empty() const noexcept { return (0 == m_size); }

private:
    std::vector<unsigned char> m_buffer;
    size_t m_size;
};


class winsid
{
public:
    winsid();
    winsid(const std::wstring& sid);
    winsid(PSID sid);
    virtual ~winsid();

    void clear();
    winsid& operator = (const winsid& other);

    std::wstring to_string() const noexcept;
    bool from_string(const std::wstring& sid);
    bool from_sid(PSID sid) noexcept;

    inline bool is_valid() const { return (NULL != _sid); }
    inline operator PSID() const { return _sid; }

private:
    PSID    _sid;
};

class security_attribute
{
public:
    security_attribute();
    virtual ~security_attribute();

    inline operator LPSECURITY_ATTRIBUTES () noexcept { return is_valid() ? (&_sa) : NULL; }
    inline bool is_valid() const noexcept { return (NULL != _sa.lpSecurityDescriptor && NULL != _pdacl && !_eas.empty()); }

    void clear() noexcept;
    bool generate() noexcept;
    bool add_acl(PSID sid, unsigned long access_permissions, TRUSTEE_TYPE trustee_type, ACCESS_MODE access_mode = SET_ACCESS, unsigned long inheritance = NO_INHERITANCE) noexcept;
    bool add_acl(const std::wstring& sid, unsigned long access_permissions, TRUSTEE_TYPE trustee_type, ACCESS_MODE access_mode = SET_ACCESS, unsigned long inheritance = NO_INHERITANCE) noexcept;
    bool add_acl_for_wellknown_group(SID_IDENTIFIER_AUTHORITY authority, unsigned long rid, unsigned long access_permissions, unsigned long inheritance = NO_INHERITANCE) noexcept;
    bool add_acl_for_wellknown_group2(SID_IDENTIFIER_AUTHORITY authority, unsigned long rid1, unsigned long rid2, unsigned long access_permissions, unsigned long inheritance = NO_INHERITANCE) noexcept;


protected:
    void inter_add_acl(PSID sid, unsigned long access_permissions, TRUSTEE_TYPE trustee_type, ACCESS_MODE access_mode, unsigned long inheritance) noexcept;


private:
    SECURITY_ATTRIBUTES             _sa;
    PACL                            _pdacl;
    std::vector<EXPLICIT_ACCESS>    _eas;
};


class sa_everyone : public security_attribute
{
public:
    sa_everyone(unsigned long access_permissions);
    virtual ~sa_everyone() {}
};


}   // namespace NX



#endif