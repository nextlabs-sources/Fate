

#ifndef __NUDF_BITSMAP_HPP__
#define __NUDF_BITSMAP_HPP__

#include <vector>

namespace nudf
{
namespace util
{

template<unsigned long N>
class CBitsMap
{
public:
    CBitsMap() throw()
    {
        _map.resize((N+31)/32, 0);
    }

    virtual ~CBitsMap() throw()
    {
    }

    bool Get(_In_ unsigned long bit) const throw()
    {
        if(bit >= N) {
            return false;
        }
        return (0 != (_map[bit/32] & GetBitMask(bit%32)));
    }

    bool operator [](_In_ unsigned long bit) const throw()
    {
        if(bit >= N) {
            return false;
        }
        return (0 != (_map[bit/32] & GetBitMask(bit%32)));
    }

    void Set(_In_ unsigned long bit) const throw()
    {
        if(bit >= N) {
            return;
        }
        _map[bit/32] |= GetBitMask(bit%32);
    }

    void Clear(_In_ unsigned long bit) const throw()
    {
        if(bit >= N) {
            return;
        }
        _map[bit/32] &= (~GetBitMask(bit%32));
    }

    CBitsMap<N>& operator = (_In_ unsigned long data)
    {
        _map[0] = data;
        return *this;
    }

    CBitsMap<N>& operator = (_In_ int data)
    {
        _map[0] = (unsigned long)data;
        return *this;
    }

    unsigned long size() const throw() {return N;}

protected:
    __forceinline unsigned long GetBitMask(_In_ unsigned long bit) const throw()
    {
        switch(bit)
        {
        case 0: return 0x00000001;
        case 1: return 0x00000002;
        case 2: return 0x00000004;
        case 3: return 0x00000008;
        case 4: return 0x00000010;
        case 5: return 0x00000020;
        case 6: return 0x00000040;
        case 7: return 0x00000080;
        case 8: return 0x00000100;
        case 9: return 0x00000200;
        case 10: return 0x00000400;
        case 11: return 0x00000800;
        case 12: return 0x00001000;
        case 13: return 0x00002000;
        case 14: return 0x00004000;
        case 15: return 0x00008000;
        case 16: return 0x00010000;
        case 17: return 0x00020000;
        case 18: return 0x00040000;
        case 19: return 0x00080000;
        case 20: return 0x00100000;
        case 21: return 0x00200000;
        case 22: return 0x00400000;
        case 23: return 0x00800000;
        case 24: return 0x01000000;
        case 25: return 0x02000000;
        case 26: return 0x04000000;
        case 27: return 0x08000000;
        case 28: return 0x10000000;
        case 29: return 0x20000000;
        case 30: return 0x40000000;
        case 31: return 0x80000000;
        default: break;
        }
        return 0;
    }

private:
    std::vector<unsigned long> _map;
};

    
}   // namespace nudf::util
}   // namespace nudf


#endif  // #ifndef __NUDF_BITSMAP_HPP__