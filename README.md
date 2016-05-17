# CWrapper
Template class for wrapping C resources in RAII

_______________________________________________


## Overview


CWrapper is **one** class that you can use to wrap C resource management. Designed to be easy-to-use, and compatible with most common C practices.

It generates 
* constructor
* move constructor
* move assignment operator
* destructor

for you automatically. You can choose whether copy constructor should be deleted or generated.

Optionally, `operator->` and conversion operators can be generated, with or without *const safety*.

## Requirements

CWrapper is one header file, with no dependencies, you can use it for any C library.

However, it higly relies on C++11 features, so you will need a C++11-compatible compiler.

## Basic usage

Declaration:

```C++
template<
    typename HANDLE_T,
    typename FUNCTIONS,
    CWrapperType TYPE = CWrapperType::Get,
    bool CONSTSAFE = true>
using CWrapper = /* internal template magic */;
```
#### `HANDLE_T`

Type of handle to wrap, example: `FILE*`. `HANDLE_T` must be copy-constructable and copy-assignable. There must be a possible value of `HANDLE_T` indicating that the handle is invalid, for example, `nullptr` for pointers. This invalid value is customizable.

If `HANDLE_T` is a pointer type, `operator->` will be generated for the wrapper.

#### `FUNCTIONS`

This shall be a class that has **static** functions called `ctor_func` and `dtor_func`, these will be called in constructor and destructor. If `FUNCTIONS` has a static function called `copy_func`, CWrapper will also generate copy constructor and copy assignment operator. Optionally, `invalid_value` static member, `validate_func` static function and `exception` nested type can be specified. None of these functions are allowed to throw an exception.

However, it should be called `CONTROLLER` as this term describes its behavior better.

* `invalid_value` shall be an instance of `HANDLE_T` or convertible to `HANDLE_T`. Handle is set to `invalid_value` after moving out the handle from an rvalue reference. It has a default value: `nullptr` if `HANDLE_T` is a pointer, and `0` otherwise.
* `validate_func` shall have one `HANDLE_T` parameter. It is called after all `ctor_func` and `copy_func` calls. It shall return `true` if the handle passed as parameter is valid, `false` otherwise. It has a default value, checks if parameter is `invalid_value`.
* `ctor_func` shall return with `HANDLE_T`. It may have any parameters except `HANDLE_T`, and it may also be overloaded. The return value will be checked with `validate_func`.
* `dtor_func` shall have one `HANDLE_T` parameter, its return value is discarded. The function is called only if `validate_func` returned `true`. 
* `copy_func` shall return with `HANDLE_T` and it shall have one `const HANDLE_T` parameter. The return value will be checked with `validate_func`.
* `exception` is thrown (with its default constructor) whenever `ctor_func` or `copy_func` returns an invalid handle. Its default type is `std::bad_alloc`. 
* This seems a bad limit that the default constructor is called, but this is more flexible than you think. Just make a custom exception type with a more informative default constructor. 

Example:

```C++
class stdc_error : public std::exception
{
    int error_number = errno;
public:
    virtual const char * what() const noexcept override
    {
        return strerror(error_number);
    }
};
```


However, it is not required use actual functions, they can be any function objects, like `std::function`, function pointers or auto variables: `static constexpr auto ctor_func = fopen;`

#### `TYPE`

Its type is `CWrapperType`:

```C++
enum class CWrapperType
{
    Implicit,
    Explicit,
    Get,
};
```
* `Implicit`: CWrapper will generate an implicit conversion operator to `HANDLE_T`. 
* `Explicit`: CWrapper will generate an explicit conversion operator to `HANDLE_T`. 
* `Get`: CWrapper will generate a member function called `.get()` that returns `HANDLE_T`. 

`TYPE` is `CWrapperType::Get` by default.

#### `CONSTSAFE`

If `CONSTSAFE` is `true` and `HANDLE_T` is a **pointer** type, CWrapper will have two conversion operators and two `operator->`.

For example, if `TYPE` is `CWrapperType::Get`, wrapper will have the following functions:

```C++
PTR_T* get();
PTR_T const* get() const;

PTR_T* operator->();
PTR_T const* operator->() const;
```

If `CONSTSAFE` is `false` and `HANDLE_T` is a **pointer** type, wrapper will have these functions:

```C++
PTR_T* get() const;
PTR_T* operator->() const;
```

If `HANDLE_T` is **not** a pointer type, `CONSTSAFE` has no effect, and wrapper will have this function:

```C++
HANDLE_T get() const;
```

`CONSTSAFE` is `true` by default.

## Advanced usage

CWrapper's main purpose is to hide resource management in RAII, but keep the behaviour of the original handle. It is *not* designed for converting all global handling functions to member functions, it is your job! However, CWrapper makes it easier, I will add examples showing this.


## Known issues

This is not really an issue of `CWrapper`, but it must be noted: **`dtor_func` should never be called directly**.

##### BAD EXAMPLE:

```C++
using FileWrapper = CWrapper<FILE*, FileWrapperFunctions, CWrapperType::Implicit, false>;

int main() 
{
    FileWrapper file("example.txt", "r");
    // use file
    fclose(file); // <- ERROR
}
```

The destructor of file will call `fclose` again on the internal pointer, so this will result in a crash.

##### Solutions:

1. Do not use implicit conversion, so there is a smaller chance that you will call `fclose` without paying attention.
2. Delete the function:

```C++
void fclose(FileWrapper const&) = delete;
```

Currently there is no elegant solution if you really need to close a wrapper, to call its destructor function explicitly. This might be inevitable for example if it is a global variable.

##### Workaround:

```C++
{ FileWrapper temp = std::move(file); }
```

## More examples

Current `main.cpp` wraps `FILE*` as it is the only C resource management in the standard library.

I will add some more examples soon, using various libraries like SDL2, iniparser and pcap.
