

#ifndef _NUDF_LRU_CACHE_HPP__
#define _NUDF_LRU_CACHE_HPP__


#include <Windows.h>
#include <assert.h>
#include <unordered_map>

namespace nudf {
namespace util {

template<typename K, typename V>
class CLRUCache
{
public:
    CLRUCache()
    {
        InitializeCriticalSection(&_lock);
    }
    CLRUCache(unsigned long maxsize)
    {
        InitializeCriticalSection(&_lock);
        _list.set_maxsize(maxsize);
    }

    virtual ~CLRUCache()
    {
        DeleteCriticalSection(&_lock);
    }

    V Find(const K& key) throw()
    {
        V value;
        ::EnterCriticalSection(&_lock);
        std::unordered_map<K, Node*>::iterator it = _map.find(key);
        if(it != _map.end()) {
            // found!
            Node* node = it->second;
            value = node->_value;
            // move this it to list header if it is not
            if(!_list.IsHead(node)) {
                _list.RemoveNode(node);
                _list.InsertHead(node);
            }
        }
        ::LeaveCriticalSection(&_lock);
        return value;
    }

    V TopItem() throw()
    {
        V value;
        Node* node = NULL;
        ::EnterCriticalSection(&_lock);
        node = _list.GetHead();
        if(NULL != node) {
            value = node->_value;
        }
        ::LeaveCriticalSection(&_lock);
        return value;
    }

    unsigned int Size() const throw()
    {
        return _list.GetSize();
    }

    unsigned int MaxSize() const throw()
    {
        return _list.GetMaxSize();
    }

    void Insert(const K& key, const V& value)
    {
        Node* node = NULL;
        ::EnterCriticalSection(&_lock);
        std::unordered_map<K, Node*>::iterator it = _map.find(key);
        if(it != _map.end()) {
            // already exist!
            node = it->second;
            node->_key = key;
            node->_value = value;
            // move this it to list header if it is not
            if(!_list.IsHead(node)) {
                _list.RemoveNode(node);
                _list.InsertHead(node);
            }
        }
        else  {
            node = new Node();
            if(NULL == node) {
                throw std::bad_alloc();
            }
            if(_list.full()) {
                Remove(_list.GetTail()->_key);  // Remove tail
            }
            _list.InsertHead(node);
            node->_key = key;
            node->_value = value;
            _map.insert(std::pair<K, Node*>(key, node));
        }
        ::LeaveCriticalSection(&_lock);
    }

    void Remove(const K& key) throw()
    {
        Node* node = NULL;
        ::EnterCriticalSection(&_lock);
        std::unordered_map<K, Node*>::iterator it = _map.find(key);
        if(it != _map.end()) {
            // already exist!
            node = it->second;
            _list.RemoveNode(node);
            delete node;
            node = NULL;
            _map.erase(it);
        }
        ::LeaveCriticalSection(&_lock);
    }

    void Clear()
    {
        ::EnterCriticalSection(&_lock);
        _map.clear();
        _list.clear();
        ::LeaveCriticalSection(&_lock);
    }

    void Dump(std::vector<V>& v)
    {
        ::EnterCriticalSection(&_lock);
        _list.dump(v);
        ::LeaveCriticalSection(&_lock);
    }

private:
    class Node
    {
    public:
        Node() : _prev(NULL), _next(NULL)
        {
        }
        virtual ~Node()
        {
        }

        virtual void Detach() throw()
        {
            if(NULL == _prev || NULL == _next) {
                return;
            }

            _prev->_next = _next;
            _next->_prev = _prev;
            _prev = NULL;
            _next = NULL;
        }

        virtual BOOL Linked() const throw()
        {
            return (NULL != _prev && NULL != _next);
        }

        Node* _prev;
        Node* _next;
        K     _key;
        V     _value;
    };

    class List : public Node
    {
    public:
        List() : _size(0), _maxsize(128)
        {
            _next = this;
            _prev = this;
        }
        List(unsigned long maxsize) : _size(0), _maxsize(maxsize)
        {
            _next = this;
            _prev = this;
        }
        ~List()
        {
        }

        bool empty() const throw()
        {
            return (_next == this && _prev == this);
        }

        bool full() const throw()
        {
            return (_size == _maxsize);
        }

        unsigned long get_maxsize() const throw() {return _maxsize;}
        void set_maxsize(unsigned long size) throw() {_maxsize = size;}

        bool IsHead(Node* node)
        {
            return (_next!=this && _next==node);
        }
        bool IsTail(Node* node)
        {
            return (_prev!=this && _prev==node);
        }

        void RemoveNode(Node* node)
        {
            node->Detach();
            _size--;
        }

        void InsertHead(Node* node)
        {
            node->_next = _next;
            node->_prev = (Node*)this;
            _next->_prev = node;
            _next = node;
            _size++;
        }

        Node* RemoveHead()
        {
            Node* node = NULL;
            if(_next != this) {
                node = _next;
                node->Detach();
            }
            else {
                assert(0 == _size);
            }
            return node;
        }

        void InsertTail(Node* node)
        {
            node->_prev = _prev;
            _prev->_next = node;
            node->_next = (Node*)this;
            _prev = node;
            _size++;
        }

        Node* RemoveTail()
        {
            Node* node = NULL;
            if(_prev != this) {
                node = _prev;
                node->Detach();
                _size--;
            }
            else {
                assert(0 == _size);
            }
            return node;
        }

        virtual void clear()
        {
            Node* node = NULL;
            while(NULL != (node = RemoveHead())) {
                delete node;
            }
            assert(0 == _size);
        }
        

        void dump(std::vector<V>& v)
        {
            v.clear();
            Node* node = _next;
            while(node != this) {
                v.push_back(node->_value);
                node = node->_next;
            }
        }

        unsigned int GetSize() const throw()
        {
            return _size;
        }

        unsigned int GetMaxSize() const throw()
        {
            return _maxsize;
        }
        

        Node* GetHead()
        {
            if(empty()) {
                return NULL;
            }
            return _next;
        }

        Node* GetTail()
        {
            if(empty()) {
                return NULL;
            }
            return _prev;
        }

    private:
        unsigned long _size;
        unsigned long _maxsize;
    };

private:
    List                _list;
    CRITICAL_SECTION    _lock;
    std::unordered_map<K, Node*> _map;
};


}   // namespace nudf::util
}   // namespace nudf

#endif  // _NUDF_LRU_CACHE_HPP__