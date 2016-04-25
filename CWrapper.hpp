#ifndef CWRAPPER_HPP_INCLUDED
#define CWRAPPER_HPP_INCLUDED

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
    bool C,
    typename E>
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
    typename EXCEPTION_T>
class CWrapperBase
    : public ConversionHandler<
        HANDLE_T,
        CWrapperBase<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T>,
        TYPE,
        CONSTSAFE>
{
    friend class ConversionHandler<
        HANDLE_T,
        CWrapperBase<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T>,
        TYPE,
        CONSTSAFE>;
protected:
    HANDLE_T ptr;

public:

    explicit CWrapperBase(HANDLE_T ptr) :
        ptr{ptr}
    {
        if(ptr == nullptr)
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
        old.ptr = nullptr;
    }

    CWrapperBase& operator=(CWrapperBase&& old)
    {
        if(this != &old)
        {
            FUNCTIONS::dtor_func(ptr);
            ptr = old.ptr;
            old.ptr = nullptr;
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
            if(new_ptr == nullptr)
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
    typename EXCEPTION_T>
class CWrapperNonCopiable
    : public ConversionHandler<
        HANDLE_T,
        CWrapperNonCopiable<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T>,
        TYPE,
        CONSTSAFE>
{
    friend class ConversionHandler<
        HANDLE_T,
        CWrapperNonCopiable<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T>,
        TYPE,
        CONSTSAFE>;

protected:
    HANDLE_T ptr;

public:

    explicit CWrapperNonCopiable(HANDLE_T ptr) :
        ptr{ptr}
    {
        if(ptr == nullptr)
            throw EXCEPTION_T{};
    }

    template<typename... ARGS>
    explicit CWrapperNonCopiable(ARGS&&... args) :
        CWrapperNonCopiable{FUNCTIONS::ctor_func(std::forward<ARGS>(args)...)}
    { }

    CWrapperNonCopiable(CWrapperNonCopiable&& old) :
        CWrapperNonCopiable{old.ptr}
    {
        old.ptr = nullptr;
    }

    CWrapperNonCopiable& operator=(CWrapperNonCopiable&& old)
    {
        if(this != &old)
        {
            FUNCTIONS::dtor_func(ptr);
            ptr = old.ptr;
            old.ptr = nullptr;
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


template<typename T1, typename T2>
struct EQ
{
    static constexpr bool value = false;
};

template<typename T>
struct EQ<T, T>
{
    static constexpr bool value = true;
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

template <typename FUNCTIONS>
static void has_copy_helper(...) { }

template <typename FUNCTIONS>
static int has_copy_helper(int, decltype(FUNCTIONS::copy_func)* func = nullptr) { return 0; }

public:

    using type = typename COND< EQ<decltype(has_copy_helper<F>(0)), int>::value,
        CWrapperBase<H, F, TY, C, E>,
        CWrapperNonCopiable<H, F, TY, C, E>>::type;
};

template<
    typename HANDLE_T,
    typename FUNCTIONS,
    CWrapperType TYPE = CWrapperType::Get,
    bool CONSTSAFE = true,
    typename EXCEPTION_T = std::bad_alloc>
using CWrapper = typename __CWrapperHelper__<HANDLE_T, FUNCTIONS, TYPE, CONSTSAFE, EXCEPTION_T>::type;

#endif // CWRAPPER_HPP_INCLUDED
