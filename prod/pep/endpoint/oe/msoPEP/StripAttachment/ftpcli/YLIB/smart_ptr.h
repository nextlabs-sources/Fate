#pragma once
#ifndef _YLIB_SMART_PTR_H_
#define _YLIB_SMART_PTR_H_
#include <assert.h>
#include <basetsd.h>

namespace YLIB
{
    namespace COMMON
    {
        class ref_count
        {
            //- Constructor and destructor
        protected:
            ref_count(void) : _nRefCount(0){};
        public:
            virtual ~ref_count(void) {};

            //- Reference count interface
        public:
            inline void addRef(void) 
            {
                InterlockedIncrement(&_nRefCount);
            }

            inline void release(void)
            {
                _ASSERT(_nRefCount>0);
                if(InterlockedDecrement(&_nRefCount)==0) delete this;
            };
        protected:
            LONG volatile _nRefCount;
        };

        template<class _T>
        class ref_counted : public ref_count
        {
        public:
            ref_counted(_T * p) : _p(p) { return; }
            virtual ~ref_counted(void)
            { 
                if (_p)
                {
                    delete _p;
                    _p = NULL;
                }
                return;
            }

            inline _T * ptr(void) const { return _p; }

            //- Function defined but WON'T be implemented
            ref_counted(const ref_counted<_T> & that);
            ref_counted<_T> & operator=(const ref_counted<_T> & that);

        protected:
            _T * _p;
        };

        template<class _T>
        class smart_ptr
        {
        public:
            explicit smart_ptr(_T * p = NULL)
                : _pref(new ref_counted<_T>(p))
            { 
                if (_pref)
                    _pref->addRef();
                return; 
            };

            smart_ptr(const smart_ptr<_T> & lp)
                : _pref(lp._pref)
            {
                if (_pref)
                    _pref->addRef();
                return;
            }

            ~smart_ptr(void)
            {
                if (_pref)
                    _pref->release();
                return;
            }

            inline _T & operator *() const 
            {
                _ASSERT(_pref); 
                _ASSERT(_pref->ptr());
                return *(_pref->ptr());
            }

            inline _T * operator ->() const 
            { 
                return operator _T * (); 
            }

            inline bool operator !() 
            { 
                return operator _T * () ? false:true;
            }

            inline bool operator <(_T * pT) const
            { 
                _ASSERT(_pref); 
                return _pref->ptr() < pT; 
            }

            inline bool operator ==(_T * pT) const
            { 
                _ASSERT(_pref); 
                return _pref->ptr() == pT;
            }
        private:
            // Release the interface and set to NULL
            inline void release(void)
            {
                ref_counted<_T> * pTemp = _pref;
                if (pTemp)
                {
                    _pref = NULL; // It's important to set _pref to NULL before invoke release().
                    pTemp->release();
                }
                return;
            }

            // Attach to an existing interface (does not AddRef)
            inline void attach(ref_counted<_T> * pref)
            {
                release();
                _pref = pref;
                return;
            }

            // Detach the interface (does not Release)
            inline ref_counted<_T> * detach(void)
            {
                ref_counted<_T> * pref = _pref;
                _pref = NULL;
                return pref;
            }

            inline bool CopyTo(ref_counted<_T> ** ppref)
            {
                _ASSERT(ppref);
                if (!ppref)
                    return false;
                *ppref = _pref;
                if (_pref)
                    _pref->addRef();
                return true;
            }
        public:
            inline smart_ptr<_T> & operator= (const smart_ptr<_T> & that)
            {
                // do addRef first in case that == *this or that._pref == _pref.
                if (that._pref)
                    that._pref->addRef();
                attach(that._pref);
                return *this;
            }

            inline _T * get() const
            { 
                _ASSERT(_pref);
                if(!_pref) return NULL;
                return _pref->ptr(); 
            }
            inline bool valid() const
            {
                _ASSERT(_pref);
                if(!_pref)
                    return false;
                //_ASSERT(_pref->ptr() != (_T*)(0xfeeefeee));
                return _pref->ptr()!=0; 
            }
        private:
            inline operator _T * () const // private it to avoid SystemApi(smart_ptr);
            { 
                _ASSERT(_pref); 
                if(!_pref)
                    return NULL;
                return _pref->ptr(); 
            }
        protected:
            ref_counted<_T> * _pref;
        };
    }
}

#endif