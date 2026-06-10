#include <future>
#include <iostream>
#include <stdexcept>

// This triggers __cxa_init_primary_exception / __from_native_exception_pointer
// which are present in libc++ 18 headers but missing from Apple's system libc++.
// Reproduces: https://github.com/llvm/llvm-project/issues/86077

int main()
{
    std::promise<int> p;
    auto f = p.get_future();

    try {
        p.set_exception(std::make_exception_ptr(std::runtime_error("test")));
        f.get();
    } catch (const std::runtime_error& e) {
        std::cout << "Caught: " << e.what() << std::endl;
    }

    return 0;
}
