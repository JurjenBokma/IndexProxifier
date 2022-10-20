
#ifndef retbyref_hh_defd
#define retbyref_hh_defd

#include "../../../indexproxifier.hh"
#include "cstddef"
#include <algorithm>
#include <initializer_list>

struct RetByRef : protected IndexProxifier<RetByRef>
{

    typedef int data_t;
    enum { Count = 4};
    data_t d_data[Count] = {};

    typedef IndexProxifier<RetByRef> BaseT;
    
public:

    RetByRef() = default;
    RetByRef(RetByRef const &other) = delete;
    RetByRef(std::initializer_list<int> items);

    using BaseT::operator[];
    
private:

    data_t &proxy_return_action(std::size_t ix);
    data_t &proxy_accept_action(std::size_t ix, data_t &newvalue);

    friend BaseT;
    
};

inline RetByRef::RetByRef(std::initializer_list<int> items)
{
    std::copy(items.begin(), items.end(), d_data);
}

inline RetByRef::data_t &RetByRef::proxy_return_action(std::size_t ix)
{
    return d_data[ix];
}

inline RetByRef::data_t &RetByRef::proxy_accept_action(std::size_t ix, data_t &newvalue)
{
    return d_data[ix] = newvalue;
}


#endif //retbyref_hh_defd
