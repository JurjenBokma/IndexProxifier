/**
   Ensure that IndexProxifier's index operators return LRProxies.
   Such an LRProxy holds a reference to the IndexProxifier's Derived type.
   That reference must match the cvrref-ness of the IndexProxifier.

   Example:
        typedef UsesProxy const cUP;
        cUP{}[1];
   The index operator is applied on an anonymous temporary const UsesProxy.
   The index operator returns an LRProxy.
   Therefore, the reference inside that LRProxy should be a const rvalue
   reference to UsesProxy.
 */

#include "indexproxifier.hh"
#include <type_traits>

struct MyStruct: protected IndexProxifier<MyStruct>
{
    typedef IndexProxifier<MyStruct> BaseT;

    MyStruct() = default;
    MyStruct(MyStruct const &other) = default;
    MyStruct(MyStruct &&tmp) = default;
    
    int proxy_return_action(int) const volatile;
    int proxy_accept_action(int, int);

    friend BaseT;
    
    using BaseT::operator[];
};

int MyStruct::proxy_return_action(int) const volatile
{
    return 0;
}

int MyStruct::proxy_accept_action(int, int)
{
    return 0;
}

MyStruct factory()
{
    MyStruct retval;
    return retval;
}

MyStruct const cfactory()
{
    MyStruct const retval;
    return retval;
}

// Volatile parameters and return types deprecated in C++20.

using namespace std;

int main()
{
    MyStruct ms;
    static_assert(std::is_same<MyStruct &, decltype(ms[1])::Owner_T>::value,
                  "Index operator on value should have a reference Owner_T.");
    
    MyStruct &ref(ms);
    static_assert(std::is_same<MyStruct &, decltype(ref[1])::Owner_T>::value,
                  "Index operator on lvalue ref should have a reference Owner_T.");
    
    MyStruct const &cref(ms);
    static_assert(std::is_same<MyStruct const &, decltype(cref[1])::Owner_T>::value,
                  "Index operator on const lvalue ref should have a const reference Owner_T.");

    MyStruct volatile &vref(ms);
    static_assert(std::is_same<MyStruct volatile &, decltype(vref[1])::Owner_T>::value,
                  "Index operator on volatile lvalue ref should have a volatile reference Owner_T.");

    MyStruct const volatile cvref(ms);
    static_assert(std::is_same<MyStruct const volatile &, decltype(cvref[1])::Owner_T>::value,
                  "Index operator on const volatile lvalue ref should have a const volatile reference Owner_T.");


    static_assert(std::is_same<MyStruct &&, decltype(factory()[1])::Owner_T>::value,
                  "Index operator on rvalue reference should have an rvalue reference Owner_T.");

    static_assert(std::is_same<MyStruct const &&, decltype(cfactory()[1])::Owner_T>::value,
                  "Index operator on rvalue reference should have an rvalue reference Owner_T.");
    
}
