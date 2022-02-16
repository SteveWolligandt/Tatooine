#ifndef TATOOINE_TAGS_H
#define TATOOINE_TAGS_H
//==============================================================================
#include <concepts>
//==============================================================================
namespace tatooine {
//==============================================================================
struct forward_tag {};
static constexpr forward_tag forward;
struct backward_tag {};
static constexpr backward_tag backward;
template <typename T>
concept forward_or_backward_tag =
    (std::same_as<T, forward_tag>) || (std::same_as<T, backward_tag>);
//==============================================================================
}  // namespace tatooine
//==============================================================================
namespace tatooine::execution_policy {
//==============================================================================
struct parallel_t {};
static constexpr parallel_t parallel;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
struct sequential_t {};
static constexpr sequential_t sequential;
template <typename T>
concept policy = std::same_as<T, parallel_t> || std::same_as<T, sequential_t>;
//==============================================================================
}  // namespace tatooine::execution_policy
//==============================================================================
namespace tatooine::tag {
//==============================================================================
struct frobenius_t {};
static constexpr frobenius_t frobenius;
struct full_t {};
static constexpr full_t full;
struct economy_t {};
static constexpr economy_t economy;
struct eye_t {};
static constexpr eye_t eye;
struct automatic_t {};
static constexpr automatic_t automatic;
struct central_t {};
static constexpr central_t central;
struct quadratic_t {};
static constexpr quadratic_t quadratic;
struct analytical_t {};
static constexpr analytical_t analytical;
struct numerical_t {};
static constexpr numerical_t numerical;
struct heap {};
struct stack {};

template <typename Real>
struct fill {
  Real value;
};
template <typename Real>
fill(Real) -> fill<Real>;

struct zeros_t {};
static constexpr zeros_t zeros;

struct ones_t {};
static constexpr ones_t ones;
//==============================================================================
}  // namespace tatooine::tag
//==============================================================================
#endif
