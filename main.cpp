#include <iostream>
#include <cstdio>
#include "CWrapper.hpp"

FILE * copy_file(const FILE *)
{
    return nullptr;
}

struct FileWrapperBasic
{
    static constexpr auto ctor_func = fopen;
    static constexpr auto dtor_func = fclose;
    static constexpr auto copy_func = copy_file;
};

struct FileWrapperAdvanced
{
    static FILE* ctor_func(const char* filename)
    {
        return fopen(filename, "wt");
    }
    static FILE* ctor_func(const char* filename, const char* mode)
    {
        return fopen(filename, mode);
    }

    static constexpr auto dtor_func = fclose;
};

int main()
{
    using FileWrapper = CWrapper<FILE*, FileWrapperBasic>;

    FileWrapper file("whatever.txt", "wt");
    fprintf(file.get(), "Hello Unified CWrapper!");

    FileWrapper file2(file);
    (void)file2;

    return 0;
}
