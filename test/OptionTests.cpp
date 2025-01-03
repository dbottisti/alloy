#include <type_traits>
#include <utility>

namespace alloy {

// Forward declarations
template <typename T>
class Option;

namespace detail {

struct None {};

template <typename T>
class Some {
public:
    Some(T value) : value_{std::move(value)} {}

private:
    friend class Option<T>;

    template <typename U>
    friend class Option;
    T value_;
};

}  // namespace detail

template <typename T>
class Option {
public:
    explicit Option(detail::None) : value_{}, is_some_{false} {}

    template <typename U,
              typename = std::enable_if_t<std::is_convertible<U, T>::value>>
    explicit Option(detail::Some<U> some)
        : value_{std::move(some.value_)}, is_some_{true} {}

    bool is_some() const { return is_some_; }

private:
    T value_;
    bool is_some_ = false;
};

// Helper functions
template <typename T>
typename detail::Some<T> Some(const T& value) {
    return {value};
}

static constexpr detail::None None;

}  // namespace alloy

//------------------------------------------------------------------------------

#include <cstdio>

#include <catch2/catch_test_macros.hpp>

SCENARIO("Option") {
    using namespace alloy;

    GIVEN("An empty Option") {
        Option<std::uint32_t> option{None};

        WHEN("calling is_some") {
            const auto result = option.is_some();
            THEN("the result is false") { CHECK(result == false); }
        }
    }

    GIVEN("An Option with a value") {
        Option<std::uint32_t> option{Some(42u)};

        WHEN("calling is_some") {
            const auto result = option.is_some();
            THEN("the result is true") { CHECK(result == true); }
        }
    }
}
