
#ifndef def_h_include_lrproxy_hh
#define def_h_include_lrproxy_hh

#ifndef def_h_include_indexproxifier_hh
#error "Don't include lrproxy.hh. Include indexproxifier.hh instead."
#endif

#include "../indexproxifier.hh" // A no-op except for the IDE.
#include "../../debug/debug.hh"
#include "../../ostreamable/ostreamable.hh"
#include "../../istreamable/istreamable.hh"
#include "../../proper_forward/proper_forward.hh"
#include <iostream>
#include <utility>
#include <type_traits>


#define template_IndexProxifier_LRProxy_boilerplate \
    template <typename Derived, template <typename, typename> typename KeyTypeChooser> \
    template <typename K, typename Owner>

template <typename Proxy>
concept IsLRProxy = requires(Proxy proxy)
{
    Proxy::Owner_T;
    Proxy::indexproxifier_conversion_type;
    std::move(proxy).operator typename Proxy::indexproxifier_conversion_type();
    std::cout << proxy;
    std::cin >> proxy;
};

/**
   This nested class LRProxy is the actual proxy, introduced by the IndexProxifier.
   It has
   - an operator return_type() to convert to whatever Derived::proxy_return_action returns,
   - an operator=() template to convert from whatever is offered.

   It should be hard to create a standalone LRProxy. But user can still do it with 'auto'.

   FixMe:
   operator= should pass on values by value, not by rvalue reference.
   But it should pass on lvalue references by lvalue reference.
   Regardless of that, it should not return conversion results if those cannot be chained.

   FixMe:
   Test: What if there are multiple proxy_return_action()s? 

   FixMe:
   overload operators like <=>, +, *=, -> etc.

*/
template_IndexProxifier_LRProxy_boilerplate
class IndexProxifier<Derived, KeyTypeChooser>::
LRProxy:
    public Ostreamable<IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>>,
    public Istreamable<IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>>
{

    Owner d_owner; // Owner is a (cv) (rvalue) reference, but not a value.
    K d_key; // K may be value or (cv) (rvalue) reference.
    // NB: Reference members _don't_ extend the lifetime of the referred-to object.

    // Constructors all private. User must not instantiate stdalone LRProxy. But 'auto' :-/
    explicit constexpr LRProxy(Owner &&owner, K &&key); // Don't overload!
                                                        // &&owner is a forwarding reference.
                                                        // Explicit to prevent use from 'auto'.

    // Functions must not return an LRProxy.
    constexpr LRProxy(LRProxy const &other) = delete;
    constexpr LRProxy(LRProxy &&tmp) = delete;

public: // types

    typedef Owner Owner_T; // Solely for debug/test.

    // constexpr operator auto() const; // Not all compilers accept this. Hence next line.
    typedef typename proper_forward<decltype(std::forward<Owner>(d_owner).proxy_return_action(d_key))>::type indexproxifier_conversion_type;

public: //member functions

    // All public members are rvalue-ref-qualified to discourage named proxies.
    // Although decltype(auto), they don't return rvalue references.
    
    constexpr operator indexproxifier_conversion_type() &&; // Anonymous temporary objects can also be converted.

    // Using convert_or_pass_on, one assignment template handles all cases.
    template <typename T>
    constexpr decltype(auto) operator=(T &&whatever) &&;
    
    template <typename T>
    constexpr decltype(auto) operator+=(T &&whatever) &&;
    
private:

    //Internally, we don't care about rvalue-ref-qualifiers.
    
    // Easier to call than operator indexproxifier_conversion_type;
    constexpr indexproxifier_conversion_type indexproxifier_conversion_value() const;
    
    template <typename T>
    constexpr decltype(auto) run_accept_action(T &&value) const;
    constexpr void run_accept_action(void) const; // In case of empty return type.

    template <typename T>
    static constexpr decltype(auto) convert_or_pass_on(T &&arg);

    constexpr std::ostream &write(std::ostream &os) const; // Forced const by operator<<.
    constexpr std::istream &read(std::istream &os) &&;

    friend class LRProxy_unittest; // For testing purposes.
    friend class IndexProxifier_unittest; // For testing.
    friend IndexProxifier<Derived, KeyTypeChooser>; // Could be tighter.
    friend std::ostream &operator<< <>(std::ostream &, Ostreamable<IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>> const &);
    friend std::istream &operator>> <>(std::istream &, Istreamable<IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>> &&);
    
};


