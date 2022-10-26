

#ifndef __NUDF_ARGS_HPP__
#define __NUDF_ARGS_HPP__


namespace nudf
{
namespace util
{


template<typename T>
class CArguments
{
public:
    CArguments() : m_bValid(false)
    {
    }

    virtual ~CArguments()
    {
    }

    inline bool IsValid() const throw() {return m_bValid;}

    virtual void SetArguments(int argc, T** argv) = 0;

private:
    bool m_bValid;
};


}   // namespace nudf::util
}   // namespace nudf



#endif  // #ifndef __NUDF_ARGS_HPP__