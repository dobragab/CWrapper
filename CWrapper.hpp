#ifndef CWRAPPER_HPP_INCLUDED
#define CWRAPPER_HPP_INCLUDED


#define HAS_STATIC_MEMBER_DETECTOR(member)                                  \
template<typename T>                                                        \
class HAS_STATIC_MEMBER_DETECTOR_CLASS_ ## member                           \
{                                                                           \
    struct two_ints_type { int two_ints_1, two_ints_2; };                   \
    template<typename U>                                                    \
    static two_ints_type has_func_helper(...) { }                           \
    template<typename U>                                                    \
    static int has_func_helper(int, decltype(U::member)* func = nullptr)    \
    { return 0; }                                                           \
public:                                                                     \
    static constexpr bool value =                                           \
        sizeof(decltype(has_func_helper<T>(0))) == sizeof(int);             \
}

#define HAS_STATIC_MEMBER(type, member)                                     \
    (HAS_STATIC_MEMBER_DETECTOR_CLASS_ ## member<type>::value)

#define HAS_NESTED_TYPE_DETECTOR(member)                                    \
template<typename T>                                                        \
class HAS_NESTED_TYPE_DETECTOR_CLASS_ ## member                             \
{                                                                           \
    struct two_ints_type { int two_ints_1, two_ints_2; };                   \
    template<typename U>                                                    \
    static two_ints_type has_func_helper(...) { }                           \
    template<typename U>                                                    \
    static int has_func_helper(int, typename U::member* ty = nullptr)       \
    { return 0; }                                                           \
public:                                                                     \
    static constexpr bool value =                                           \
        sizeof(decltype(has_func_helper<T>(0))) == sizeof(int);             \
}

#define HAS_NESTED_TYPE(type, member)                                       \
    (HAS_NESTED_TYPE_DETECTOR_CLASS_ ## member<type>::value)


enum class CWrapperType
{
    Implicit,
    Explicit,
    Get,
};

template<
    typename H,
    typename F,
    CWrapperType TY,
    bool C>
class __CWrapperHelper__
{

template<
    typename HANDLE_T,
    typename BASE,
    bool CONSTSAFE>
struct ArrowHandler
{ };

template<
    typename PTR_T,
    typename BASE>
struct ArrowHandler<PTR_T*, BASE, false>
{
    PTR_T* operator->() const
    {   return static_cast<BASE*>(this)->ptr; }
};

template<
    typename PTR_T,
    typename BASE>
struct ArrowHandler<PTR_T*, BASE, true>
{
    PTR_T* operator->()
    {   return static_cast<BASE*>(this)->ptr; }
    PTR_T const* operator->() const
    {   return static_cast<BASE*>(this)->ptr; }
};


template<
    typename HANDLE_T,
    typename BASE,
    CWrapperType TYPE,
    bool CONSTSAFE>
struct ConversionHandler : public ArrowHandler<HANDLE_T, BASE, CONSTSAFE>
{ };

template<
    typename HANDLE_T,
    typename BASE,
    bool CONSTSAFE>
struct ConversionHandler<HANDLE_T, BASE, CWrapperType::Implicit, CONSTSAFE>
    : public ArrowHandler<HANDLE_T, BASE, CONSTSAFE>
{
    operator HANDLE_T() const
    {   return static_cast<BASE*>(this)->ptr; }
};
template<
    typename HANDLE_T,
    typename BASE,
    bool CONSTSAFE>
struct ConversionHandler<HANDLE_T, BASE, CWrapperType::Explicit, CONSTSAFE>
    : public ArrowHandler<HANDLE_T, BASE, CONSTSAFE>
{
    explicit operator HANDLE_T() const
    {   return static_cast<BASE*>(this)->ptr; }
};
template<
    typename HANDLE_T,
    typename BASE,
    bool CONSTSAFE>
struct ConversionHandler<HANDLE_T, BASE, CWrapperType::Get, CONSTSAFE>
    : public ArrowHandler<HANDLE_T, BASE, CONSTSAFE>
{
    HANDLE_T get() const
    {   return static_cast<BASE*>(this)->ptr; }
};

template<
    typename PTR_T,
    typename BASE>
struct ConversionHandler<PTR_T*, BASE, CWrapperType::Implicit, true>
    : public ArrowHandler<PTR_T*, BASE, true>
{
    operator PTR_T*()
    {   return static_cast<BASE*>(this)->ptr; }
    operator PTR_T const*() const
    {   return static_cast<BASE*>(this)->ptr; }
};
template<
    typename PTR_T,
    typename BASE>
struct ConversionHandler<PTR_T*, BASE, CWrapperType::Explicit, true>
    : public ArrowHandler<PTR_T*, BASE, true>
{
    explicit operator PTR_T*()
    {   return static_cast<BASE*>(this)->ptr; }
    explicit operator PTR_T const*() const
    {   return static_cast<BASE*>(this)->ptr; }
};
template<
    typename PTR_T,
    typename BASE>
struct ConversionHandler<PTR_T*, BASE, CWrapperType::Get, true>
    : public ArrowHandler<PTR_T*, BASE, true>
{
    PTR_T* get()
    {   return static_cast<BASE*>(this)->ptr; }
    PTR_T const* get() const
    {   return static_cast<BASE*>(this)->ptr; }
};

template<
    typename HANDLE_T,
    typename FUNCTIONS,
    CWrapperType TYPE,
    bool CONSTSAFE,
    typename EXCEPTION_T,
    typename INVALID_T,
    typename VALIDATE_T>
class CWrapperBase
    : public ConversionHandler<
        HANDLE_T,
        CWrapperBase<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T, INVALID_T, VALIDATE_T>,
        TYPE,
        CONSTSAFE>
{
    friend class ConversionHandler<
        HANDLE_T,
        CWrapperBase<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T, INVALID_T, VALIDATE_T>,
        TYPE,
        CONSTSAFE>;
protected:
    HANDLE_T ptr;

public:

    explicit CWrapperBase(HANDLE_T ptr) :
        ptr{ptr}
    {
        if(!VALIDATE_T::validate_func(ptr))
            throw EXCEPTION_T{};
    }

    CWrapperBase(CWrapperBase const& other) :
        CWrapperBase{FUNCTIONS::copy_func(other.ptr)}
    { }

    // This one is needed to prevent variadic ctor to be called
    // when you want to copy a non-const object.
    CWrapperBase(CWrapperBase& other) :
        CWrapperBase{FUNCTIONS::copy_func(other.ptr)}
    { }

    template<typename... ARGS>
    explicit CWrapperBase(ARGS&&... args) :
        CWrapperBase{FUNCTIONS::ctor_func(std::forward<ARGS>(args)...)}
    { }

    CWrapperBase(CWrapperBase&& old) :
        CWrapperBase{old.ptr}
    {
        old.ptr = INVALID_T::invalid_value;
    }

    CWrapperBase& operator=(CWrapperBase&& old)
    {
        if(this != &old)
        {
            FUNCTIONS::dtor_func(ptr);
            ptr = old.ptr;
            old.ptr = INVALID_T::invalid_value;
        }
        return *this;
    }

    ~CWrapperBase()
    {
        FUNCTIONS::dtor_func(ptr);
    }

    CWrapperBase& operator=(CWrapperBase const& other)
    {
        if(this != &other)
        {
            HANDLE_T new_ptr = FUNCTIONS::copy_func(other.ptr);
            if(!VALIDATE_T::validate_func(new_ptr))
                throw EXCEPTION_T{};

            FUNCTIONS::dtor_func(ptr);
            ptr = new_ptr;
        }
        return *this;
    }
};

template<
    typename HANDLE_T,
    typename FUNCTIONS,
    CWrapperType TYPE,
    bool CONSTSAFE,
    typename EXCEPTION_T,
    typename INVALID_T,
    typename VALIDATE_T>
class CWrapperNonCopiable
    : public ConversionHandler<
        HANDLE_T,
        CWrapperNonCopiable<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T, INVALID_T, VALIDATE_T>,
        TYPE,
        CONSTSAFE>
{
    friend class ConversionHandler<
        HANDLE_T,
        CWrapperNonCopiable<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T, INVALID_T, VALIDATE_T>,
        TYPE,
        CONSTSAFE>;

protected:
    HANDLE_T ptr;

public:

    explicit CWrapperNonCopiable(HANDLE_T ptr) :
        ptr{ptr}
    {
        if(!VALIDATE_T::validate_func(ptr))
            throw EXCEPTION_T{};
    }

    template<typename... ARGS>
    explicit CWrapperNonCopiable(ARGS&&... args) :
        CWrapperNonCopiable{FUNCTIONS::ctor_func(std::forward<ARGS>(args)...)}
    { }

    CWrapperNonCopiable(CWrapperNonCopiable&& old) :
        CWrapperNonCopiable{old.ptr}
    {
        old.ptr = INVALID_T::invalid_value;
    }

    CWrapperNonCopiable& operator=(CWrapperNonCopiable&& old)
    {
        if(this != &old)
        {
            FUNCTIONS::dtor_func(ptr);
            ptr = old.ptr;
            old.ptr = INVALID_T::invalid_value;
        }
        return *this;
    }

    ~CWrapperNonCopiable()
    {
        FUNCTIONS::dtor_func(ptr);
    }

    // This one is needed to prevent variadic ctor to be called
    // when you want to copy a non-const object.
    CWrapperNonCopiable(CWrapperNonCopiable& other) = delete;
    CWrapperNonCopiable(CWrapperNonCopiable const& other) = delete;
    CWrapperNonCopiable& operator=(CWrapperNonCopiable const& other) = delete;
};

template<bool condition, typename TRUE_TYPE, typename FALSE_TYPE>
struct COND
{
    using type = TRUE_TYPE;
};

template<typename TRUE_TYPE, typename FALSE_TYPE>
struct COND<false, TRUE_TYPE, FALSE_TYPE>
{
    using type = FALSE_TYPE;
};

HAS_NESTED_TYPE_DETECTOR(exception);
HAS_STATIC_MEMBER_DETECTOR(copy_func);
HAS_STATIC_MEMBER_DETECTOR(invalid_value);
HAS_STATIC_MEMBER_DETECTOR(validate_func);

template<typename TYPE, typename DEFTYPE, bool>
struct default_exception_type
{
    using type = DEFTYPE;
};

template<typename TYPE, typename DEFTYPE>
struct default_exception_type<TYPE, DEFTYPE, true>
{
    using type = typename TYPE::exception;
};

template<typename T>
struct default_invalid_value
{
    static constexpr T invalid_value = 0;
};
template<typename T>
struct default_invalid_value<T*>
{
    static constexpr T* invalid_value = nullptr;
};

template<typename T, typename INVALID_T>
struct default_validate_func
{
    static constexpr bool validate_func(T ptr)
    {
        return ptr != INVALID_T::invalid_value;
    }
};

using E = typename default_exception_type<F, std::bad_alloc, HAS_NESTED_TYPE(F, exception)>::type;
using D = typename COND< HAS_STATIC_MEMBER(F, invalid_value),
    F, default_invalid_value<H>>::type;
using V = typename COND< HAS_STATIC_MEMBER(F, validate_func),
    F, default_validate_func<H, D>>::type;

public:

    using type = typename COND< HAS_STATIC_MEMBER(F, copy_func),
        CWrapperBase<H, F, TY, C, E, D, V>,
        CWrapperNonCopiable<H, F, TY, C, E, D, V>>::type;
};



template<
    typename HANDLE_T,
    typename FUNCTIONS,
    CWrapperType TYPE = CWrapperType::Get,
    bool CONSTSAFE = true>
using CWrapper = typename __CWrapperHelper__<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE>::type;

#endif // CWRAPPER_HPP_INCLUDED
