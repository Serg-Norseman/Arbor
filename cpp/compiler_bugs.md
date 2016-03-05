# Catalog of Compiler Bugs

## Intel C++ Compiler

### ICC doesn't allow to declare a _template_ class where a static member function of the class is used inside the class body

ICC fails to compile the following code:

```cpp
template <typename T, size_t Alignment>
class foo_t
{
private:
    static constexpr std::size_t maxAlignment()
    {
        return max(std::alignment_of<T>::value, Alignment);
    }

    static_assert(maxAlignment(), "some message");
};

foo_t<__m128, 33> foo {};
```

The compiler finds that:

```
error : function call must have a constant value in a constant expression
               static_assert(maxAlignment(),
                             ^
```

The reason for this error is that when ICC tries to compute compile-time constant value for the first argument of `static_assert`, it has `foo_t` type as **incomplete** one (`maxAlignment` class member is declared, but not yet defined).

Here is a more clear example for this situation:

```cpp
class foo_t
{
private:
    static constexpr std::size_t maxAlignment()
    {
        return max(std::alignment_of<T>::value, Alignment);
    }

    static_assert(maxAlignment(), "some message");
};
```

MSVC and ICC both fail to compile this because `foo_t` is incomplete when a compiler checks the assertion. GCC fails too.

See ["constexpr not working if the function is declared inside class scope"](http://stackoverflow.com/questions/16493652/constexpr-not-working-if-the-function-is-declared-inside-class-scope) topic for more information.

When MSVC compiles the first code:

```cpp
template <typename T, size_t Alignment>
class foo_t
{
. . .
```

it succeeds. I believe because MSVC (and it looks like GCC too) instantiates `foo_t<__m128, 33> foo {}` having complete information about `foo_t`. But ICC does not have complete type there.

It may by a bug or a by-design behaviour. But I lean towards a bug. Because:
- 14.7.1 Implicit instantiation [temp.inst]
  - 14.7.1 The implicit instantiation of a class template specialization causes the implicit instantiation of the declarations, but not of the definitions,    
but
  - 14.7.3 Unless a member of a class template or a member template has been explicitly instantiated or explicitly specialized, the specialization of the member is implicitly instantiated when the specialization is referenced in a context that requires the member definition to exist.

Therefore when a compiler has implicitly instantiated `foo` at `foo_t<__m128, 33> foo {}`, it makes compile-time calculation to check the assertion. And here '14.7.3' comes into play. But ICC ignores it and handles `foo_t` template class as a normal class type.

The problem is discussed in ["static_assert and Intel C++ compiler"]
(http://stackoverflow.com/questions/35764069/static-assert-and-intel-c-compiler) topic.