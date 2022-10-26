

#ifndef __NUDF_NXL_FILE_HPP__
#define __NUDF_NXL_FILE_HPP__


#include <string>
#include <vector>
#include <map>
#include <set>

#include <nudf\secure.hpp>

namespace NX {
namespace NXL {

class nxl_section
{
public:
    nxl_section()
        : _offset(0), _size(0), _checksum(0) {}
    nxl_section(const std::wstring& name, unsigned long offset, unsigned long size, unsigned long checksum)
        : _name(name), _offset(offset), _size(size), _checksum(checksum) {}
    virtual ~nxl_section() {}

    inline const std::wstring& name() const noexcept { return _name; }
    inline unsigned long offset() const noexcept { return _offset; }
    inline unsigned long size() const noexcept { return _size; }
    inline unsigned long checksum() const noexcept { return _checksum; }

    inline void set_checksum(unsigned long checksum) noexcept { _checksum = checksum; }

    inline bool empty() const noexcept { return _name.empty(); }
    inline void clear() noexcept
    {
        _name.clear();
        _offset = 0;
        _size = 0;
        _checksum = 0;
    }

    nxl_section& operator = (const nxl_section& other)
    {
        if (this != &other) {
            _name = other.name();
            _offset = other.offset();
            _size = other.size();
            _checksum = other.checksum();
        }
        return *this;
    }

private:
    std::wstring    _name;
    unsigned long   _offset;
    unsigned long   _size;
    unsigned long   _checksum;
};

class nxl_key_id
{
public:
    nxl_key_id();
    nxl_key_id(unsigned long algorithm, const unsigned char* id, unsigned long size);
    nxl_key_id(unsigned long algorithm, const std::vector<unsigned char>& id);
    virtual ~nxl_key_id();

    inline unsigned long algorithm() const noexcept { return _algorithm; }
    inline const unsigned char* id() const noexcept { return _id.empty() ? NULL : _id.data(); }
    inline unsigned long size() const noexcept { return (unsigned long)_id.size(); }

    inline bool empty() const noexcept { return (0 == size()); }
    inline void clear() noexcept { _algorithm = 0; _id.clear(); }

    nxl_key_id& operator = (const  nxl_key_id& other);
    bool operator == (const  nxl_key_id& other) const noexcept;

private:
    unsigned long   _algorithm;
    std::vector<unsigned char> _id;
};

class nxl_key_pkg
{
public:
    nxl_key_pkg();
    nxl_key_pkg(const nxl_key_id& id, const unsigned char* key, const unsigned long size);
    nxl_key_pkg(const nxl_key_id& id, const std::vector<unsigned char>& key);
    nxl_key_pkg(const nxl_key_id& id, const NX::secure_mem& key);
    virtual ~nxl_key_pkg();


    inline bool empty() const noexcept { return (_id.empty() || _key.empty()); }
    inline void clear() noexcept { _id.clear(); _key.clear(); }

    inline const nxl_key_id& id() const noexcept { return _id; }
    inline const NX::secure_mem& key() const noexcept { return _key; }
    inline std::vector<unsigned char> plain_key() const noexcept { return _key.decrypt(); }

    nxl_key_pkg& operator = (const nxl_key_pkg& other) noexcept;

private:
    nxl_key_id      _id;
    NX::secure_mem  _key;
};

class nxl_file;

class nxl_header
{
public:
    nxl_header();
    virtual ~nxl_header();

    void load(const std::wstring& file);
    void load(HANDLE h);
    void create(const std::wstring& message, const nxl_key_pkg& primary_key, const nxl_key_pkg& recovery_key, const std::map<std::wstring, unsigned long>& section);
    void clear() noexcept;

    std::vector<unsigned char> to_buffer();

    void validate_sections(const std::vector<unsigned char>& key, HANDLE h = INVALID_HANDLE_VALUE /*optional*/) const;
    void validate_section(const std::wstring& name, HANDLE h = INVALID_HANDLE_VALUE /*optional*/) const;
    void validate_section(const nxl_section& sc, HANDLE h) const;

