#ifndef TATOOINE_TRANSPOSED_TENSOR_H
#define TATOOINE_TRANSPOSED_TENSOR_H
//==============================================================================
#include <tatooine/base_tensor.h>
#include <tatooine/concepts.h>
#include <tatooine/invoke_reversed.h>
//==============================================================================
namespace tatooine {
//==============================================================================
template <static_tensor Tensor>
struct transposed_static_tensor {
  static auto constexpr rank() { std::decay_t<Tensor>::rank(); }
  static auto constexpr dimensions() {
    return std::ranges::reverse(std::decay_t<Tensor>::dimensions());
  }
  static auto constexpr dimension(std::size_t const i) {
    return std::decay_t<Tensor>::dimension(rank() - i - 1);
  }
  static auto constexpr is_tensor() { return true; }
  static auto constexpr is_static() { return true; }
  static auto constexpr is_transposed() { return true; }
  using value_type = typename std::decay_t<Tensor>::value_type;
  //============================================================================
 private:
  Tensor m_internal_tensor;

  //============================================================================
 public:
  constexpr explicit transposed_static_tensor(static_tensor auto&& t)
      : m_internal_tensor{std::forward<decltype(t)>(t)} {}
  //----------------------------------------------------------------------------
  auto constexpr at(integral auto const... is) const -> decltype(auto) {
    return invoke_reversed(
        [this](auto const... is) -> decltype(auto) {
          return internal_tensor()(is...);
        },
        is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr at(integral auto const... is) -> decltype(auto) {
    return invoke_reversed(
        [this](auto const... is) -> decltype(auto) {
          return internal_tensor()(is...);
        },
        is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral auto const... is) const -> decltype(auto) {
    return at(is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral auto const... is) -> decltype(auto) {
    return at(is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr at(integral_range auto is) const -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr at(integral_range auto is) -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral_range auto is) const -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral_range auto is) -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
  //----------------------------------------------------------------------------
  auto internal_tensor() const -> auto const& { return internal_tensor(); }
};
//------------------------------------------------------------------------------
template <static_tensor T>
transposed_static_tensor(T&&) -> transposed_static_tensor<std::decay_t<T>>;
template <static_tensor T>
transposed_static_tensor(T&) -> transposed_static_tensor<T&>;
template <static_tensor T>
transposed_static_tensor(T const&) -> transposed_static_tensor<T const&>;
//==============================================================================
// dynamic tensor
//==============================================================================
template <dynamic_tensor Tensor>
struct transposed_dynamic_tensor {
  static auto constexpr is_tensor() { return true; }
  static auto constexpr is_transposed() { return true; }
  static auto constexpr is_dynamic() { return true; }
  using value_type = typename std::decay_t<Tensor>::value_type;
  //============================================================================
  Tensor const& m_internal_tensor;
  //============================================================================
  transposed_dynamic_tensor(dynamic_tensor auto&& t)
      : m_internal_tensor{std::forward<decltype(t)>(t)} {}
  //============================================================================
  auto internal_tensor() -> auto& { return m_internal_tensor; }
  auto internal_tensor() const -> auto const& { return m_internal_tensor; }
  //------------------------------------------------------------------------------
  auto constexpr rank() const { return internal_tensor().rank(); }
  auto constexpr dimensions() const {
    auto s = internal_tensor().dimensions();
    std::ranges::reverse(s);
    return s;
  }
  auto constexpr dimension(std::size_t const i) const {
    return internal_tensor().dimension(rank() - i - 1);
  }
  //============================================================================
  auto constexpr at(integral auto const... is) const -> decltype(auto) {
    return invoke_reversed(
        [this](auto const... is) -> decltype(auto) {
          return internal_tensor()(is...);
        },
        is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr at(integral auto const... is) -> decltype(auto) {
    return invoke_reversed(
        [this](auto const... is) -> decltype(auto) {
          return internal_tensor()(is...);
        },
        is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral auto const... is) const -> decltype(auto) {
    return at(is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral auto const... is) -> decltype(auto) {
    return at(is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr at(integral_range auto is) const -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr at(integral_range auto is) -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral_range auto is) const -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto constexpr operator()(integral_range auto is) -> decltype(auto) {
    std::ranges::reverse(is);
    return internal_tensor()(is);
  }
};
//------------------------------------------------------------------------------
template <dynamic_tensor T>
transposed_dynamic_tensor(T&&) -> transposed_dynamic_tensor<std::decay_t<T>>;
template <dynamic_tensor T>
transposed_dynamic_tensor(T&) -> transposed_dynamic_tensor<T&>;
template <dynamic_tensor T>
transposed_dynamic_tensor(T const&) -> transposed_dynamic_tensor<T const&>;
//==============================================================================
auto transposed(dynamic_tensor auto&& t) {
  return transposed_dynamic_tensor{std::forward<decltype(t)>(t)};
}
//------------------------------------------------------------------------------
auto constexpr transposed(static_tensor auto&& t) {
  return transposed_static_tensor{std::forward<decltype(t)>(t)};
}
//------------------------------------------------------------------------------
auto constexpr transposed(transposed_tensor auto&& t) {
  return t.internal_tensor();
}
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
