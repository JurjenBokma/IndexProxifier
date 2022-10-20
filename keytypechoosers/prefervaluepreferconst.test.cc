
#include "../indexproxifier.hh"


// This template simply reduces the amount of code.
template <typename KeyT, typename ReturnT>
struct Object
{
    ReturnT proxy_return_action(KeyT);
};

template <typename KeyT, typename ReturnT>
ReturnT Object<KeyT, ReturnT>::proxy_return_action(KeyT)
{
    return ReturnT{};
}


// A struct to use as a key type.
struct Struct
{
    void *pointers[4]; // Big enough to cause KeyT to be a reference.
};


typedef Object<int, double> BuiltInKey;
void builtin_key_by_value_works();

typedef Object<Struct, double> ObjectKey;
void object_key_by_const_ref_works();

typedef Object<int &, double> BuiltInRefKey;
void builtin_key_by_ref_works();

typedef Object<Struct &, double> ObjectRefKey;
void object_key_by_ref_works();

using namespace std;

int main()
{
    // Functions merely contain static_asserts.
    // Running does nothing. It it compiles, we're good.
    builtin_key_by_value_works();
    object_key_by_const_ref_works();
    builtin_key_by_ref_works();
    object_key_by_ref_works();
}

void builtin_key_by_value_works()
{
    typedef int expected_t;
    // Values remain values but keep cv-qualifiers.
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<int, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t const, PreferValuePreferConst<int const, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<int &, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t const, PreferValuePreferConst<int const &, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t volatile, PreferValuePreferConst<int volatile &, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t const volatile, PreferValuePreferConst<int const volatile &, BuiltInKey>::type>::value
        );
    // Rvalue references become values but keep their qualifiers.
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<int &&, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t const, PreferValuePreferConst<int const &&, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t volatile, PreferValuePreferConst<int volatile &&, BuiltInKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t const volatile, PreferValuePreferConst<int const volatile &&, BuiltInKey>::type>::value
        );
}


void object_key_by_const_ref_works()
{
    typedef Struct const &expected_t;
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<Struct, ObjectKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<Struct const, ObjectKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<Struct &, ObjectKey>::type>::value
        );
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<Struct const &, ObjectKey>::type>::value
        );
    // proxy_return_action doesn't handle volatile, so type simply becomes KeyT.
    static_assert(
        std::is_same<Struct volatile &, PreferValuePreferConst<Struct volatile &, ObjectKey>::type>::value
        );
    static_assert(
        std::is_same<Struct const volatile&, PreferValuePreferConst<Struct const volatile &, ObjectKey>::type>::value
        );

    static_assert(
        std::is_same<Struct, PreferValuePreferConst<Struct &&, ObjectKey>::type>::value
        );
    static_assert(
        std::is_same<Struct const, PreferValuePreferConst<Struct const &&, ObjectKey>::type>::value
        );
    static_assert(
        std::is_same<Struct volatile &&, PreferValuePreferConst<Struct volatile &&, ObjectKey>::type>::value
        );
    static_assert(
        std::is_same<Struct const volatile &&, PreferValuePreferConst<Struct const volatile &&, ObjectKey>::type>::value
        );
}

void builtin_key_by_ref_works()
{
    typedef int &expected_t;
    // The only thing that makes sense to pass is an int&:
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<int &, BuiltInRefKey>::type>::value
        );
    // Cannot pass int const to proxy_return_action taking int&
    // Cannot pass many other types either.
    // We could pass volatile types, but C++20 just deprecated those. Not even trying.
}

void object_key_by_ref_works()
{
    typedef Struct &expected_t;
    // The only thing that makes sense to pass is a Struct&:
    static_assert(
        std::is_same<expected_t, PreferValuePreferConst<Struct &, ObjectRefKey>::type>::value
        );
}