    inline bool empty() const noexcept { return (0 == _version || _thumbprint.empty()); }

    inline const std::wstring& message() const noexcept { return _message; }
    inline const std::wstring& thumbprint() const noexcept { return _thumbprint; }
    inline unsigned long version() const noexcept { return _version; }
    inline unsigned long flags() const noexcept { return _flags; }
    inline unsigned long alignment() const noexcept { return _alignment; }
    inline unsigned long content_offset() const noexcept { return _content_offset; }
    inline unsigned long algorithm() const noexcept { return _algorithm; }
    inline unsigned long cbc_size() const noexcept { return _cbc_size; }
    inline const nxl_key_id& primary_key_id() const noexcept { return _primary_key_id; }
    inline const std::vector<unsigned char>& primary_content_key() const noexcept {return _primary_content_key; }
    inline const nxl_key_id& recovery_key_id() const noexcept { return _recovery_key_id; }
    inline const std::vector<unsigned char>& recovery_content_key() const noexcept {return _recovery_content_key; }
    inline const std::vector<unsigned char>& section_table_checksum() const noexcept { return _sections_checksum; }

    __int64 content_length() const noexcept { return _content_size; }

    bool is_content_key_aes128() const noexcept;
    bool is_content_key_aes256() const noexcept;
    bool is_primary_key_aes128() const noexcept;
    bool is_primary_key_aes256() const noexcept;
    inline bool is_primary_key_aes() const noexcept { return (is_primary_key_aes128() || is_primary_key_aes256()); }
    inline bool has_recovery_key() const noexcept { return (!_recovery_key_id.empty()); }
    bool is_recovery_key_aes128() const noexcept;
    bool is_recovery_key_aes256() const noexcept;
    bool is_recovery_key_rsa1024() const noexcept;
    bool is_recovery_key_rsa2048() const noexcept;
    inline bool is_recovery_key_aes() const noexcept { return (is_recovery_key_aes128() || is_recovery_key_aes256()); }
    inline bool is_recovery_key_rsa() const noexcept { return (is_recovery_key_rsa1024() || is_recovery_key_rsa2048()); }

    std::vector<unsigned char> decrypt_content_key(const std::vector<unsigned char>& key) const noexcept;
    std::vector<unsigned char> recovery_content_key(const std::vector<unsigned char>& key) const noexcept;
    unsigned long decrypt_sections_checksum(const std::vector<unsigned char>& key) const noexcept;
    void update_sections_checksum(const std::vector<unsigned char>& key);
    unsigned long calc_sections_checksum() const noexcept;

    inline const std::vector<nxl_section>& sections() const noexcept { return _sections; }
    inline std::vector<nxl_section>& sections() noexcept { return _sections; }
    

private:
    std::wstring    _message;
    std::wstring    _thumbprint;
    unsigned long   _version;
    unsigned long   _flags;
    unsigned long   _alignment;
    unsigned long   _content_offset;
    unsigned long   _algorithm;
    unsigned long   _cbc_size;
    nxl_key_id      _primary_key_id;
    // encrypted key data
    std::vector<unsigned char>  _primary_content_key;
    nxl_key_id      _recovery_key_id;
    // encrypted key data
    std::vector<unsigned char>  _recovery_content_key;
    __int64         _content_size;
    __int64         _allocation_size;
    std::vector<nxl_section>    _sections;
    std::vector<unsigned char>  _sections_checksum;

    friend class nxl_file;
};


class nxl_file
{
public:
    typedef std::map<std::wstring, std::wstring>    AttributeMapType;
    typedef std::set<std::wstring>                  TagValueMapType;
    typedef std::map<std::wstring, TagValueMapType> TagMapType;

public:
    nxl_file();
    virtual ~nxl_file();