template_IndexProxifier_LRProxy_boilerplate
constexpr IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::LRProxy(Owner &&owner, K &&key)
    : d_owner(std::forward<Owner>(owner)),
      d_key(std::forward<K>(key))
{
    // Assert here, not in IndexProxifier constructor.
    static_assert(
        std::is_base_of<IndexProxifier<Derived, KeyTypeChooser>, Derived>::value,
        "Trying to construct LRProxy, but IndexProxifier is not a base of Derived."
        );
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Constructing IndexProxifier from rvalue reference to Owner.\n";
        });
}

// operator indexproxifier_conversion_type
template_IndexProxifier_LRProxy_boilerplate
constexpr IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::operator indexproxifier_conversion_type() &&
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Conversion of rvalue-reference-to-LRProxy to indexproxifier_conversion_type.\n";
        });
    return indexproxifier_conversion_value();
}

template_IndexProxifier_LRProxy_boilerplate
constexpr typename IndexProxifier<Derived, KeyTypeChooser>::template LRProxy<K, Owner>::indexproxifier_conversion_type
IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::indexproxifier_conversion_value() const
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Returning value of rvalue-reference-to-LRProxy.\n";
        });
    return std::forward<Owner>(d_owner).proxy_return_action(d_key);
}

template_IndexProxifier_LRProxy_boilerplate
template <typename T>
constexpr decltype(auto)
IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::run_accept_action(T &&value) const
{
    return std::forward<Owner>(d_owner).proxy_accept_action(d_key, std::forward<T>(value));
}

template_IndexProxifier_LRProxy_boilerplate
constexpr void
IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::run_accept_action(void) const
{
}

template_IndexProxifier_LRProxy_boilerplate
template <typename T>
constexpr decltype(auto)
IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::convert_or_pass_on(T &&arg)
{
    if constexpr (IsLRProxy<typename std::remove_reference<T>::type>)
                     std::forward<T>(arg).indexproxifier_conversion_value();
    else
        return std::forward<T>(arg);
    // FixMe?: Passing rvalue references through this function ensures that
    // their lifetime is never extended. It may be better to pass those by value,
    // and relying on copy elision and/or return value optimization.
}

template_IndexProxifier_LRProxy_boilerplate
template <typename T>
constexpr decltype(auto)
IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::operator=(T &&whatever) &&
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Assignment to rvalue LRProxy.\n";
        });
    if constexpr (std::is_same<void, decltype(run_accept_action(convert_or_pass_on(std::forward<T>(whatever))))>::value)
                     return;
    else
        return forward_properly(run_accept_action(convert_or_pass_on(std::forward<T>(whatever))));
}


template_IndexProxifier_LRProxy_boilerplate
constexpr std::ostream &IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::write(std::ostream &os) const
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Writing LRProxy to ostream.\n";
        });
    return os << indexproxifier_conversion_value();
}

template_IndexProxifier_LRProxy_boilerplate
constexpr std::istream &IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::read(std::istream &is) &&
{
    static_assert(
        not std::is_const<typename std::remove_reference<indexproxifier_conversion_type>::type>::value,
        "Cannot read a new value into a const type or reference."
        );
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Reading LRProxy from istream.\n";
        });
    if constexpr (std::is_default_constructible<indexproxifier_conversion_type>::value)
                 {
                     indexproxifier_conversion_type newvalue;
                     if (is >> newvalue)
                         run_accept_action(newvalue);
                     // Read may fail. Stream will have flag then, and no changes were made.
                 }
    else 
    {
        // Awkward double handling of return value in case of reference. 
        decltype(auto) value_or_ref = indexproxifier_conversion_value();
        if (is >> value_or_ref)
            run_accept_action(value_or_ref);
    }
    return is;
}

template_IndexProxifier_LRProxy_boilerplate
template <typename T>
constexpr decltype(auto)
IndexProxifier<Derived, KeyTypeChooser>::LRProxy<K, Owner>::operator+=(T &&whatever) &&
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Operator += on rvalue LRProxy.\n";
        });
    return forward_properly(run_accept_action(indexproxifier_conversion_value() + convert_or_pass_on(std::forward<T>(whatever))));
}


#undef template_IndexProxifier_LRProxy_boilerplate

#endif //def_h_include_lrproxy_hh
