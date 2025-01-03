#include <utility>

namespace alloy {
namespace detail {

template <typename Function, typename... Args>
struct is_callable {
private:
    template <typename Function2, typename... Args2>
    static constexpr std::true_type test(
        decltype(std::declval<Function2>()(std::declval<Args2>()...))*) {
        return {};
    }

    template <typename Function2, typename... Args2>
    static constexpr std::false_type test(...) {
        return {};
    }

public:
    static constexpr bool value
        = decltype(test<Function, Args...>(nullptr))::value;
};

}  // namespace detail

}  // namespace alloy