    inline bool empty() const noexcept { _header.empty(); }
    inline bool opened() const noexcept { return (_h != INVALID_HANDLE_VALUE); }
    inline bool read_only() const noexcept { return _read_only; }
    inline const nxl_header& header() const noexcept { return _header; }
    inline const std::wstring& file_name() const noexcept { return _file_name; }

    void create(const std::wstring& target,
                const std::wstring& source,
                const nxl_key_pkg& primary_key,
                const nxl_key_pkg& recovery_key,
                const std::wstring& message,
                const std::map<std::wstring, unsigned long>& sections);
    void convert(const std::wstring& file,
                 const nxl_key_pkg& primary_key,
                 const nxl_key_pkg& recovery_key,
                 const std::wstring& message,
                 const std::map<std::wstring, unsigned long>& sections);
    void open(const std::wstring& source, const nxl_key_pkg& key_package, bool read_only = true);
    void open(const std::wstring& source, const std::vector<nxl_key_pkg>& key_pkgs = std::vector<nxl_key_pkg>(), bool read_only = true);
    void decrypt(const std::wstring& target);
    void close() noexcept;

    void add_section(const std::vector<std::pair<std::wstring, unsigned long>>& sections);
    void remove_section(const std::vector<std::pair<std::wstring, unsigned long>>& sections);

    std::vector<unsigned char> get_section_data(const std::wstring& name, bool* validated);
    void set_section_data(const std::wstring& name, const std::vector<unsigned char>& data);
    void set_section_data(const std::wstring& name, const unsigned char* data, unsigned long size);

    AttributeMapType get_nxl_attributes(bool* validated);
    void set_nxl_attributes(const AttributeMapType& attributes);

    bool is_remote_eval(bool* validated);
    void set_remote_eval(bool b);

    TagMapType get_nxl_tags(bool* validated);
    void set_nxl_tags(const TagMapType& tags);

    static bool verify(const std::wstring& file) noexcept;

    static std::wstring find_nxl_attribute(const AttributeMapType& attributes, const std::wstring& name) noexcept;
    static void set_nxl_attribute(AttributeMapType& attributes, const std::wstring& name, const std::wstring& value) noexcept;
    static bool remove_nxl_attribute(AttributeMapType& attributes, const std::wstring& name) noexcept;

    static TagValueMapType find_nxl_tags(const TagMapType& tags, const std::wstring& name) noexcept;
    static void set_nxl_tag(TagMapType& tags, const std::wstring& name, const std::wstring& value) noexcept;
    static bool remove_nxl_tag(TagMapType& tags, const std::wstring& name) noexcept;
    static bool remove_nxl_tag(TagMapType& tags, const std::wstring& name, const TagValueMapType& values) noexcept;

protected:
    ULONG_PTR round_to_size(unsigned long length, unsigned long alignment)
    {
        //  This macro takes a length & rounds it up to a multiple of the alignment
        //  Alignment is given as a power of 2
        return ((((ULONG_PTR)(length)) + ((alignment)-1)) & ~(ULONG_PTR)((alignment)-1));
    }

    bool is_aligned(unsigned long length, unsigned long alignment)
    {
        //  Checks if 1st argument is aligned on given power of 2 boundary specified
        //  by 2nd argument
        return ((((ULONG_PTR)(length)) & ((alignment)-1)) == 0);
    }

    bool attribute_on(unsigned long file_attributes, unsigned long attribute) const
    {
        return ((INVALID_FILE_ATTRIBUTES != file_attributes) && (0 != (file_attributes & attribute)));
    }

    std::wstring get_temp_file_name(const std::wstring& file_or_dir);

    void update_section_table_checksum();
    
    std::vector<unsigned char> create_tags_buffer_legacy(const TagMapType& tags);
    std::vector<unsigned char> create_tags_buffer_json(const TagMapType& tags);

private:
    HANDLE      _h;
    nxl_header  _header;
    bool        _read_only;
    NX::secure_mem _content_key;
    std::wstring _file_name;
};


}   // namespace NXL
}   // namespace NX



#endif