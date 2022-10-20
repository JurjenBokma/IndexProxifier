
#include "eightbits/eightbits.hh"

#include "../../indexproxifier.hh"
#include "../../../unittest/unittest.hh"
#include <experimental/type_traits>

template<typename T>
struct Attempt
{
    static_assert(std::is_same<T, EightBits>::value,
                  "Attempt is only meant for use with EightBits.");
    
    decltype(T(0b01000010)[1]) &ref; // Lvalue reference

    // Sneaky: constructor binds an lvalue reference to an LRProxy that holds an
    // rvalue reference to it owner (because the owner *is* temporary and
    // anonymous).
    constexpr Attempt()
        : ref(T(0b01000010)[1])
    {}
};

template <typename T>
using Possible = decltype(T(0b01000010)[1] = 1);

template <typename T>
using Impossible = decltype(T{}.ref = 1);

using namespace std;

int main()
{

    static_assert(std::experimental::is_detected<Possible, EightBits>::value,
                  R"explain(
    Temporary EightBits::LRProxy should support assignment: the LRProxy will
    rvalue-reference a temporary object in the same expression.
)explain");

    static_assert(false == std::experimental::is_detected<Impossible, EightBits>::value,
                  R"explain(
    Lvalue reference to an EightBits::LRProxy that holds an rvalue reference to
    an anonymous temporary EightBits should _not_ support assignment, because
    the EightBits will already be destructed at the time of assignment.
)explain");

}
