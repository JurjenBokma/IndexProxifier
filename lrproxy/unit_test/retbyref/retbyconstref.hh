
#ifndef retbyconstref_hh_defd
#define retbyconstref_hh_defd

#include "../../../indexproxifier.hh"
#include "cstddef"

class RetByConstRef : protected IndexProxifier<RetByConstRef>
{

    typedef int data_t;
    data_t d_data[4] = {};

    typedef IndexProxifier<RetByConstRef> BaseT;
    
public:

    RetByConstRef() = default;
    RetByConstRef(RetByConstRef const &other) = delete;
    RetByConstRef(std::initializer_list<int> items);
    
    using BaseT::operator[];
    
private:

    data_t const &proxy_return_action(std::size_t ix);
    data_t const &proxy_accept_action(std::size_t ix, data_t &newvalue);

    friend BaseT;
    
};

inline RetByConstRef::RetByConstRef(std::initializer_list<int> items)
{
    std::copy(items.begin(), items.end(), d_data);
}


inline RetByConstRef::data_t const &RetByConstRef::proxy_return_action(std::size_t ix)
{
    return d_data[ix];
}

inline RetByConstRef::data_t const &RetByConstRef::proxy_accept_action(std::size_t ix, data_t &newvalue)
{
    return d_data[ix] = newvalue;
}


#endif //retbyref_hh_defd
