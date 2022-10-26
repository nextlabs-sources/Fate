

#ifndef __NUDF_EXCEPTION_H__
#define __NUDF_EXCEPTION_H__

#include <exception>

namespace nudf
{


class CException : public std::exception
{
public:
    CException() throw() : std::exception(), file_(NULL), func_(NULL), line_(-1), code_(0L)
    {
    }

    explicit CException(_In_opt_ const char* file,
                        _In_opt_ const char* func,
                        _In_ long line,
                        _In_ long code,
                        _In_opt_ const char* what
                        ) throw() : std::exception(what), file_(file), func_(func), line_(line), code_(code)
    {
    }

    virtual ~CException() throw()
    {
    }

    inline const char* GetFile() const throw() {return file_;}
    inline const char* GetFunction() const throw() {return func_;}
    inline long GetLine() const throw() { return line_; }
    inline long GetCode() const throw() {return code_;}

    CException& operator = (const nudf::CException& e) throw()
    {
        if ((&e) == this) {return *this;}   // avoid self-assign

        ((std::exception&)(*this)) = ((const std::exception&)e);
        file_ = e.GetFile();
        func_ = e.GetFunction();
        line_ = e.GetLine();
        code_ = e.GetCode();
        return *this;
    }

protected:
    mutable char const* file_;  // the pointer always points to __FILE__ data, don't need to free
    mutable char const* func_;  // the pointer always points to __FUNCTION__ data, don't need to free
    mutable long        line_;  // comes from __LINE__
    mutable long        code_;
};


#define EXCEPTION(code, what)   nudf::CException(__FILE__, __FUNCTION__, __LINE__, code, what)
#define WIN32ERROR()            nudf::CException(__FILE__, __FUNCTION__, __LINE__, GetLastError(), NULL)
#define WIN32ERROR2(code)       nudf::CException(__FILE__, __FUNCTION__, __LINE__, code, NULL)
#define WIN32ERROREX(what)      nudf::CException(__FILE__, __FUNCTION__, __LINE__, GetLastError(), what)


#define try_return(S)   {S; goto try_exit;}
#define try_return2(S)  {S; goto try_exit2;}


}   // namespace nudf


#endif  // #ifndef __NUDF_EXCEPTION_H__