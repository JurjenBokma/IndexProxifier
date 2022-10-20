
#include "flexible/flexible.hh"

#include "../../indexproxifier.hh"
#include "../../../unittest/unittest.hh"

struct MyStruct
{
    double value = 3.14;
    MyStruct() = default;
    MyStruct(double initial)
        : value(initial)
    {}
};

bool operator==(MyStruct const &lhs, MyStruct const &rhs)
{
    return lhs.value == rhs.value;
}

enum class Incompatible{};

using namespace std;

int main()
{

    //       typename DataT,
    //       typename KeyT,
    //       typename ReturnT,
    //       typename AcceptT,
    //       typename ValueT,
    //       typename cKeyT = KeyT,
    //       typename cReturnT = ReturnT,
    //       typename cAcceptT = AcceptT,
    //       typename cValueT = ValueT,
    //       typename rrKeyT = KeyT,
    //       typename rrReturnT = ReturnT,
    //       typename rrAcceptT = AcceptT,
    //       typename rrValueT = ValueT

    double const original_value = 130.17;
    MyStruct ms(original_value);

    // A Flexible contains an internal array of two MyStructs.
    Flexible<MyStruct, int, MyStruct &, MyStruct &, MyStruct &> bender;

    // This must compile.
    MyStruct str = bender[0]; // Conversion of LRProxy to a value.
    bender[0] = ms; // Assignment to an LRProxy.
    static_assert(std::is_same<MyStruct&, decltype(bender[0] = ms)>::value,
                  "Expected a reference out of assignment");
    str = bender[0] = bender[1] = bender[1] = ms; // Chained assignment.
    
    // And then this must be the result.
    test("Assignment to/from proxies actually gives the right results.",
         [&str, &bender, &ms]() -> bool
          {
              return str == bender[0]
                  && bender[0] == bender[1]
                  && bender[1] == ms;
          });

    test("Since bender's type return a reference, we can take the address of return_value.",
         [&bender]() -> bool
         {
             return &static_cast<MyStruct&>(bender[1]) - &static_cast<MyStruct&>(bender[0]) == 1;
         });
    
    return TestCount::result();
}
