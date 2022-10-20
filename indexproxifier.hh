
#ifndef def_h_include_indexproxifier_hh
#define def_h_include_indexproxifier_hh

#include <experimental/type_traits> // is_detected
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

#ifdef DEBUG
    #define DEBUG_INDEXPROXIFIER true
#else
    #define DEBUG_INDEXPROXIFIER false
#endif

#include "keytypechoosers/prefervaluepreferconst.hh" // Keytype choice policy.
#include "keytypechoosers/byvalue.hh" // Alternative policy (example).


/**
   Uses CRTP to provide its template parameter Derived with:

       LRProxy<Key, [cv] Derived [&[&]]> operator[](Key);

   LRProxy is sensitive to being lvalue or rvalue.

   Derived's cv-qualifications and rvalue-ness carry over into the reference to
   it contained by LRProxy. This is to prevent the CRTP pattern's static_casts
   from lying about the cv-qualifiers and the reference type (rvalue or not) of
   its Derived type.

   Requires C++17 or later (for auto template parameters).
 */
template <typename Derived, template <typename, typename> typename KeyTypeChooser = PreferValuePreferConst>
class IndexProxifier
{

    // Derived is (and should be) incomplete; postpone check that we are a base
    // of Derived until constructor of LRProxy.
  
private:

    // Declaration of nested class LRProxy. (See below.)
    // Owner will be some reference. K perhaps too, but most likely a value.
    template <typename K, typename Owner>
    class LRProxy;

    friend class IndexProxifier_unittest;
    friend class LRProxy_unittest;
    
protected:

    // Protect constructors. User must not create standalone IndexProxifiers.
    constexpr IndexProxifier() = default;
    constexpr IndexProxifier(IndexProxifier const &other) = default;
    constexpr IndexProxifier(IndexProxifier &&tmp) = default;

    // Constness, volatility and rvalue-reference-ness of *this carry over onto
    // the Derived [const] [volatile] [&]d_owner in the LRProxy.
    // KeyTypeChooser chooses LRProxy's d_key based on K.
    template <typename K>
    constexpr LRProxy<K, Derived &>                       operator[](K &&key) &;

    template <typename K>
    constexpr LRProxy<K, Derived const &>                 operator[](K &&key) const &;
    
    template <typename K>
    constexpr LRProxy<K, Derived volatile &>              operator[](K &&key) volatile &;

    template <typename K>
    constexpr LRProxy<K, Derived const volatile &>        operator[](K &&key) const volatile &;

    template <typename K>
    constexpr LRProxy<K, Derived &&>                      operator[](K &&key) &&;

    template <typename K>
    constexpr LRProxy<K, Derived const &&>                operator[](K &&key) const &&;

    template <typename K>
    constexpr LRProxy<K, Derived volatile &&>             operator[](K &&key) volatile &&;

    template <typename K>
    constexpr LRProxy<K, Derived const volatile &&>       operator[](K &&key) const volatile &&;

};


// The nested LRProxy class template.
#include "lrproxy/lrproxy.hh"

// The index operator function templates. All differ in four congruent spots.

template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived &> // 1: Derived&
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) & // 2: '&' ref-qualifier
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Normal reference, operator[] -> LRProxy<K, Derived&>.\n";
        });
    return LRProxy<K, Derived &>( // 3: Derived&
        static_cast<Derived &>(*this), // 4: Static cast to Derived& passed to constructor.
        std::forward<K>(key)
        );
}

template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived const &>
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) const &
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Const reference, operator[] -> LRProxy<K, Derived const &>.\n";
        });
    return LRProxy<K, Derived const &>(
        static_cast<Derived const &>(*this),
        std::forward<K>(key)
        );
}

template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived volatile &>
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) volatile &
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Volatile reference, operator[] -> LRProxy<K, Derived volatile &>.\n";
        });
    return LRProxy<K, Derived volatile &>(
        static_cast<Derived volatile &>(*this),
        std::forward<K>(key)
        );
}

template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived const volatile &>
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) const volatile &
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Const volatile reference, operator[] -> LRProxy<K, Derived const volatile&>.\n";
        });
    return LRProxy<K, Derived const volatile &>(
        static_cast<Derived const volatile &>(*this),
        std::forward<K>(key)
        );
}

// RValue cases ...
template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived &&>
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) &&
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Rvalue reference, operator[] -> LRProxy<K, Derived &&>.\n";
        });
    return LRProxy<K, Derived &&>(
        static_cast<Derived &&>(*this),
        std::forward<K>(key)
        );
}

template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived const &&>
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) const &&
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Const rvalue reference, operator[] -> LRProxy<K, Derived const &&> (which is silly).\n";
        });
    return LRProxy<K, Derived const &&>(
        static_cast<Derived const &&>(*this),
        std::forward<K>(key)
        );
}

template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived volatile &&>
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) volatile &&
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Volatile rvalue reference, operator[] -> LRProxy<K, Derived volatile &&>.\n";
        });
    return LRProxy<K, Derived volatile &&>(
        static_cast<Derived volatile &&>(*this),
        std::forward<K>(key)
        );
}

template <typename Derived, template <typename, typename> typename KeyTypeChooser>
template <typename K>
constexpr typename IndexProxifier<Derived, KeyTypeChooser>:: template LRProxy<K, Derived const volatile &&>
IndexProxifier<Derived, KeyTypeChooser>::operator[](K &&key) const volatile &&
{
    ifdebug<DEBUG_INDEXPROXIFIER>::run(
        []()
        {
            std::cout << "Const volatile rvalue reference, operator[] -> LRProxy<K, Derived const volatile&&>.\n";
        });
    return LRProxy<K, Derived const volatile &&>(
        static_cast<Derived const volatile &&>(*this),
        std::forward<K>(key)
        );
}

#endif //def_h_include_indexproxifier_hh
