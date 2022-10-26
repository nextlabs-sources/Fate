

#ifndef __NUDF_LISTENTRY_HPP__
#define __NUDF_LISTENTRY_HPP__


#include <Windows.h>
#include <assert.h>
#include <unordered_map>


namespace nudf
{
namespace util
{


__forceinline
VOID InitializeListHead(_In_ PLIST_ENTRY ListHead)
{
	ListHead->Flink = ListHead->Blink = ListHead;
}

__forceinline
BOOL IsListEmpty(_In_ const LIST_ENTRY* ListHead)
{
	return (BOOLEAN)(ListHead->Flink == ListHead);
}

__forceinline
BOOL RemoveEntryList(_In_ PLIST_ENTRY Entry)
{
	PLIST_ENTRY Blink;
	PLIST_ENTRY Flink;

	Flink = Entry->Flink;
	Blink = Entry->Blink;
	Blink->Flink = Flink;
	Flink->Blink = Blink;
	return (Flink == Blink);
}

__forceinline
PLIST_ENTRY RemoveHeadList(_In_ PLIST_ENTRY ListHead)
{
	PLIST_ENTRY Flink;
	PLIST_ENTRY Entry;

    if(IsListEmpty(ListHead)) {
        return NULL;
    }

	Entry = ListHead->Flink;
	Flink = Entry->Flink;
	ListHead->Flink = Flink;
	Flink->Blink = ListHead;
	return Entry;
}

__forceinline
PLIST_ENTRY RemoveTailList(_In_ PLIST_ENTRY ListHead)
{
	PLIST_ENTRY Blink;
	PLIST_ENTRY Entry;

    if(IsListEmpty(ListHead)) {
        return NULL;
    }

	Entry = ListHead->Blink;
	Blink = Entry->Blink;
	ListHead->Blink = Blink;
	Blink->Flink = ListHead;
	return Entry;
}


__forceinline
VOID InsertTailList(_In_ PLIST_ENTRY ListHead, _In_ PLIST_ENTRY Entry)
{
	PLIST_ENTRY Blink;

	Blink = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = Blink;
	Blink->Flink = Entry;
	ListHead->Blink = Entry;
}

__forceinline
VOID InsertHeadList(_In_ PLIST_ENTRY ListHead, _In_ PLIST_ENTRY Entry)
{
	PLIST_ENTRY Flink;

	Flink = ListHead->Flink;
	Entry->Flink = Flink;
	Entry->Blink = ListHead;
	Flink->Blink = Entry;
	ListHead->Flink = Entry;
}

template<typename K, typename V>
struct HASHLISTITEM {
    LIST_ENTRY  link;
    K           key;
    V           value;
};

template <typename K, typename V, size_t N=-1>
class CHashList
{
public:
    CHashList() : _size(0)
    {
        InitializeListHead(&_head);
    }

    virtual ~CHashList()
    {
        Clear();
    }


    size_t Size() const throw() {return _size;}
    bool Full() const throw() {return (_size >= N);}
    bool Empty() const throw() {return IsListEmpty(&_head) ? true : false;}

    virtual bool InsertHead(const K& key, const V& value)
    {
        HASHLISTITEM<K,V>* item = NULL;

        if(N>0 && _size >= N) {
            return false;
        }

        std::unordered_map<K, HASHLISTITEM<K,V>*>::iterator pos = _map.find(key);
        if(pos != _map.end()) {
            // Remove existing
            item = (*pos).second;
            item->value = value;
            RemoveEntryList(&item->link);
            InsertHeadList(&_head, &item->link);
            return true;
        }

        item = new HASHLISTITEM<K,V>;
        if(NULL == item) {
            return false;
        }

        item->key = key;
        item->value = value;
        InsertHeadList(&_head, &item->link);
        _map[key] = item;
        ++_size;
        return true;
    }

