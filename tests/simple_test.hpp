#ifndef SIMPLE_TEST_HPP
#define SIMPLE_TEST_HPP

// A minimal, header-only test framework with no external dependencies.
//
// Usage:
//   TEST(my_test_name) {
//       CHECK(some_condition);
//       CHECK_EQ(actual, expected);
//   }
//
//   // Provide main() in exactly one translation unit:
//   SIMPLE_TEST_MAIN()
//
// Running the resulting executable runs every registered test and returns a
// non-zero exit code if any check fails.

#include <cstddef>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace simple_test {

struct TestCase {
    std::string name;
    std::function<void(int&)> fn;  // increments the failure counter on a failed check
};

/// Global registry of tests (function-local static to avoid init-order issues).
inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

/// Helper that registers a test at static-initialization time.
struct Registrar {
    Registrar(const std::string& name, std::function<void(int&)> fn) {
        registry().push_back(TestCase{name, std::move(fn)});
    }
};

/// Runs all registered tests. Returns 0 on success, 1 if any test failed.
inline int run_all() {
    std::size_t passed = 0;
    std::size_t failed = 0;
    for (const auto& test : registry()) {
        int failures = 0;
        test.fn(failures);
        if (failures == 0) {
            std::cout << "[ PASS ] " << test.name << "\n";
            ++passed;
        } else {
            std::cout << "[ FAIL ] " << test.name << " (" << failures
                      << " check(s) failed)\n";
            ++failed;
        }
    }
    std::cout << "\n"
              << (failed == 0 ? "All tests passed" : "Some tests failed")
              << ": " << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}

}  // namespace simple_test

// Defines and registers a test body. The body has access to `__failures`
// indirectly through the CHECK macros.
#define TEST(test_name)                                                        \
    static void test_name(int& __failures);                                    \
    static ::simple_test::Registrar __registrar_##test_name(#test_name,        \
                                                            test_name);        \
    static void test_name(int& __failures)

// Asserts that `expr` is true.
#define CHECK(expr)                                                            \
    do {                                                                       \
        if (!(expr)) {                                                         \
            ++__failures;                                                      \
            std::cout << "    CHECK failed: " << #expr << "\n"                  \
                      << "      at " << __FILE__ << ":" << __LINE__ << "\n";    \
        }                                                                      \
    } while (false)

// Asserts that `actual == expected`, printing both values on failure.
#define CHECK_EQ(actual, expected)                                             \
    do {                                                                       \
        const auto __a = (actual);                                             \
        const auto __e = (expected);                                           \
        if (!(__a == __e)) {                                                   \
            ++__failures;                                                      \
            std::ostringstream __oss;                                          \
            __oss << "    CHECK_EQ failed: " << #actual << " == " << #expected \
                  << "\n      actual:   " << __a << "\n      expected: " << __e \
                  << "\n      at " << __FILE__ << ":" << __LINE__ << "\n";      \
            std::cout << __oss.str();                                          \
        }                                                                      \
    } while (false)

// Emits a main() that runs all registered tests.
#define SIMPLE_TEST_MAIN()                                                     \
    int main() { return ::simple_test::run_all(); }

#endif  // SIMPLE_TEST_HPP
