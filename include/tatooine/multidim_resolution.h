#ifndef TATOOINE_MULTIDIM_RESOLUTION_H
#define TATOOINE_MULTIDIM_RESOLUTION_H

#include <array>
#include <cassert>
#include <numeric>
#include <sstream>

#include "functional.h"
#include "index_ordering.h"
#include "multidim.h"
#include "template_helper.h"
#include "type_traits.h"
#include "utility.h"

//==============================================================================
namespace tatooine {
//==============================================================================
template <typename Indexing, size_t... Resolution>
struct static_multidim_resolution {
  static constexpr size_t num_dimensions() { return sizeof...(Resolution); }
#if has_cxx17_support()
  //----------------------------------------------------------------------------
  static constexpr size_t num_elements() { return (Resolution * ...); }
#else
  //----------------------------------------------------------------------------
  static constexpr size_t num_elements() {
    constexpr auto res = resolution();
    return std::accumulate(begin(res), end(res), size_t(1),
                           std::multiplies<size_t>{});
  }
#endif
  //----------------------------------------------------------------------------
  static constexpr auto resolution() {
    return std::array<size_t, num_dimensions()>{Resolution...};
  }
  //----------------------------------------------------------------------------
  static constexpr auto size(size_t i) { return resolution()[i]; }
  //----------------------------------------------------------------------------
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr bool in_range(Is... is) {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
#if has_cxx17_support()
    return ((is >= 0) && ...) &&
           ((static_cast<size_t>(is) < Resolution) && ...);
#else
    const std::array<size_t, N> is{static_cast<size_t>(is)...};
    for (size_t i = 0; i < N; ++i) {
      if (is[i] < 0 || is[i] >= resolution(i)) { return false; }
    }
    return true;
#endif
  }
  // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
  template <size_t N>
  static constexpr auto in_range(const std::array<size_t, N>& is) {
    return invoke_unpacked([](auto... is) { return in_range(is...); },
                           unpack(is));
  }

  //----------------------------------------------------------------------------
#ifndef NDEBUG
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  static auto plain_idx(Is... is) {
    if (!in_range(std::forward<Is>(is)...)) {
      std::stringstream ss;
      ss << "is out of bounds: [ ";
      for (auto i :
           std::array<size_t, sizeof...(Is)>{static_cast<size_t>(is)...}) {
        ss << std::to_string(i) + " ";
      }
      ss << "]\n";
      throw std::runtime_error{ss.str()};
    }
#else
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr auto plain_idx(Is... is) {
#endif
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return Indexing::plain_idx(
        std::array<size_t, num_dimensions()>{Resolution...}, is...);
  }
  // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
  template <size_t N>
  static constexpr auto plain_idx(const std::array<size_t, N>& is) {
    return invoke_unpacked([](auto... is) { return plain_idx(is...); },
                           unpack(is));
  }
  //----------------------------------------------------------------------------
  static constexpr auto indices() { return static_multidim{Resolution...}; }
};
//==============================================================================
template <typename Indexing>
class dynamic_multidim_resolution {
  //----------------------------------------------------------------------------
  // members
  //----------------------------------------------------------------------------
  std::vector<size_t> m_resolution;

  //----------------------------------------------------------------------------
  // ctors
  //----------------------------------------------------------------------------
 public:
  dynamic_multidim_resolution() = default;
  dynamic_multidim_resolution(const dynamic_multidim_resolution& other) =
      default;
  dynamic_multidim_resolution(dynamic_multidim_resolution&& other) = default;
  dynamic_multidim_resolution& operator                            =(
      const dynamic_multidim_resolution& other) = default;
  dynamic_multidim_resolution& operator=(dynamic_multidim_resolution&& other) =
      default;
  //----------------------------------------------------------------------------
  template <typename OtherIndexing>
  dynamic_multidim_resolution(
      const dynamic_multidim_resolution<OtherIndexing>& other)
      : m_resolution{other.resolution()} {}
  template <typename OtherIndexing>
  dynamic_multidim_resolution(
      dynamic_multidim_resolution<OtherIndexing>&& other)
      : m_resolution{std::move(other.m_resolution)} {}
  template <typename OtherIndexing>
  dynamic_multidim_resolution& operator=(
      const dynamic_multidim_resolution& other) {
    m_resolution = other.m_resolution;
    return *this;
  }
  template <typename OtherIndexing>
  dynamic_multidim_resolution& operator=(dynamic_multidim_resolution&& other) {
    m_resolution = std::move(other.m_resolution);
    return *this;
  }
  //----------------------------------------------------------------------------
  template <typename... Resolution,
            enable_if_integral<std::decay_t<Resolution>...> = true>
  dynamic_multidim_resolution(Resolution... resolution)
      : m_resolution{static_cast<size_t>(resolution)...} {}
  //----------------------------------------------------------------------------
  dynamic_multidim_resolution(const std::vector<size_t>& resolution)
      : m_resolution(resolution) {}
  //----------------------------------------------------------------------------
  dynamic_multidim_resolution(std::vector<size_t>&& resolution)
      : m_resolution(std::move(resolution)) {}

