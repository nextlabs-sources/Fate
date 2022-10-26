// Following John Robbins' lead, lifted from ATL.

// This is a part of the Active Template Library.
// Copyright (C) 1996-2001 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef _CCOLLECTION_H
#define _CCOLLECTION_H

#pragma once

#pragma push_macro("malloc")
#undef malloc
#pragma push_macro("realloc")
#undef realloc
#pragma push_macro("free")
#undef free

#pragma push_macro("new")
#undef new

template <class T>
class CStructureEqualHelper
{
  public:
    static bool IsEqual(const T& t1, const T& t2)
    {
      return memcmp(&t1, &t2, sizeof(T)) == 0;
    }
};

// template class helpers with functions for comparing elements
// override if using complex types without operator==
template <class T>
class CCollectionEqualHelper
{
  public:
    static bool IsEqual(const T& t1, const T& t2)
    {
      return t1 == t2;
    }
};

template <class T>
class CCollectionEqualHelperFalse
{
  public:
    static bool IsEqual(const T&, const T&)
    {
      ASSERT(false);
      return false;
    }
};

template <class TKey, class TVal>
class CCollectionMapEqualHelper
{
  public:
    static bool IsEqualKey(const TKey& k1, const TKey& k2)
    {
      return CCollectionEqualHelper<TKey>::IsEqual(k1, k2);
    }

    static bool IsEqualValue(const TVal& v1, const TVal& v2)
    {
      return CCollectionEqualHelper<TVal>::IsEqual(v1, v2);
    }
};

template <class TKey, class TVal>
class CCollectionMapEqualHelperFalse
{
  public:
    static bool IsEqualKey(const TKey& k1, const TKey& k2)
    {
      return CCollectionEqualHelper<TKey>::IsEqual(k1, k2);
    }

    static bool IsEqualValue(const TVal&, const TVal&)
    {
      ASSERT(FALSE);
      return false;
    }
};

template <class T, class TEqual = CCollectionEqualHelper< T > >
class SYSTEMS_API CCollection
{
  public:

    // Construction/destruction

    CCollection() : mpArray(NULL), mCount(0), mSize(0), mSizeFixed(FALSE)
    { }

    CCollection(int fixedRecordCount) : mpArray(NULL), mCount(0), mSize(fixedRecordCount), mSizeFixed(TRUE)
    {
      mpArray = (T *) malloc(mSize * sizeof(T));
    }
      
    ~CCollection()
    {
      RemoveAll();
    }

    CCollection(const CCollection<T, TEqual>& src):  mpArray(NULL), mCount(0), mSize(0), mSizeFixed(FALSE)
    {
      mpArray = (T*) malloc(src.GetCount() * sizeof(T));
      if (mpArray != NULL)
      {
        mSize = src.GetCount();
        for (int i = 0; i < src.GetCount(); i++)
          Add(src[i]);
      }
    }

    CCollection<T, TEqual>& operator = (const CCollection<T, TEqual>& src)
    {
      if (GetCount() != src.GetCount())
      {
        RemoveAll();
        mpArray = (T*) malloc(src.GetCount() * sizeof(T));
        if (mpArray != NULL)
          mSize = src.GetCount();
      }
      else
      {
        for (int i = GetCount(); i > 0; i--)
          RemoveAt(i - 1);
      }

      for (int i = 0; i < src.GetCount(); i++)
        Add(src[i]);

      return *this;
    }


    // Operations

    int GetCount() const
    {
      return mCount;
    }

    int GetSize() const
    {
      return mSize * sizeof(T);
    }

    BOOL Add(const T& t)
    {
      if (mCount == mSize)
      {
        if (mSizeFixed)
          return FALSE;

        T* pT;
        int newSize = (mSize == 0) ? 1 : (mSize * 2);
        pT = (T*) realloc(mpArray, newSize * sizeof(T));
        if (pT == NULL)
          return FALSE;
        mSize = newSize;
        mpArray = pT;
      }
      mCount++;
      InternalSetAtIndex(mCount - 1, t);
      return TRUE;
    }

    BOOL Remove(const T& t)
    {
      int index = Find(t);
      if (index == -1)
        return FALSE;
      return RemoveAt(index);
    }

    BOOL RemoveAt(int index)
    {
      ASSERT(index >= 0 && index < mCount);

      if (index < 0 || index >= mCount)
        return FALSE;

      mpArray[index].~T();

      if (index != (mCount - 1))
        memmove((void*) (mpArray + index), (void*) (mpArray + index + 1), (mCount- (index + 1)) * sizeof(T));
      mCount--;
      return TRUE;
    }

    void RemoveAll()
    {
      if (mpArray != NULL)
      {
        for (int i = 0; i < mCount; i++)
          mpArray[i].~T();
        free(mpArray);
        mpArray = NULL;
      }
      mCount = 0;
      mSize = 0;
    }

    const T& operator[] (int index) const
    {
      ASSERT(index >= 0 && index < mCount);

      return mpArray[index];
    }

    T& operator[] (int index)
    {
      ASSERT(index >= 0 && index < mCount);

      return mpArray[index];
    }

    T* GetData() const
    {
      return mpArray;
    }

    int Find(const T& t) const
    {
      for (int i = 0; i < mCount; i++)
      {
        if (TEqual::IsEqual(mpArray[i], t))
          return i;
      }
      return -1;  // not found
    }

    BOOL SetAtIndex(int index, const T& t)
    {
      if (index < 0 || index >= mCount)
        return FALSE;

      InternalSetAtIndex(index, t);
      return TRUE;
    }

    // Wrapper simply holds an instance t of type T at location p
    class Wrapper
    {
      public:
        Wrapper(const T& _t) : t(_t)
        { }

        template <class _Ty>
        void * __cdecl operator new(size_t, _Ty* p)
        {
          return p;
        }
        template <class _Ty>
        void __cdecl operator delete(void*, _Ty*)
        { }

        T t;
    };

    // Implementation

    void InternalSetAtIndex(int index, const T& t)
    {
      ASSERT(index >= 0 && index < mCount);
      // Creating at existing location
      new(mpArray + index) Wrapper(t);
    }

    typedef T _ArrayElementType;
    T* mpArray;
    int mCount;
    int mSize;
    bool mSizeFixed;

};

#pragma pop_macro("new")

#pragma pop_macro("free")
#pragma pop_macro("realloc")
#pragma pop_macro("malloc")


#endif  // _JSIMPLECOLL_H
