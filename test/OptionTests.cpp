#include <type_traits>
#include <utility>

#include "detail/is_callable.hpp"

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
    ~Option() {
        if (is_some()) {
            value_.~T();
        }
    }
    Option(const Option&) noexcept = default;
    Option(Option&&) noexcept = default;

    Option& operator=(const Option& other) noexcept {
        if (is_some() && other.is_some()) {
            value_ = other.value_;
        } else if (is_some()) {
            // other is None
            value_.~T();
            filler_ = true;
        } else if (other.is_some()) {
            // *this is None
            new (&value_) T{other.value_};
        }

        tag_ = other.tag_;
        return *this;
    }
    Option& operator=(Option&& other) noexcept {
        if (is_some() && other.is_some()) {
            value_ = std::move(other.value_);
        } else if (is_some()) {
            // other is None
            value_.~T();
            filler_ = true;
        } else if (other.is_some()) {
            // *this is None
            new (&value_) T{std::move(other.value_)};
        }

        tag_ = other.tag_;
        return *this;
    }

    explicit Option(detail::None) {}

    template <typename U,
              typename = std::enable_if_t<std::is_convertible<U, T>::value>>
    explicit Option(detail::Some<U> some) : tag_{Tag::SOME} {
        new (&value_) T{std::move(some.value_)};
    }

    bool is_some() const { return tag_ == Tag::SOME; }
    template <typename Function>
    bool is_some_and(Function f) const {
        static_assert(detail::is_callable<Function, T>::value,
                      "Function must be callable with an object of type T");
        return is_some() && f(value_);
    }

    bool is_none() const { return tag_ == Tag::NONE; }
    template <typename Function>
    bool is_none_or(Function f) const {
        static_assert(detail::is_callable<Function, T>::value,
                      "Function must be callable with an object of type T");
        return is_none() || f(value_);
    }

private:
    enum class Tag { NONE, SOME };
    union {
        bool filler_ = true;
        T value_;
    };
    Tag tag_ = Tag::NONE;
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

class NotDefaultConstructible {
    explicit NotDefaultConstructible(std::int32_t) {}
};

TEST_CASE(
    "An option can be constructed from a non-default constructible type") {
    using namespace alloy;
    Option<NotDefaultConstructible> option{None};
    (void)option;
}

SCENARIO("Option") {
    using namespace alloy;

    const auto greater_than_one = [](const auto x) { return x > 1; };

    GIVEN("an empty Option") {
        Option<std::uint32_t> option{None};

        WHEN("calling is_some") {
            const auto result = option.is_some();
            THEN("the result is false") { CHECK(result == false); }
        }

        WHEN("calling is_some_and") {
            const auto result = option.is_some_and(greater_than_one);
            THEN("the result is false") { CHECK(result == false); }
        }

        WHEN("calling is_none") {
            const auto result = option.is_none();
            THEN("the result is true") { CHECK(result == true); }
        }

        WHEN("calling is_none_or") {
            const auto result = option.is_none_or(greater_than_one);
            THEN("the result is true") { CHECK(result == true); }
        }
    }

    GIVEN("an Option with a value greater than 1") {
        Option<std::uint32_t> option{Some(42u)};

        WHEN("calling is_some") {
            const auto result = option.is_some();
            THEN("the result is true") { CHECK(result == true); }
        }

        WHEN("calling is_some_and") {
            const auto result = option.is_some_and(greater_than_one);
            THEN("the result is true") { CHECK(result == true); }
        }

        WHEN("calling is_none") {
            const auto result = option.is_none();
            THEN("the result is false") { CHECK(result == false); }
        }

        WHEN("calling is_none_or") {
            const auto result = option.is_none_or(greater_than_one);
            THEN("the result is true") { CHECK(result == true); }
        }
    }

    GIVEN("an Option with a zero value") {
        Option<std::uint32_t> option{Some(0u)};

        WHEN("calling is_some_and") {
            const auto result = option.is_some_and(greater_than_one);
            THEN("the result is false") { CHECK(result == false); }
        }

        WHEN("calling is_none_or") {
            const auto result = option.is_none_or(greater_than_one);
            THEN("the result is false") { CHECK(result == false); }
        }
    }
}
