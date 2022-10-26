

#ifndef __NUDF_RWLOCK_HPP__
#define __NUDF_RWLOCK_HPP__


namespace nudf
{
namespace util
{

class CRwLock
{
public:
    CRwLock() throw();
    virtual ~CRwLock() throw();

    inline PSRWLOCK GetLock() {return &m_srwLock;}

private:
    SRWLOCK m_srwLock;
};

class CRwExclusiveLocker
{
public:
    CRwExclusiveLocker(_In_ CRwLock* lock) throw();
    ~CRwExclusiveLocker() throw();

private:
    CRwLock* m_pLock;
};

class CRwSharedLocker
{
public:
    CRwSharedLocker(_In_ CRwLock* lock) throw();
    ~CRwSharedLocker() throw();

private:
    CRwLock* m_pLock;
};
    
}   // namespace nudf::util
}   // namespace nudf


#endif  // #ifndef __NUDF_RWLOCK_HPP__