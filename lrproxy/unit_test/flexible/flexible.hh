#ifndef flexible_hh_defd
#define flexible_hh_defd

#include "../../../indexproxifier.hh"
#include "../../../../unittest/unittest.hh"


template <typename DataT,
          typename KeyT,
          typename ReturnT,
          typename AcceptT,
          typename ValueT,
          typename cKeyT = KeyT,
          typename cReturnT = ReturnT,
          typename cAcceptT = AcceptT,
          typename cValueT = ValueT,
          typename rrKeyT = KeyT,
          typename rrReturnT = ReturnT,
          typename rrAcceptT = AcceptT,
          typename rrValueT = ValueT
         >
struct Flexible: protected IndexProxifier<Flexible<DataT, KeyT,   ReturnT,   AcceptT,   ValueT,
                                                         cKeyT,  cReturnT,  cAcceptT,  cValueT,
                                                        rrKeyT, rrReturnT, rrAcceptT, rrValueT
                                                   >
                                          >
{
    typedef IndexProxifier<Flexible> BaseT;
    friend BaseT;
    
    DataT d_data[2] = {};
    
public:
    
    using BaseT::operator[];
    
private:

    ReturnT proxy_return_action(KeyT key) &
    {
        return d_data[key];
    }
    
    cReturnT proxy_return_action(cKeyT key) const &
    {
        return d_data[key];
    }
    
    rrReturnT proxy_return_action(rrKeyT key) &&
    {
        return d_data[key];
    }
    
    AcceptT proxy_accept_action(KeyT key, ValueT value) &
    {
        d_data[key] = value;
        return d_data[key];
    }
    
    cAcceptT proxy_accept_action(cKeyT key, cValueT value) const &
    {
        d_data[key] = value;
        return d_data[key];
    }
    
    rrAcceptT proxy_accept_action(rrKeyT key, rrValueT value) &&
    {
        d_data[key] = value;
        return d_data[key];
    }
    
};

#endif //flexible_hh_defd
