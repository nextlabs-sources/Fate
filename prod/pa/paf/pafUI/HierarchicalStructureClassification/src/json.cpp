

#include <Windows.h>

#include <nudf\web\json.hpp>
#include <nudf\web\conversions.hpp>



using namespace NX;
using namespace NX::web;
using namespace NX::web::json;


bool json::details::g_keep_json_object_unsorted = false;
void json::keep_object_element_order(bool keep_order)
{
    json::details::g_keep_json_object_unsorted = keep_order;
}

std::wostream& json::operator << (std::wostream &os, const json::value &val)
{
    val.serialize(os);
    return os;
}

std::wistream& json::operator >> (std::wistream &is, json::value &val)
{
    val = json::value::parse(is);
    return is;
}

json::value::value() :
    m_value(utility::details::make_unique<json::details::_Null>())
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::Null)
#endif
    { }

json::value::value(int32_t value) :
    m_value(utility::details::make_unique<json::details::_Number>(value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::Number)
#endif
    { }

json::value::value(uint32_t value) :
    m_value(utility::details::make_unique<json::details::_Number>(value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::Number)
#endif
    { }

json::value::value(int64_t value) :
    m_value(utility::details::make_unique<json::details::_Number>(value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::Number)
#endif
    { }

json::value::value(uint64_t value) :
    m_value(utility::details::make_unique<json::details::_Number>(value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::Number)
#endif
    { }

json::value::value(double value) :
    m_value(utility::details::make_unique<json::details::_Number>(value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::Number)
#endif
    { }

json::value::value(bool value) :
    m_value(utility::details::make_unique<json::details::_Boolean>(value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::Boolean)
#endif
    { }

json::value::value(std::wstring value) :
    m_value(utility::details::make_unique<json::details::_String>(std::move(value)))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::String)
#endif
    { }

json::value::value(std::wstring value, bool has_escape_chars) :
m_value(utility::details::make_unique<json::details::_String>(std::move(value), has_escape_chars))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
, m_kind(value::String)
#endif
{ }

json::value::value(const wchar_t* value) :
    m_value(utility::details::make_unique<json::details::_String>(value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(value::String)
#endif
    { }

json::value::value(const wchar_t* value, bool has_escape_chars) :
m_value(utility::details::make_unique<json::details::_String>(std::wstring(value), has_escape_chars))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
, m_kind(value::String)
#endif
{ }

json::value::value(const value &other) :
    m_value(other.m_value->_copy_value())
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(other.m_kind)
#endif
    { }

json::value &json::value::operator=(const value &other)
{
    if(this != &other)
    {
        m_value = std::unique_ptr<details::_Value>(other.m_value->_copy_value());
#ifdef ENABLE_JSON_VALUE_VISUALIZER
        m_kind = other.m_kind;
#endif
    }
    return *this;
}

json::value::value(value &&other) throw() :
    m_value(std::move(other.m_value))
#ifdef ENABLE_JSON_VALUE_VISUALIZER
    ,m_kind(other.m_kind)
#endif
{}

json::value &json::value::operator=(json::value &&other) throw()
{
    if(this != &other)
    {
        m_value.swap(other.m_value);
#ifdef ENABLE_JSON_VALUE_VISUALIZER
        m_kind = other.m_kind;
#endif
    }
    return *this;
}

json::value json::value::null()
{
    return json::value();
}

json::value json::value::number(double value)
{
    return json::value(value);
}

json::value json::value::number(int32_t value)
{
    return json::value(value);
}

json::value json::value::number(int64_t value)
{
    return json::value(value);
}

json::value json::value::number(uint32_t value)
{
    return json::value(value);
}

json::value json::value::number(uint64_t value)
{
    return json::value(value);
}

json::value json::value::boolean(bool value)
{
    return json::value(value);
}

json::value json::value::string(std::wstring value)
{
    std::unique_ptr<details::_Value> ptr = NX::utility::details::make_unique<details::_String>(std::move(value));
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::String
#endif
            );
}

json::value json::value::string(std::wstring value, bool has_escape_chars)
{
    std::unique_ptr<details::_Value> ptr = NX::utility::details::make_unique<details::_String>(std::move(value), has_escape_chars);
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::String
#endif
            );
}

json::value json::value::string(const std::string &value)
{
    std::unique_ptr<details::_Value> ptr = NX::utility::details::make_unique<details::_String>(utility::conversions::to_utf16string(value));
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::String
#endif
            );
}

json::value json::value::object(bool keep_order)
{
    std::unique_ptr<details::_Value> ptr = NX::utility::details::make_unique<details::_Object>(keep_order);
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::Object
#endif
            );
}

json::value json::value::object(std::vector<std::pair<std::wstring, value>> fields, bool keep_order)
{
    std::unique_ptr<details::_Value> ptr = utility::details::make_unique<details::_Object>(std::move(fields), keep_order);
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::Object
#endif
            );
}

json::value json::value::array()
{
    std::unique_ptr<details::_Value> ptr = utility::details::make_unique<details::_Array>();
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::Array
#endif
            );
}

json::value json::value::array(size_t size)
{
    std::unique_ptr<details::_Value> ptr = utility::details::make_unique<details::_Array>(size);
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::Array
#endif
            );
}

