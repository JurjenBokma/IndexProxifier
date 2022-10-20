
#ifndef retbyvalue_hh_defd
#define retbyvalue_hh_defd

#include "../../../indexproxifier.hh"
#include "cstddef"
#include <algorithm>
#include <initializer_list>

template <typename T>
struct RetByValue : protected IndexProxifier<RetByValue<T>>
{

    typedef T data_t;
    enum { Count = 4};
    data_t d_data[Count] = {};

    typedef IndexProxifier<RetByValue<T>> BaseT;
    
public:

    RetByValue() = default;
    RetByValue(RetByValue const &other) = delete;
    RetByValue(std::initializer_list<data_t> items);

    using BaseT::operator[];
    
private:

    data_t &proxy_return_action(std::size_t ix);
    data_t &proxy_accept_action(std::size_t ix, data_t &newvalue);

    friend BaseT;
    
};

template <typename T>
RetByValue<T>::RetByValue(typename std::initializer_list<RetByValue<T>::data_t> items)
{
    std::copy(items.begin(), items.end(), d_data);
}

template <typename T>
typename RetByValue<T>::data_t &RetByValue<T>::proxy_return_action(std::size_t ix)
{
    return d_data[ix];
}

template <typename T>
typename RetByValue<T>::data_t &RetByValue<T>::proxy_accept_action(std::size_t ix, typename RetByValue<T>::data_t &newvalue)
{
    return d_data[ix] = newvalue;
}


#endif //retbyvalue_hh_defd
