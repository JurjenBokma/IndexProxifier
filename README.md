# IndexProxifier CRTP template in C++.

## Purpose of a Transparent Proxy

Occasionally we need to provide a class with an index operator that cannot
return a reference. This happens e.g. when using the object as an lvalue
requires different behavior from using it as an rvalue, or when there is no
referenceable content in the first place. (Examples further down).
We might then create some Proxy type and return that, but writing one is
somewhat error-prone.

## Use

    class myclass : protected IndexProxifier < myclass >
	  {...};

    class myclass : protected IndexProxifier < myclass,
                    my_keychoice_policy >
	  {...};

    The IndexProxifier uses C++20 concepts, so -std=c++20 is required.

## Configuration
The class `myclass` must do the following:

1. Provide a (private) function

       [constexpr] ret_type proxy_return_action(Key key) const;

   The resulting ret_type will be returned by the index operator when
   it is on the right hand side of an assignment.

2. Provide a (private) function

       [constexpr] some_type proxy_accept_action(Key key, Value value);

   This will be called when the index operator is used on the left hand
   side of an assignment. The return type some_type will be returned from the
   assignment operator on an LRProxy.

3. Declare

       friend class IndexProxifier<myclass>;

   so the proxifier will be allowed to call the above two functions.

4. Make the injected index operator(s) public, if desired:

       public:
       using IndexProxifier<myclass>::operator[];

## What it does
The template IndexProxifier uses CRTP to provide its template parameter with:

       LRProxy<Key, Derived> operator[](Key &&);

The LRProxy is sensitive to being being on the left hand side or right hand
side of an assignment.

IndexProxifier  has  no  members. Empty base class optimization applies. It
doesn't have a virtual destructor. So insofar as public inheritance suggests
polymorphism, it is unfit for that. Use private or protected inheritance
instead.

## Why it exists

IndexProxifier will handle situations where operator[] cannot return a value
or reference.

Most index operators can and do return by value or by reference. E.g.

    constexpr int &vector<int>::operator[](size_t index);
    constexpr int const &vector<int>::operator[](size_t index) const;

Sometimes this is not possible, e.g. with

    bitset::operator[](size_t index); // Assuming 8 bits stored in 1 byte.

A single bit is not addressable. Moreover, it matters whether the returned
type is used as an lvalue or an rvalue:

    bitset bs;
    bool truth = bs[0]; // Just convert to bool.
    bs[0] = true; // Flip bit in bs *after* operator[] has returned(!)

The solution is to return an LRProxy that holds a reference to the indexed
object as well as the index. The LRProxy has a conversion operator (in this
case to bool so `truth` can be set. And it has an assignment operator (from
bool here) so that `bs[0]` can be set.

IndexProxifier creates this LRProxy based on the `proxy_return_action` and
`proxy_accept_action` member functions.

## Notes
IndexProxifier tries hard to Just Work.

A hand-crafted Proxy might create an undesirable back door:

    struct myclass::proxy
    {
        myclass &owner; // Modifiable even if proxy is const
    };
    myclass const mc;
    mc[1] = true; // Inadvertently allowed.

IndexProxifier prevents this by transferring the cv- and rvalue-reference
qualifiers of the indexed object onto the owner inside the LRProxy.

Often, a hand-crafted myclass' operator[](...) const can return by value.
IndexProxifier instead returns an LRProxy even here:

    LRProxy operator[](keytype key) const;

It does this for multiple reasons:

- Orthogonality, or the principle of least surprise.
  The non-const index operator will not call proxy_return_action upon
  creation of the LRProxy return object, but only when the LRProxy is
  converted to a value. Therefore, the const index operator should also
  only call proxy_return_action when conversion is done.

- Odd configurations, e.g. proxy_accept_action(...) const, perhaps with
  a mutable data member, don't break anything.

- If the user would rather have a `return_type operator[](...)` const,
  that is easily hand-crafted with a wrapper:

  myclass::value_type myclass::operator[](keytype key) const
  {
      // perhaps even use (assuming conversion):
      return IndexProxifier<myclass>::operator[](key);
  }

The LRProxy deletes its copy constructor to prevent dangerous constructions
like:

    auto cpy = bitset{}[2];
    cpy = true; // Temporary bitset already destructed.

and like:

    auto function()
    {
        bitset bs;
		return bs[3]; // 'bs' going out of scope.
    }

Both are dangerous because binding a return value to a class-scope reference
does not extend the lifetime of the object.

For the same reason, storing the `key` by value is to be preferred over storing
it by reference. Yet if the key is an object, which may be expensive to copy,
or if `proxy_accept_action` or `proxy_return`_action take the key by non-const
reference, LRProxy must store it by reference. The user can overide this
behavior by providing IndexProxifier with a second formal parameter that
implements a key choice policy class. Such a non-default policy class can
start out as a copy of the default `PreferValuePreferConst`, which is
commented to explain.

To prevent the idiom:

    auto const &lref = bs[3]; // Bad: dangling danger.

the LRProxy's assignment operator works only if it is an rvalue reference.

## Known Bugs
The LRProxy should be a transparent proxy, never seen nor handled by the user.
Yet the user can do e.g.:

    auto &&bad_thing = myclass{}[1]; //rref to LRProxy
    auto &&also_bad = myclass{}[1] = myclass{}[2];
    // Temporary myclass objects already destructed.
    bad_thing = true; // May segfault.

This binds rvalue references to anonymous temporary LRProxies. It extends the
lifetime of the proxies, but those proxies hold internal references that are
already dangling when used.

This bug is not unique to IndexProxifier. Hand-crafted proxies suffer from it
as well. In fact, we mitigate it by returning a value from LRProxy::operator=,
which is possible and makes sense because a copy of an LRProxy references the
same owner as the original, and the copy will be relatively light-weight
(unless the index type is expensive to copy). But it is still bad and ugly.

## Example
The class EightBits below, by deriving from IndexProxifier<EightBits>, creates
an index operator that allows getting/setting individual bits.

### EightBits

    class EightBits: protected IndexProxifier<EightBits>
    {

        uint8_t d_data = 0;

    public:
                            // One for modifiable and one for const objects.
        using IndexProxifier<EightBits>::operator[];

    private:

        friend IndexProxifier<EightBits>;

        bool proxy_return_action(int key) const;
        bool proxy_accept_action(int key, bool value);

        ... all other member functions left out ...
    };

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

    int main()
	{
        EightBits eb;
		                    // eb[0] on rhs is converted to bool
        bool value = eb[0];
		                    // eb[0] on lhs can be assigned to
        eb[0] = 1;
		                    // chaining works
        eb[2] = eb[1] = eb[0];
	    
        EightBits const ebc;
        value = ebc[1];
        ebc[1] = false; // Won't compile: ebc[1] is const if ebc is const;
	}

### Environment

Another case is e.g. an `Environment` class, where 

    Environment env;        // class definition not shown here
	env["HOME"] = "/root";  // needs to call setenv
	cout << env["HOME"];    // merely needs getenv

## Optimization

The LRProxy was written with const-correctness in mind.
When the compiler is allowed to optimize (-O2), typically no IndexProxifier or
even LRProxy objects show up in the object files or executables. Only the
`proxy_accept_action` and `proxy_return_action` function members of the
inheriting class will be called.