    virtual bool InsertTail(const K& key, const V& value)
    {
        HASHLISTITEM<K,V>* item = NULL;

        if(N>0 && _size >= N) {
            return false;
        }

        std::unordered_map<K, HASHLISTITEM<K,V>*>::iterator pos = _map.find(key);
        if(pos != _map.end()) {
            // Remove existing
            item = (*pos).second;
            item->value = value;
            RemoveEntryList(&item->link);
            InsertTailList(&_head, &item->link);
            return true;
        }

        item = new HASHLISTITEM<K,V>;
        if(NULL == item) {
            return false;
        }

        item->key = key;
        item->value = value;
        InsertTailList(&_head, &item->link);
        _map[key] = item;
        ++_size;
        return true;
    }

    virtual bool RemoveHead(_Out_ K& key, _Out_ V& value)
    {
        HASHLISTITEM<K,V>* item = (HASHLISTITEM<K,V>*)RemoveHeadList(&_head);
        if(NULL != item) {
            RemoveHashEntry(item->key);
            key = item->key;
            value = item->value;
            delete item; item = NULL;
            --_size;
            return true;
        }

        return false;
    }

    virtual bool RemoveTail(_Out_ K& key, _Out_ V& value)
    {
        HASHLISTITEM<K,V>* item = (HASHLISTITEM<K,V>*)RemoveTailList(&_head);
        if(NULL != item) {
            RemoveHashEntry(item->key);
            key = item->key;
            value = item->value;
            delete item; item = NULL;
            --_size;
            return true;
        }

        return false;
    }

    virtual bool RemoveHead()
    {
        HASHLISTITEM<K,V>* item = (HASHLISTITEM<K,V>*)RemoveHeadList(&_head);
        if(NULL != item) {
            RemoveHashEntry(item->key);
            delete item; item = NULL;
            --_size;
            return true;
        }

        return false;
    }

    virtual bool RemoveTail()
    {
        HASHLISTITEM<K,V>* item = (HASHLISTITEM<K,V>*)RemoveTailList(&_head);
        if(NULL != item) {
            RemoveHashEntry(item->key);
            delete item; item = NULL;
            --_size;
            return true;
        }

        return false;
    }
    virtual bool RemoveItem(const K& key)
    {
        std::unordered_map<K, HASHLISTITEM<K,V>*>::iterator pos = _map.find(key);
        if(pos != _map.end()) {
            HASHLISTITEM<K,V>* item = (*pos).second;
            RemoveEntryList(&item->link);
            delete item;
            --_size;
            _map.erase(pos);
            return true;
        }

        return false;
    }

    virtual void Clear()
    {
        _map.clear();
        while(!Empty()) {
            HASHLISTITEM<K,V>* item = (HASHLISTITEM<K,V>*)RemoveHeadList(&_head);
            delete item;
        }
    }
    
protected:
    void RemoveHashEntry(const K& key)
    {
        std::unordered_map<K, HASHLISTITEM<K,V>*>::iterator pos = _map.find(key);
        if(pos != _map.end()) {
            _map.erase(pos);
        }
    }

protected:
    LIST_ENTRY  _head;
    size_t      _size;
    std::unordered_map<K, HASHLISTITEM<K,V>*> _map;
};



template <typename K, typename V, size_t N=-1>
class CLRUList : public CHashList<K,V,N>
{
public:
    CLRUList() : CHashList<K,V,N>()
    {
    }

    virtual ~CLRUList()
    {
    }

    bool Insert(_In_ const K& key, _In_ const V& value)
    {
        if(Full()) {
            RemoveTail();
        }
        return InsertHead(key, value);
    }

    void Remove(_In_ const K& key)
    {
        RemoveItem(key);
    }

    bool Find(_In_ const K& key, _Out_ V& value)
    {
        std::unordered_map<K, HASHLISTITEM<K,V>*>::iterator pos = _map.find(key);
        if(pos != _map.end()) {
            // Remove existing
            HASHLISTITEM<K,V>* item = (*pos).second;
            value = item->value;
            RemoveEntryList(&item->link);
            InsertHeadList(&_head, &item->link);
            return true;
        }
        return false;
    }
};
    

}   // namespace nudf::util
}   // namespace nudf


#endif  // #ifndef __NUDF_LISTENTRY_HPP__