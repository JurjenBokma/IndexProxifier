#ifndef byvalue_hh_defd
#define byvalue_hh_defd

#ifndef def_h_include_indexproxifier_hh
#error "Don't include byvalue.hh. Include indexproxifier.hh instead."
#endif

#include <experimental/type_traits>
#include <type_traits>

// Could be used as keytype choice policy for IndexProxifier.
template <typename KeyT, typename OwnerT>
struct ByValue
{
    // Always returns a value type.
    typedef std::remove_reference<KeyT> const type;
};

//FixMe: unit-test.

#endif //byvalue_hh_defd
