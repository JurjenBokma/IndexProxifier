
#include "../../indexproxifier.hh"
#include "../../../unittest/unittest.hh"
#include "eightbits/eightbits.hh"
#include "retbyref/retbyref.hh"
#include "retbyref/retbyconstref.hh"
#include "retbyref/retbyvalue.hh"

#include <chrono>
#include <ios>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace {
    void ut_op_indexproxifier_conversion_type();
    void ut_no_copy_constructor();
    void ut_op_assign();
    void ut_io();
}

struct CopyCounter
{
    std::size_t d_count = 0;
public:
    CopyCounter() = default;
    CopyCounter(CopyCounter const &other)
        : d_count(other.d_count + 1)
    {}
    CopyCounter(CopyCounter &&tmp)
        : d_count(tmp.d_count + 1)
    {}
    CopyCounter &operator=(CopyCounter const &tmp)
    {
        d_count = tmp.d_count + 1;
        return *this;
    }
    CopyCounter &operator=(CopyCounter &&tmp)
    {
        d_count = tmp.d_count + 1;
        return *this;
    }
};

using namespace std;

int main()
{
    ut_no_copy_constructor();
    ut_op_indexproxifier_conversion_type();
    ut_op_assign();
    ut_io();

    return TestCount::result();
}

namespace {

    void ut_no_copy_constructor()
    {
        typedef typename std::remove_reference<decltype(EightBits{}[1])>::type Proxy;

        static_assert(not std::is_copy_constructible<Proxy>::value,
                      "LRProxy should not be copy-constructible.");
    
        static_assert(not std::is_copy_assignable<Proxy>::value,
                      "LRProxy should not be copy-assignable. Any chaining should only be done on conversion values.");

        static_assert(not std::is_move_assignable<Proxy>::value,
                      "LRProxy should not be move-assignable, even though conversion of the new value will still accomplish it.");

        //NB: std::is_copy_assignable forces normal reference.
        //NB: So any assert specifically for Proxy&& is pointless.
    }

    void ut_op_indexproxifier_conversion_type()
    {

        test(
            "Eightbits index can be statically cast to bool.",
            []()
            {
                EightBits eb{0b00001111};
                EightBits const cb{0b00001111};
                return
                    static_cast<bool>(eb[1]) == 1
                    &&
                    cb[1] // Implict cast
                    &&
                    not cb[6];
            });
        
        test(
            "RetByRef index can be cast to both int and int&.",
            []()
            {
                RetByRef rbr{1, 2, 3, 4};
                return
                    static_cast<int&>(rbr[2]) == 3
                    &&
                    static_cast<int>(rbr[3]) == 4;
            });
        
        test(
            "RetByConstRef index can be cast to both int and int const&.",
            []()
            {
                RetByConstRef rbcr{1, 2, 3, 4};

                return
                    static_cast<int const &>(rbcr[2]) == 3
                    &&
                    static_cast<int>(rbcr[3]) == 4;
            });
        test(
            "If proxy_return_value returns by value, index operator should elide.",
            []()
            {
                typedef RetByValue<CopyCounter> PassCCByValue;
                PassCCByValue pbv;
                CopyCounter value(pbv[0]);
                return value.d_count == 1;
            });
    }

    
    void ut_op_assign()
    {
        // Assignment to LRProxy from own conversion type should work.
        {
            int runtime_determined = std::chrono::system_clock::now().time_since_epoch().count() == 0 ? 0 : 1;
            test("It should be possible to assign from the converte-to type to the proxy.",
                 [&runtime_determined]()
                 {
                     if (runtime_determined != 1)
                         fail("This program needs 'runtime_determined' to be 1. Please fix! ");
                     EightBits bits(runtime_determined);
                     bits[1] = true;
                     return (bits.internal() & 0b1) != 0;
                 });
        }
        {
            test("If proxy_assign_action returns a reference, we should be able to assign to it through the proxy.",
                 []()
             {
                 RetByRef numbers{1, 2, 3, 4};
                 int &picked = numbers[1];
                 picked = 12;
                 if (&picked != &numbers.d_data[1])
                     fail("Returned reference has unexpected address, outside indexed object."
                         "Return-by-reference should pass on the reference.");
                 int newval = 14; // new value also taken by reference, so must be variable.
                 numbers[1] = newval;
                 return numbers[1] == 14;
             });
        }
        {
            test(
                "Assignment to LRProxy from convertible-to-own-conversion-type should work.",
                []()
                {
                    EightBits eb;
                    double const pi = 3.14159;
                    eb[2] = pi;
                    return static_cast<bool>(eb[2]); // Should be true.
                });
        }
        {
            EightBits bits(0b01000010);
         
            test("Proxy-to-proxy assignment on identical LRProxy types should work.",
                 [&bits]() -> bool
                 {
                     return bits[0] = bits[1];
                 });
        }
        {
            test("Assignment to LRProxy from other LRProxy types should work.",
                 []()
                 {
                     EightBits bits(0b00001111);
                     RetByRef rbr{1, 1, 0, 0};
                     bits[0] = rbr[3];
                     bits[7] = rbr[0];
                     return
                         not bits[0] &&
                         bits[7];
                 });
        }
        // Assignment to LRProxy should not work if Owner is a const type.
        {
            test("If only assignment is used, proxy_return_action is not needed. "
                 "And if proxy_accept_action returns void, so should assignment.",
                 []()
                 {
                     struct Local: protected IndexProxifier<Local>
                     {
                         int proxy_return_action(size_t ix) const;
                         void proxy_accept_action(size_t ix, int newvalue)
                         {}
                         using IndexProxifier<Local>::operator[];
                         friend IndexProxifier<Local>;
                     };
                     Local local;
                     return std::is_same<void, decltype(local[1] = 3)>::value;
                 });
        }
        {
            EightBits eb;
            static_assert(std::is_same<bool, decltype(eb[1] = true)>::value,
                          "If proxy_accept_action returns by value, so should assignment.");
        }
        {
            RetByRef rbr;
            int newval = 100;
            static_assert(std::is_same<int &, decltype(rbr[1] = newval)>::value,
                          "If proxy_accept_action returns by reference, so should assignment.");
        }
        {
            test("Chained LRProxy assignment should work.",
                 []()
                 {
                     EightBits bits(0b0);
         
                     return bits[1] = bits[0] = true;
                 });
        }
        test(
            "If proxy_accept_value returns by value, chained assignment should elide.",
            []()
            {
                typedef RetByValue<CopyCounter> PassCCByValue;
                PassCCByValue pbv;
                CopyCounter value(pbv[0] = pbv[1]);
                return value.d_count == 2; // Once for construction and once for assignment.
            });
    }

    void ut_io()
    {


        test("Formatted input from stream to LRProxy",
             []()
             {
                 EightBits bits(12);
                 istringstream in("0 1 0 1 1 0 1 0"s);
                 for (size_t ix = 0; ix != 8; ++ix)
                 {
                     in >> bits[ix];
                     if (not in)
                     {
                         fail();
                         break;
                     }
                 }
                 return bits.internal() == 0b01011010;
             });

        test("Formatted output of LRProxy to stream",
             []()
             {
                 EightBits bits(0b01011010);
                 ostringstream out;
                 for (size_t ix = 0; ix != 8; ++ix)
                     out << bits[ix];
                 return out.str() == "01011010";
             });
    }
    
}



//     {
//    
//     }
//     {
//    
// 
//         bits[1] == bits[2];
//         bits[1] <=> bits[2];
//         bits[1] && bits[2];
//         bits[1] & bits[2];
//         bits[1] += true;
//         bits[1] += bits[2];
//     }
