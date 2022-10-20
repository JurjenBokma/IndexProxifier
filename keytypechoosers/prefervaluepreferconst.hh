#ifndef prefervaluepreferconst_hh_defd
#define prefervaluepreferconst_hh_defd

#ifndef def_h_include_indexproxifier_hh
#error "Don't include prefervaluepreferconst.hh. Include indexproxifier.hh instead."
#endif

#include <experimental/type_traits>
#include <type_traits>

// Helper to detect if Derived has a proxy_return_action that accepts Key.
template <typename Derived, typename Key>
using proxy_return_action_can_handle_key = decltype(std::declval<Derived>().proxy_return_action(std::declval<Key>()));

/**
   Could be used as keytype choice for IndexProxifier.
   A key which' value is kept in the LRProxy is safer than a reference, because
   the reference may dangle.

   This template prefers a plain value for built-in types and small objects, but
   const reference for larger objects, unless the OwnerT cannot handle the
   preferred type. In the latter case, it will simply use KeyT.
*/
template <typename KeyT, typename OwnerT>
class PreferValuePreferConst
{
    typedef typename std::remove_reference<KeyT>::type nonref_t;
    
    enum : bool
    {
        IsRValueReference = std::is_rvalue_reference<KeyT>::value,
        IsBuiltin = not std::is_class<nonref_t>::value
                 && not std::is_union<nonref_t>::value,
        // Copying a small object is as efficient as dereferencing a reference.
        IsSmallObject = sizeof(nonref_t) <= 2 * sizeof(void *),
        IsArray = std::is_array<KeyT>::value,
    };

    static_assert(
        not IsArray,
        "Cannot handle arrays as IndexProxifier keys, because assigment doesn't "
        "work on them. Use a different KeyTypeChooser policy."
        );
    
    typedef
    typename std::conditional
    <
        IsBuiltin || IsRValueReference || IsSmallObject,
        nonref_t,           // Prefer value for built-in and rvalue references,
        nonref_t const &    // but const ref for objects.
    >::type preferred_t;

    enum : bool
    {
        CanHandlePreferred =
        std::experimental::is_detected
        <
            proxy_return_action_can_handle_key,
            OwnerT,
            preferred_t
        >::value,
    };

public:

    typedef typename std::conditional
    <
        CanHandlePreferred,
        preferred_t, // If OwnerT can handle the preferred type, use it,
        KeyT         // else revert to Original KeyT and let the chips fall where they may.
    >::type type;
};


#endif //prefervaluepreferconst_hh_defd
