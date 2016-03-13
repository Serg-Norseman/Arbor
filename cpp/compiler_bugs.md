# Catalog of Compiler Bugs

## Intel C++ Compiler

### ICC doesn't allow to declare a _template_ class where a static constexpr member function is used inside the class body

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

It may be a bug or a by-design behaviour. But I lean towards a bug. Because:
- 14.7.1 Implicit instantiation [temp.inst]
  - 14.7.1 The implicit instantiation of a class template specialization causes the implicit instantiation of the declarations, but not of the definitions,    
but
  - 14.7.3 Unless a member of a class template or a member template has been explicitly instantiated or explicitly specialized, the specialization of the member is implicitly instantiated when the specialization is referenced in a context that requires the member definition to exist.

Therefore when a compiler has implicitly instantiated `foo` at `foo_t<__m128, 33> foo {}`, it makes compile-time calculation to check the assertion. And here '14.7.3' comes into play. But ICC ignores it and handles `foo_t` template class as a normal class type.

The problem is discussed in ["static_assert and Intel C++ compiler"]
(http://stackoverflow.com/questions/35764069/static-assert-and-intel-c-compiler) topic.

### ICC can't deduce variable type when a variable is declared using a placeholder type

ICC fails when compiles one of the following code:

```cpp
int foo = 3;
auto boo {foo};
foo = boo + 1;
```

or

```cpp
int foo;
auto boo {4};
foo = boo + 1;
```

Intel C++ compiler considers code ill-formed because:

```
error : no operator "+" matches these operands
        operand types are: std::initializer_list<int> + int
     foo = boo + 1;
               ^
```

Therefore it deduces type of `boo` to `std::initializer_list<int>`. This was OK before C++17. But now this violates '7.1.6.4 auto specifier [dcl.spec.auto]':

> When a variable declared using a placeholder type is initialized, or a return statement occurs in a function
declared with a return type that contains a placeholder type, the deduced return type or variable type is
determined from the type of its initializer. In the case of a return with no operand or with an operand of
type void, the declared return type shall be auto and the deduced return type is void. Otherwise, let T be
the declared type of the variable or return type of the function. If the placeholder is the auto type-specifier,
the deduced type is determined using the rules for template argument deduction. If the initialization is
direct-list-initialization then the braced-init-list shall contain only a single assignment-expression L. If the
deduction is for a return statement and the initializer is a braced-init-list (8.5.4), the program is ill-formed.
Otherwise, obtain P from T by replacing the occurrences of auto with either a new invented type template
parameter U or, if the initialization is copy-list-initialization, with std::initializer_list<U>. Deduce
a value for U using the rules of template argument deduction from a function call (14.8.2.1), where P is
a function template parameter type and the corresponding argument is the initializer, or L in the case of
direct-list-initialization. If the deduction fails, the declaration is ill-formed. Otherwise, the type deduced for
the variable or return type is obtained by substituting the deduced U into P.

See ["ICL can't deduce variable type when a variable is declared using a placeholder type (auto) [bug since C++17]"](https://software.intel.com/en-us/forums/intel-c-compiler/topic/611948) topic for more information.

*(this is a bug confirmed by Intel; it is to be fixed in Intel C++ Compiler 17.0 -- see the forum topic I specified above)*