json::value json::value::array(std::vector<value> elements)
{
    std::unique_ptr<details::_Value> ptr = utility::details::make_unique<details::_Array>(std::move(elements));
    return json::value(std::move(ptr)
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            ,value::Array
#endif
            );
}

json::number json::value::as_number() const
{
    return m_value->as_number();
}

double json::value::as_double() const
{
    return m_value->as_double();
}

int json::value::as_integer() const
{
    return m_value->as_integer();
}

bool json::value::as_bool() const
{
    return m_value->as_bool();
}

json::array& json::value::as_array()
{
    return m_value->as_array();
}

const json::array& json::value::as_array() const
{
    return m_value->as_array();
}

json::object& json::value::as_object()
{
    return m_value->as_object();
}

const json::object& json::value::as_object() const
{
    return m_value->as_object();
}

bool json::number::is_int32() const
{
#pragma push_macro ("max")
#pragma push_macro ("min")
#undef max
#undef min
    switch (m_type)
    {
    case signed_type : return m_intval >= std::numeric_limits<int32_t>::min() && m_intval <= std::numeric_limits<int32_t>::max();
    case unsigned_type : return m_uintval <= std::numeric_limits<int32_t>::max();
    case double_type :
    default :
        return false;
    }
#pragma pop_macro ("min")
#pragma pop_macro ("max")
}

bool json::number::is_uint32() const
{
#pragma push_macro ("max")
#undef max
    switch (m_type)
    {
    case signed_type : return m_intval >= 0 && m_intval <= std::numeric_limits<uint32_t>::max();
    case unsigned_type : return m_uintval <= std::numeric_limits<uint32_t>::max();
    case double_type :
    default :
        return false;
    }
#pragma pop_macro ("max")
}

bool json::number::is_int64() const
{
#pragma push_macro ("max")
#undef max
    switch (m_type)
    {
    case signed_type : return true;
    case unsigned_type : return m_uintval <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    case double_type :
    default :
        return false;
    }
#pragma pop_macro ("max")
}

bool json::details::_String::has_escape_chars(const _String &str)
{
    static const auto escapes = L"\"\\\b\f\r\n\t";
    return str.m_string.find_first_of(escapes) != std::wstring::npos;
}

json::details::_Object::_Object(const _Object& other) :
    json::details::_Value(other), m_object(other.m_object.m_elements, other.m_object.m_keep_order) {}

json::value::value_type json::value::type() const { return m_value->type(); }

bool json::value::is_integer() const
{
    if(!is_number())
    {
        return false;
    }
    return m_value->is_integer();
}

bool json::value::is_double() const
{
    if(!is_number())
    {
        return false;
    }
    return m_value->is_double();
}

json::value& json::details::_Object::index(const std::wstring &key)
{
    return m_object[key];
}

bool json::details::_Object::has_field(const std::wstring &key) const
{
    return m_object.find(key) != m_object.end();
}

std::wstring json::value::to_string() const
{
    return m_value->to_string();
}

bool json::value::operator==(const json::value &other) const
{
    if (this->m_value.get() == other.m_value.get())
        return true;
    if (this->type() != other.type())
        return false;

    switch(this->type())
    {
    case Null:
        return true;
    case Number:
        return this->as_number() == other.as_number();
    case Boolean:
        return this->as_bool() == other.as_bool();
    case String:
        return this->as_string() == other.as_string();
    case Object:
        return static_cast<const json::details::_Object*>(this->m_value.get())->is_equal(static_cast<const json::details::_Object*>(other.m_value.get()));
    case Array:
        return static_cast<const json::details::_Array*>(this->m_value.get())->is_equal(static_cast<const json::details::_Array*>(other.m_value.get()));
    }
    __assume(0);
}

// at() overloads
json::value& json::value::at(size_t index)
{
    return this->as_array().at(index);
}

const json::value& json::value::at(size_t index) const
{
    return this->as_array().at(index);
}

json::value& json::value::at(const std::wstring& key)
{
    return this->as_object().at(key);
}

const json::value& json::value::at(const std::wstring& key) const
{
    return this->as_object().at(key);
}

json::value& json::value::operator [] (const std::wstring &key)
{
    if ( this->is_null() )
    {
        m_value.reset(new json::details::_Object(details::g_keep_json_object_unsorted));
#ifdef ENABLE_JSON_VALUE_VISUALIZER
        m_kind = value::Object;
#endif
    }
    return m_value->index(key);
}

json::value& json::value::operator[](size_t index)
{
    if ( this->is_null() )
    {
        m_value.reset(new json::details::_Array());
#ifdef ENABLE_JSON_VALUE_VISUALIZER
        m_kind = value::Array;
#endif
    }
    return m_value->index(index);
}

const json::details::json_error_category_impl& json::details::json_error_category()
{
    static json::details::json_error_category_impl instance;
    return instance;
}
