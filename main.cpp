#include <iostream>
#include <cstdio>
#include <cstring>
#include "CWrapper.hpp"

class stdc_error : public std::exception
{
    int error_number = errno;
public:
    virtual const char * what() const noexcept override
    {
        return strerror(error_number);
    }
};

// this one is here only to test detecting copy_func.
//FILE * copy_file(const FILE *)
//{
//    return nullptr;
//}

// This controller class is enough to provide
// RAII, move semantics, .get() and throws bad_alloc.
struct FileWrapperBasic
{
    static constexpr auto ctor_func = fopen;
    static constexpr auto dtor_func = fclose;
};

// This class shows the flexibility of CWrapper,
// using all the features of controller class.
class FileWrapperAdvanced
{
    friend CWrapperFriend<FILE*, FileWrapperAdvanced>;

    static FILE* ctor_func(const char* filename)
    {
        return fopen(filename, "rt");
    }
    static FILE* ctor_func(const char* filename, const char* mode)
    {
        return fopen(filename, mode);
    }

    static constexpr auto dtor_func = fclose;
//    static constexpr auto copy_func = copy_file;
    using exception = stdc_error;
    static constexpr FILE* invalid_value = nullptr;
    static bool validate_func(FILE* ptr)
    {
        std::cout << "Validating" << std::endl;
        return ptr != nullptr;
    }
};

int main() try
{
    using FileWrapper = CWrapper<FILE*, FileWrapperAdvanced>;

    FileWrapper file("you/shall/not/path", "rt");
    fprintf(file.get(), "Hello Unified CWrapper!");

//    FileWrapper file2(file);
//    (void)file2;

    return 0;
}
catch(std::exception& x)
{
    std::cout << x.what();
}
