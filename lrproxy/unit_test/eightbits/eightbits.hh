#ifndef eightbits_hh_defd
#define eightbits_hh_defd

#include "../../../indexproxifier.hh"

typedef uint8_t data_t;

class EightBits: protected IndexProxifier<EightBits>
{

    data_t d_data = 0;
    
public:
    EightBits() = default;
    EightBits(data_t initial);
    EightBits(EightBits const &other) = default;
    int internal() const;
    std::size_t internal_address() const;

    EightBits &operator=(data_t value);
    
    using IndexProxifier<EightBits>::operator[];
    
private:

    friend IndexProxifier<EightBits>;
    
    bool proxy_return_action(int key) const;
    bool proxy_accept_action(int key, bool value);

    static constexpr data_t bitmask(short unsigned int index);

};


inline EightBits::EightBits(data_t initial)
    : d_data(initial)
{}

inline int EightBits::internal() const
{
    return static_cast<int>(d_data);
}

inline std::size_t EightBits::internal_address() const
{
    return reinterpret_cast<std::size_t>(const_cast<void *>(static_cast<void const *>(&d_data)));
}

inline EightBits &EightBits::operator=(data_t value)
{
    d_data = value;
    return *this;
}

inline constexpr data_t EightBits::bitmask(short unsigned int index)
{
    return static_cast<data_t>(1) << index;
}


inline bool EightBits::proxy_return_action(int key) const
{
    bool retval = (d_data & bitmask(key)) != 0;
    return retval;
}

inline bool EightBits::proxy_accept_action(int key, bool value)
{
    data_t mask = bitmask(key);
    if (value)
        d_data |= mask;
    else
        d_data &= ~mask;
    return value;
}


#endif //eightbits_hh_defd