  //----------------------------------------------------------------------------
  // comparisons
  //----------------------------------------------------------------------------
 public:
  template <typename OtherIndexing>
  bool operator==(
      const dynamic_multidim_resolution<OtherIndexing>& other) const {
    if (num_dimensions() != other.num_dimensions()) { return false; }
    for (size_t i = 0; i < num_dimensions(); ++i) {
      if (m_resolution[i] != other.size(i)) { return false; }
    }
    return true;
  }
  //----------------------------------------------------------------------------
  template <typename OtherIndexing>
  bool operator!=(
      const dynamic_multidim_resolution<OtherIndexing>& other) const {
    if (num_dimensions() == other.num_dimensions()) { return false; }
    for (size_t i = 0; i < num_dimensions(); ++i) {
      if (m_resolution[i] == other.size(i)) { return false; }
    }
    return true;
  }

  //----------------------------------------------------------------------------
  // methods
  //----------------------------------------------------------------------------
 public:
  size_t num_dimensions() const { return m_resolution.size(); }
  //----------------------------------------------------------------------------
  const auto& resolution() const { return m_resolution; }
  /// \return size of dimensions i
  auto size(const size_t i) const { return m_resolution[i]; }
  //----------------------------------------------------------------------------
  size_t num_elements() const {
    return std::accumulate(begin(m_resolution), end(m_resolution), size_t(1),
                           std::multiplies<size_t>{});
  }
  //----------------------------------------------------------------------------
  template <typename... Resolution,
            enable_if_integral<std::decay_t<Resolution>...> = true>
  void resize(Resolution... resolution) {
    m_resolution = {static_cast<size_t>(resolution)...};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  void resize(const std::array<size_t, N>& resolution) {
    m_resolution = std::vector(begin(resolution), end(resolution));
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void resize(std::vector<size_t>&& resolution) {
    m_resolution = std::move(resolution);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void resize(const std::vector<size_t>& resolution) {
    m_resolution = resolution;
  }
  //----------------------------------------------------------------------------
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  constexpr auto in_range(Is... is) const {
    assert(sizeof...(Is) == num_dimensions());
    constexpr size_t            N = sizeof...(is);
    const std::array<size_t, N> arr_is{static_cast<size_t>(is)...};
    for (size_t i = 0; i < N; ++i) {
      if (arr_is[i] < 0 || arr_is[i] >= size(i)) { return false; }
    }
    return true;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  constexpr auto in_range(const std::array<size_t, N>& is) const {
    assert(N == num_dimensions());
    return invoke_unpacked([this](auto... is) { return in_range(is...); },
                           unpack(is));
  }
  //----------------------------------------------------------------------------
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  constexpr auto plain_idx(Is... is) const {
    assert(sizeof...(Is) == num_dimensions());
    assert(in_range(is...));
    return Indexing::plain_idx(m_resolution, is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr auto plain_idx(const std::vector<size_t>& is) const {
    assert(is.size() == num_dimensions());
    assert(in_range(is));
    return Indexing::plain_idx(m_resolution, is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  constexpr auto plain_idx(const std::array<size_t, N>& is) const {
    assert(N == num_dimensions());
    return invoke_unpacked([this](auto... is) { return plain_idx(is...); },
                           unpack(is));
  }
  //----------------------------------------------------------------------------
  constexpr auto multi_index(size_t gi) const {
    return Indexing::multi_index(m_resolution, gi);
  }
  //----------------------------------------------------------------------------
  constexpr auto indices() const { return dynamic_multidim{m_resolution}; }
};

//==============================================================================
// deduction guides
//==============================================================================
#if has_cxx17_support()
dynamic_multidim_resolution()->dynamic_multidim_resolution<x_fastest>;

template <typename Indexing>
dynamic_multidim_resolution(const dynamic_multidim_resolution<Indexing>&)
    ->dynamic_multidim_resolution<Indexing>;
template <typename Indexing>
dynamic_multidim_resolution(dynamic_multidim_resolution<Indexing> &&)
    ->dynamic_multidim_resolution<Indexing>;
template <typename... Resolution>
dynamic_multidim_resolution(Resolution...)
    ->dynamic_multidim_resolution<x_fastest>;
#endif

//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
