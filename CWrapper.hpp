#ifndef CWRAPPER_HPP_INCLUDED
#define CWRAPPER_HPP_INCLUDED

enum class CWrapperType
{
    Implicit,
    Explicit,
    Get,
};

template<
    typename HANDLE_T,
    typename FUNCTIONS,
    typename EXCEPTION_T = std::bad_alloc>
class CWrapperBase
{
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

    operator HANDLE_T() const
    {
        return ptr;
    }

    HANDLE_T operator->() const
    {
        return ptr;
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
    typename EXCEPTION_T = std::bad_alloc>
class CWrapperCopiable : protected CWrapperBase<HANDLE_T, FUNCTIONS, EXCEPTION_T>
{
    using base = CWrapperBase<HANDLE_T, FUNCTIONS, EXCEPTION_T>;

public:

    using base::CWrapperBase;
    using base::operator HANDLE_T;
    using base::operator->;

    CWrapperCopiable(CWrapperCopiable const& other) :
        base{FUNCTIONS::copy_func(other.ptr)}
    { }

    // This one is needed to prevent variadic ctor to be called
    // when you want to copy a non-const object.
    CWrapperCopiable(CWrapperCopiable& other) :
        base{FUNCTIONS::copy_func(other.ptr)}
    { }

    CWrapperCopiable& operator=(CWrapperCopiable const& other)
    {
        if(this != &other)
        {
            HANDLE_T new_ptr = FUNCTIONS::copy_func(other.ptr);
            if(new_ptr == nullptr)
                throw EXCEPTION_T{};

            FUNCTIONS::dtor_func(base::ptr);
            base::ptr = new_ptr;
        }
        return *this;
    }
};

template<
    typename HANDLE_T,
    typename FUNCTIONS,
    typename EXCEPTION_T = std::bad_alloc>
class CWrapperNonCopiable : protected CWrapperBase<HANDLE_T, FUNCTIONS, EXCEPTION_T>
{
    using base = CWrapperBase<HANDLE_T, FUNCTIONS, EXCEPTION_T>;

protected:
    HANDLE_T ptr;
public:

    using base::CWrapperBase;
    using base::operator HANDLE_T;
    using base::operator->;

    // This one is needed to prevent variadic ctor to be called
    // when you want to copy a non-const object.
    CWrapperNonCopiable(CWrapperNonCopiable& other) = delete;
    CWrapperNonCopiable(CWrapperNonCopiable const& other) = delete;
    CWrapperNonCopiable& operator=(CWrapperNonCopiable const& other) = delete;
};

template<
    typename H,
    typename F,
    typename E>
class __CWrapperHelper__
{
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
    static void has_copy_helper(...)
    {
    }

    template <typename FUNCTIONS>
    static int has_copy_helper(int, decltype(FUNCTIONS::copy_func)* func = nullptr)
    {
        return 0;
    }

    template<typename FUNCTIONS>
    struct has_copy
    {
        static constexpr bool value =
            EQ<decltype(has_copy_helper<FUNCTIONS>(0)), int>::value;
    };

public:

    using type = typename COND<has_copy<F>::value,
        CWrapperCopiable<H, F, E>,
        CWrapperNonCopiable<H, F, E>>::type;
};

template<
    typename HANDLE_T,
    typename FUNCTIONS,
    typename EXCEPTION_T = std::bad_alloc>
using CWrapper = typename __CWrapperHelper__<HANDLE_T, FUNCTIONS, EXCEPTION_T>::type;


#endif // CWRAPPER_HPP_INCLUDED
