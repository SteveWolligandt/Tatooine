#ifndef TATOOINE_INDEX_ORDERING_H
#define TATOOINE_INDEX_ORDERING_H

#include <array>
#include <vector>

#include "type_traits.h"
#include "utility.h"

//==============================================================================
namespace tatooine {
//==============================================================================
/// converts multi-dimensional index to a one dimensional index where first
/// dimensions grows fastest
struct x_fastest {
  template <typename ResIt, typename... Is,
            enable_if_iterator<std::decay_t<ResIt>> = true>
  static constexpr size_t plain_idx(ResIt                      res_it,
                                    const std::vector<size_t>& is) {
    size_t multiplier = 1;
    size_t idx        = 0;
    for (auto i : is) {
      idx += i * multiplier;
      multiplier *= *res_it;
      ++res_it;
    }
    return idx;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename ResIt, typename... Is,
            enable_if_iterator<std::decay_t<ResIt>> = true,
            enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t plain_idx(ResIt res_it, Is... is) {
    size_t multiplier = 1;
    size_t idx        = 0;
    map([&](size_t i) {
          idx += i * multiplier;
          multiplier *= *res_it;
          ++res_it;
        }, is...);
    return idx;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N, typename... Is,
            enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t plain_idx(const std::array<size_t, N>& resolution,
                                    Is... is) {
    return plain_idx(begin(resolution), is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  static constexpr size_t plain_idx(const std::array<size_t, N>& resolution,
                                    const std::vector<size_t>& is) {
    return plain_idx(begin(resolution), is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t plain_idx(const std::vector<size_t>& resolution,
                                    Is... is) {
    return plain_idx(begin(resolution), is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  static constexpr size_t plain_idx(const std::vector<size_t>& resolution,
                                    const std::vector<size_t>& is) {
    return plain_idx(begin(resolution), is);
  }
  //----------------------------------------------------------------------------
  static auto multi_index(const std::vector<size_t>& resolution,
                          size_t                     plain_idx) {
    std::vector<size_t> is(resolution.size());
    size_t              multiplier =
        std::accumulate(begin(resolution), std::prev(end(resolution)),
                        size_t(1), std::multiplies<size_t>{});

    auto res_it = std::prev(end(resolution), 2);
    for (size_t j = 0; j < resolution.size(); ++j, --res_it) {
      size_t i = resolution.size() - 1 - j;
      is[i]    = plain_idx / multiplier;
      plain_idx -= is[i] * multiplier;
      if (res_it >= begin(resolution)) { multiplier /= *res_it; }
    }
    return is;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  static constexpr auto multi_index(const std::array<size_t, N>& resolution,
                                    size_t                       plain_idx) {
    auto   is = make_array<size_t, N>();
    size_t multiplier =
        std::accumulate(begin(resolution), std::prev(end(resolution)),
                        size_t(1), std::multiplies<size_t>{});

    auto res_it = std::prev(end(resolution), 2);
    for (size_t j = 0; j < N; ++j, --res_it) {
      size_t i = N - 1 - j;
      is[i]    = plain_idx / multiplier;
      plain_idx -= is[i] * multiplier;
      if (res_it >= begin(resolution)) { multiplier /= *res_it; }
    }
    return is;
  }
};
//==============================================================================
/// converts multi-dimensional index to a one dimensional index where first
/// dimensions grows slowest
struct x_slowest {
 private:
  template <typename Resolution>
  static constexpr size_t internal_plain_idx(const Resolution& resolution,
                                             const std::vector<size_t>& is) {
    size_t multiplier = 1;
    size_t idx        = 0;

    for (size_t i = 0; i < is.size(); ++i) {
      idx += is[is.size() - 1 - i] * multiplier;
      multiplier *= resolution[is.size() - 1 - i];
    }
    return idx;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename Resolution, typename... Is,
            enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t internal_plain_idx(const Resolution& resolution,
                                             Is... p_is) {
    std::array is{p_is...};

    size_t multiplier = 1;
    size_t idx        = 0;

    for (size_t i = 0; i < is.size(); ++i) {
      idx += is[is.size() - 1 - i] * multiplier;
      multiplier *= resolution[is.size() - 1 - i];
    }
    return idx;
  }

 public:
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t plain_idx(const std::vector<size_t>& resolution,
                                    Is... is) {
    assert(sizeof...(Is) == resolution.size());
    return internal_plain_idx(resolution, is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  static constexpr size_t plain_idx(const std::vector<size_t>& resolution,
                                    const std::vector<size_t>& is) {
    assert(is.size() == resolution.size());
    return internal_plain_idx(resolution, is);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N, typename... Is,
            enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t plain_idx(const std::array<size_t, N>& resolution,
                                    Is... is) {
    static_assert(sizeof...(Is) == N);
    return internal_plain_idx(resolution, is...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  static constexpr size_t plain_idx(const std::array<size_t, N>& resolution,
                                    const std::vector<size_t>& is) {
    assert(is.size() == N);
    return internal_plain_idx(resolution, is);
  }
  //----------------------------------------------------------------------------
  static auto multi_index(const std::vector<size_t>& resolution,
                          size_t /*plain_idx*/) {
    throw std::runtime_error{
        "x_slowest::multi_index(const std::vector<size_t>&, size_t) not "
        "implemented"};
    std::vector<size_t> is(resolution.size());
    return is;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  static constexpr auto multi_index(const std::array<size_t, N>& /*resolution*/,
                          size_t /*plain_idx*/) {
    throw std::runtime_error{
        "x_slowest::multi_index(const std::array<size_t, N>&, size_t) not "
        "implemented"};
    auto is = make_array<size_t, N>();
    return is;
  }
};
//==============================================================================
/// converts multi-dimensional index to a one dimensional index using a
/// space-filling curve algorithm
struct hilbert_curve {
  template <typename... Is, enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t plain_idx(const std::vector<size_t>& /*resolution*/,
                                    Is... /*is*/) {
    throw std::runtime_error{
        "hilbert_curve::plain_idx(const std::vector<size_t>&, Is... is) not "
        "implemented"};
    return 0;
  }
  template <size_t N, typename... Is,
            enable_if_integral<std::decay_t<Is>...> = true>
  static constexpr size_t plain_idx(const std::array<size_t, N>& /*resolution*/,
                                    Is... /*is*/) {
    throw std::runtime_error{
        "hilbert_curve::plain_idx(const std::array<size_t, N>&, Is... is) not "
        "implemented"};
    return 0;
  }
  //----------------------------------------------------------------------------
  static auto multi_index(const std::vector<size_t>& resolution,
                          size_t /*plain_idx*/) {
    std::vector<size_t> is(resolution.size());
    throw std::runtime_error{
        "hilbert_curve::multi_index(const std::vector<size_t>&, size_t) not "
        "implemented"};
    return is;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  static constexpr auto multi_index(const std::array<size_t, N>& /*resolution*/,
                          size_t /*plain_idx*/) {
    auto is = make_array<size_t, N>();
    throw std::runtime_error{
        "hilbert_curve::multi_index(const std::array<size_t, N>&, size_t) not "
        "implemented"};
    return is;
  }
};
//==============================================================================
}  // namespace tatooine
//==============================================================================

#endif
