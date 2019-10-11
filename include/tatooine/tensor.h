#ifndef TATOOINE_TENSOR_H
#define TATOOINE_TENSOR_H

#include <array>
#include <cassert>
#include <iostream>
#include <ostream>
#include "crtp.h"
#include "functional.h"
#include "multidimension.h"
#include "random.h"
#include "symbolic.h"
#include "type_traits.h"
#include "utility.h"

#include <lapacke.h>
#ifdef I
#undef I
#endif
//==============================================================================
namespace tatooine {
//==============================================================================

template <typename Real>
struct fill {
  Real value;
};
#if has_cxx17_support()
template <typename Real>
fill(Real)->fill<Real>;
#endif

struct zeros_t {};
static constexpr zeros_t zeros;

struct ones_t {};
static constexpr ones_t ones;

template <typename Tensor, typename Real, size_t FixedDim, size_t... Dims>
struct tensor_slice;

//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t... Dims>
struct base_tensor : crtp<Tensor> {
  using value_type = Real;
  using real_t     = Real;
  using tensor_t   = Tensor;
  using this_t     = base_tensor<Tensor, Real, Dims...>;
  using parent_t   = crtp<Tensor>;
  using parent_t::as_derived;

  //============================================================================
  static constexpr size_t num_dimensions() { return sizeof...(Dims); }
  static constexpr size_t num_components() { 
#if has_cxx17_support()
    return (Dims * ...); 
#else
    constexpr std::array<size_t, num_dimensions()>dims{Dims...};
    return std::accumulate(begin(dims), end(dims), size_t(1),
                           std::multiplies<size_t>{});
#endif
  }
  static constexpr auto   dimensions() {
    return std::array<size_t, num_dimensions()>{Dims...};
  }
  static constexpr auto   dimension(size_t i) { return dimensions()[i]; }
  static constexpr auto   indices() {
    return multi_index<num_dimensions()>{
        std::array<std::pair<size_t, size_t>, num_dimensions()>{
            {0, Dims - 1}...}};
  }
  template <typename F>
  static auto for_indices(F&& f) {
    for (auto is : indices()) {
      invoke_unpacked(std::forward<F>(f), unpack(is));
    }
  }

  //============================================================================
  constexpr base_tensor() = default;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename other_tensor_t, typename other_real_t>
  constexpr base_tensor(
      const base_tensor<other_tensor_t, other_real_t, Dims...>& other) {
    assign_other_tensor(other);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename other_tensor_t, typename other_real_t>
  constexpr auto& operator=(
      const base_tensor<other_tensor_t, other_real_t, Dims...>& other) {
    assign_other_tensor(other);
    return *this;
  }

  //============================================================================
  template <typename F>
  auto& unary_operation(F&& f) {
    for_indices([this, &f](const auto... is) { at(is...) = f(at(is...)); });
    return as_derived();
  }

  //----------------------------------------------------------------------------
  template <typename F, typename other_tensor_t, typename other_real_t>
  decltype(auto) binary_operation(
      F&& f, const base_tensor<other_tensor_t, other_real_t, Dims...>& other) {
    for_indices([this, &f, &other](const auto... is) {
      at(is...) = f(at(is...), other(is...));
    });
    return as_derived();
  }

  //----------------------------------------------------------------------------
  template <typename other_tensor_t, typename other_real_t>
  constexpr void assign_other_tensor(
      const base_tensor<other_tensor_t, other_real_t, Dims...>& other) {
    for_indices([this, &other](const auto... is) { at(is...) = other(is...); });
  }

  //----------------------------------------------------------------------------
  template <typename... Is, enable_if_integral<Is...> = true>
  constexpr decltype(auto) at(const Is... is) const {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return as_derived().at(is...);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Is, enable_if_integral<Is...> = true>
  constexpr decltype(auto) at(const Is... is) {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return as_derived().at(is...);
  }

  //----------------------------------------------------------------------------
  template <typename... Is, enable_if_integral<Is...> = true>
  constexpr decltype(auto) operator()(const Is... is) const {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return at(is...);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Is, enable_if_integral<Is...> = true>
  constexpr decltype(auto) operator()(const Is... is) {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return at(is...);
  }

  //----------------------------------------------------------------------------
  template <size_t FixedDim, size_t... Is>
  constexpr auto slice(size_t fixed_index, std::index_sequence<Is...>) {
    static_assert(FixedDim < num_dimensions(),
                  "fixed dimensions must be in range of number of dimensions");
    return tensor_slice<
        Tensor, Real, FixedDim,
        dimension(sliced_indices<num_dimensions(), FixedDim>()[Is])...>{
        &as_derived(), fixed_index};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t FixedDim>
  constexpr auto slice(size_t fixed_index) {
    static_assert(FixedDim < num_dimensions(),
                  "fixed dimensions must be in range of number of dimensions");
    return slice<FixedDim>(fixed_index,
                           std::make_index_sequence<num_dimensions() - 1>{});
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t FixedDim, size_t... Is>
  constexpr auto slice(size_t fixed_index, std::index_sequence<Is...>) const {
    static_assert(FixedDim < num_dimensions(),
                  "fixed dimensions must be in range of number of dimensions");
    return tensor_slice<
        const Tensor, Real, FixedDim,
        dimension(sliced_indices<num_dimensions(), FixedDim>()[Is])...>{
        &as_derived(), fixed_index};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t FixedDim>
  constexpr auto slice(size_t fixed_index) const {
    static_assert(FixedDim < num_dimensions(),
                  "fixed dimensions must be in range of number of dimensions");
    return slice<FixedDim>(fixed_index,
                           std::make_index_sequence<num_dimensions() - 1>{});
  }

  //----------------------------------------------------------------------------
  template <typename... Is, enable_if_integral<Is...> = true>
  static constexpr auto array_index(const Is... is) {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return static_multidimension<Dims...>::global_idx(is...);
  }

  //----------------------------------------------------------------------------
  template <typename OtherTensor, typename OtherReal>
  auto& operator+=(const base_tensor<OtherTensor, OtherReal, Dims...>& other) {
    for_indices([&](const auto... is) { at(is...) += other(is...); });
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal, enable_if_arithmetic_or_symbolic<OtherReal> = true>
  auto& operator+=(const OtherReal& other) {
    for_indices([&](const auto... is) { at(is...) += other; });
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename OtherTensor, typename OtherReal>
  auto& operator-=(const base_tensor<OtherTensor, OtherReal, Dims...>& other) {
    for_indices([&](const auto... is) { at(is...) -= other(is...); });
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal, enable_if_arithmetic_or_symbolic<OtherReal> = true>
  auto& operator-=(const OtherReal& other) {
    for_indices([&](const auto... is) { at(is...) -= other; });
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal, enable_if_arithmetic_or_symbolic<OtherReal> = true>
  auto& operator*=(const OtherReal& other) {
    for_indices([&](const auto... is) { at(is...) *= other; });
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal, enable_if_arithmetic_or_symbolic<OtherReal> = true>
  auto& operator/=(const OtherReal& other) {
    for_indices([&](const auto... is) { at(is...) /= other; });
    return *this;
  }
};

//==============================================================================
template <typename Real, size_t... Dims>
struct tensor : base_tensor<tensor<Real, Dims...>, Real, Dims...> {
  //============================================================================
  using this_t   = tensor<Real, Dims...>;
  using parent_t = base_tensor<this_t, Real, Dims...>;
  using parent_t::parent_t;
  using parent_t::operator=;
  using parent_t::dimension;
  using parent_t::num_components;
  using parent_t::num_dimensions;
  using data_container_t = std::array<Real, num_components()>;

  //============================================================================
 protected:
   data_container_t m_data;

  //============================================================================
 public:
  constexpr tensor() : m_data{make_array<Real, num_components()>()} {}
  constexpr tensor(const tensor& other) = default;
  constexpr tensor& operator=(const tensor& other) = default;

  template <typename Real_ = Real, enable_if_arithmetic_or_complex<Real_> = true>
  constexpr tensor(tensor&& other) noexcept : m_data{std::move(other.m_data)}{}
  template <typename Real_ = Real, enable_if_arithmetic_or_complex<Real_> = true>
  constexpr tensor& operator=(tensor&& other) noexcept {
    m_data = std::move(other.m_data);
    return *this;
  }
  ~tensor()                                            = default;

  //============================================================================
 public:
  template <typename _real_t = Real, enable_if_arithmetic<_real_t> = true>
  constexpr tensor(zeros_t /*zeros*/) : tensor{fill<Real>{0}} {}
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename _real_t = Real, enable_if_arithmetic<_real_t> = true>
  constexpr tensor(ones_t /*ones*/) : tensor{fill<Real>{1}} {}
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename fill_real_t, typename _real_t = Real,
            enable_if_arithmetic<_real_t> = true>
  constexpr tensor(fill<fill_real_t> f)
      : m_data{make_array<Real, num_components()>(f.value)} {}
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename RandomReal, typename Engine, typename _real_t = Real,
            enable_if_arithmetic<RandomReal> = true>
  constexpr tensor(random_uniform<RandomReal, Engine>&& rand) : tensor{} {
    this->unary_operation([&](const auto& /*c*/) { return rand.get(); });
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename RandomReal, typename Engine, typename _real_t = Real,
            enable_if_arithmetic<_real_t> = true>
  constexpr tensor(random_normal<RandomReal, Engine>&& rand) : tensor{} {
    this->unary_operation([&](const auto& /*c*/) { return rand.get(); });
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename other_tensor_t, typename other_real_t>
  constexpr tensor(
      const base_tensor<other_tensor_t, other_real_t, Dims...>& other) {
    this->assign_other_tensor(other);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename other_tensor_t, typename other_real_t>
  constexpr auto& operator=(
      const base_tensor<other_tensor_t, other_real_t, Dims...>& other) {
    this->assign_other_tensor(other);
    return *this;
  }

  //----------------------------------------------------------------------------
  static constexpr auto zeros() { return this_t{fill<Real>{0}}; }

  //----------------------------------------------------------------------------
  static constexpr auto ones() { return this_t{fill<Real>{1}}; }

  //----------------------------------------------------------------------------
  template <typename RandomEngine = std::mt19937_64>
  static constexpr auto randu(Real min = 0, Real max = 1,
                              RandomEngine&& eng = RandomEngine{
                                  std::random_device{}()}) {
    return this_t{random_uniform<Real>{eng, min, max}};
  }

  //----------------------------------------------------------------------------
  template <typename RandomEngine = std::mt19937_64>
  static constexpr auto randn(Real mean = 0, Real stddev = 1,
                              RandomEngine&& eng = RandomEngine{
                                  std::random_device{}()}) {
    return this_t{random_normal<Real>{eng, mean, stddev}};
  }

  //============================================================================
  template <typename... Is, enable_if_integral<Is...> = true>
  constexpr const auto& at(const Is... is) const {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return m_data[parent_t::array_index(is...)];
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Is, enable_if_integral<Is...> = true>
  constexpr auto& at(const Is... is) {
    static_assert(sizeof...(Is) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return m_data[parent_t::array_index(is...)];
  }

  //----------------------------------------------------------------------------
  constexpr const auto& operator[](size_t i) const { return m_data[i]; }
  constexpr auto&       operator[](size_t i) { return m_data[i]; }


  //----------------------------------------------------------------------------
  decltype(auto) data() { return m_data.data(); }
  decltype(auto) data() const { return m_data.data(); }

  //----------------------------------------------------------------------------
  template <size_t NumDims = num_dimensions(), std::enable_if_t<NumDims == 1>...>
  auto begin() { return std::begin(m_data); }
  template <size_t NumDims = num_dimensions(), std::enable_if_t<NumDims == 1>...>
  auto begin() const { return std::begin(m_data); }
  template <size_t NumDims = num_dimensions(), std::enable_if_t<NumDims == 1>...>
  auto cbegin() { return std::cbegin(m_data); }

  //----------------------------------------------------------------------------
  template <size_t NumDims = num_dimensions(), std::enable_if_t<NumDims == 1>...>
  auto end() { return std::end(m_data); }
  template <size_t NumDims = num_dimensions(), std::enable_if_t<NumDims == 1>...>
  auto end() const { return std::end(m_data); }
  template <size_t NumDims = num_dimensions(), std::enable_if_t<NumDims == 1>...>
  auto cend() { return std::cend(m_data); }

  //----------------------------------------------------------------------------
  template <typename OtherReal>
  bool operator==(const tensor<OtherReal, Dims...>& other) const {
    return m_data == other.m_data;
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal>
  bool operator<(const tensor<OtherReal, Dims...>& other) const {
    return m_data < other.m_data;
  }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if has_cxx17_support()
template <size_t C, typename... Rows>
tensor(Rows const(&&... rows)[C])
    ->tensor<promote_t<Rows...>, sizeof...(Rows), C>;
#endif

//==============================================================================
template <typename Real, size_t n>
struct vec : tensor<Real, n> {
  using parent_t = tensor<Real, n>;
  using parent_t::parent_t;

  using iterator       = typename parent_t::data_container_t::iterator;
  using const_iterator = typename parent_t::data_container_t::const_iterator;

  template <typename... Ts, enable_if_arithmetic_complex_or_symbolic<Ts...> = true>
  constexpr vec(const Ts... ts) {
    static_assert(sizeof...(Ts) == parent_t::dimension(0),
                  "number of indices does not match number of dimensions");
    this->m_data = {static_cast<Real>(ts)...};
  }

  constexpr vec(const vec&) = default;
  constexpr vec& operator=(const vec&) = default;
  ~vec()                               = default;

  template <typename Real_ = Real, enable_if_arithmetic_or_complex<Real_> = true>
  constexpr vec(vec&& other) noexcept : parent_t{std::move(other)} {}
  template <typename Real_ = Real, enable_if_arithmetic_or_complex<Real_> = true>
  constexpr vec& operator=(vec&& other) noexcept {
    parent_t::operator=(std::move(other));
    return *this;
  }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if has_cxx17_support()
template <typename... Ts>
vec(const Ts...)->vec<promote_t<Ts...>, sizeof...(Ts)>;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
using vec2 = vec<double, 2>;
using vec3 = vec<double, 3>;
using vec4 = vec<double, 4>;

//==============================================================================
template <typename Real, size_t M, size_t N>
struct mat : tensor<Real, M, N> {
  using parent_t = tensor<Real, M, N>;
  using parent_t::parent_t;

  constexpr mat(const mat&)     = default;
  constexpr mat& operator=(const mat&) = default;
  ~mat()                                   = default;
  template <typename Real_ = Real, enable_if_arithmetic_or_complex<Real_> = true>
  constexpr mat(mat&& other) noexcept : parent_t{std::move(other)} {}
  template <typename Real_ = Real, enable_if_arithmetic_or_complex<Real_> = true>
  constexpr mat& operator=(mat&& other) noexcept {
    parent_t::operator=(std::move(other));
    return *this;
  }

  template <typename... Rows, enable_if_arithmetic_or_symbolic<Rows...> = true>
  constexpr mat(Rows(&&... rows)[parent_t::dimension(1)]) {
    static_assert(
        sizeof...(rows) == parent_t::dimension(0),
        "number of given rows does not match specified number of rows");

    // lambda inserting row into data block
    auto insert_row = [r = 0UL, this](const auto& row) mutable {
      for (size_t c = 0; c < parent_t::dimension(1); ++c) {
        this->at(r, c) = static_cast<Real>(row[c]);
      }
      ++r;
    };

    for_each(insert_row, rows...);
  }

  constexpr auto row(size_t i) { return this->template slice<0>(i); }
  constexpr auto row(size_t i) const { return this->template slice<0>(i); }

  constexpr auto col(size_t i) { return this->template slice<1>(i); }
  constexpr auto col(size_t i) const { return this->template slice<1>(i); }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if has_cxx17_support()
template <size_t C, typename... Rows>
mat(Rows const(&&... rows)[C])->mat<promote_t<Rows...>, sizeof...(Rows), C>;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
using mat2 = mat<double, 2, 2>;
using mat3 = mat<double, 3, 3>;
using mat4 = mat<double, 4, 4>;


//==============================================================================
// operations
//==============================================================================

/// Returns the cosine of the angle of two normalized vectors.
template <typename Tensor0, typename Tensor1, typename Real0, typename Real1,
          size_t N>
constexpr auto cos_angle(const base_tensor<Tensor0, Real0, N>& v0,
                         const base_tensor<Tensor1, Real1, N>& v1) {
  return dot(normalize(v0), normalize(v1));
}

//------------------------------------------------------------------------------
/// Returns the angle of two normalized vectors.
template <typename Tensor0, typename Tensor1, typename Real0, typename Real1,
          size_t N>
auto angle(const base_tensor<Tensor0, Real0, N>& v0,
           const base_tensor<Tensor1, Real1, N>& v1) {
  return std::acos(cos_angle(v0, v1));
}
//------------------------------------------------------------------------------
/// Returns the angle of two normalized vectors.
template <typename Tensor0, typename Tensor1, typename Real0, typename Real1,
          size_t N>
auto min_angle(const base_tensor<Tensor0, Real0, N>& v0,
               const base_tensor<Tensor1, Real1, N>& v1) {
  return std::min(angle(v0, v1), angle(v0, -v1));
}
//------------------------------------------------------------------------------
/// Returns the angle of two normalized vectors.
template <typename Tensor0, typename Tensor1, typename Real0, typename Real1,
          size_t N>
auto max_angle(const base_tensor<Tensor0, Real0, N>& v0,
               const base_tensor<Tensor1, Real1, N>& v1) {
  return std::max(angle(v0, v1), angle(v0, -v1));
}
//------------------------------------------------------------------------------
/// Returns the cosine of the angle three points.
template <typename Tensor0, typename Tensor1, typename Tensor2,
          typename Real0, typename Real1, typename Real2, size_t N>
constexpr auto cos_angle(const base_tensor<Tensor0, Real0, N>& v0,
                         const base_tensor<Tensor1, Real1, N>& v1,
                         const base_tensor<Tensor2, Real2, N>& v2) {
  return cos_angle(v0 - v1, v2 - v1);
}
//------------------------------------------------------------------------------
/// Returns the cosine of the angle three points.
template <typename Tensor0, typename Tensor1, typename Tensor2, typename Real0,
          typename Real1, typename Real2, size_t N>
auto angle(const base_tensor<Tensor0, Real0, N>& v0,
                     const base_tensor<Tensor1, Real1, N>& v1,
                     const base_tensor<Tensor2, Real2, N>& v2) {
  return std::acos(cos_angle(v0, v1, v2));
}
//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t... Dims>
constexpr Real min(const base_tensor<Tensor, Real, Dims...>& t) {
  Real m = std::numeric_limits<Real>::max();
  t.for_indices([&](const auto... is) { m = std::min(m, t(is...)); });
  return m;
}

//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t... Dims>
constexpr Real max(const base_tensor<Tensor, Real, Dims...>& t) {
  Real m = -std::numeric_limits<Real>::max();
  t.for_indices([&](const auto... is) { m = std::max(m, t(is...)); });
  return m;
}
//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t N>
constexpr Real norm(const base_tensor<Tensor, Real, N>& t, unsigned p = 2) {
  Real n = 0;
  for (size_t i = 0; i < N; ++i) { n += std::pow(t(i), p); }
  return std::pow(n, Real(1) / Real(p));
}
//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t N>
constexpr Real norm_inf(const base_tensor<Tensor, Real, N>& t) {
  Real norm = -std::numeric_limits<Real>::max();
  for (size_t i = 0; i < N; ++i) { norm = std::max(norm, std::abs(t(i))); }
  return norm;
}
//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t N>
constexpr Real norm1(const base_tensor<Tensor, Real, N>& t) {
  Real norm = 0;
  for (size_t i = 0; i < N; ++i) { norm += std::abs(t(i)); }
  return norm;
}
//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t N>
constexpr Real sqr_length(const base_tensor<Tensor, Real, N>& t_in) {
  Real n = 0;
  for (size_t i = 0; i < N; ++i) { n += t_in(i) * t_in(i); }
  return n;
}
//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t N>
constexpr Real length(const base_tensor<Tensor, Real, N>& t_in) {
  return std::sqrt(sqr_length(t_in));
}

//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t N>
constexpr auto normalize(const base_tensor<Tensor, Real, N>& t_in) {
  return t_in / length(t_in);
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t N>
constexpr auto distance(const base_tensor<LhsTensor, LhsReal, N>& lhs,
                        const base_tensor<RhsTensor, RhsReal, N>& rhs) {
  return length(rhs - lhs);
}

//------------------------------------------------------------------------------
/// sum of all components of a vector
template <typename Tensor, typename Real, size_t VecDim>
constexpr Real sum(const base_tensor<Tensor, Real, VecDim>& v) {
  Real s = 0;
  for (size_t i = 0; i < VecDim; ++i) { s += v(i); }
  return s;
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t N>
constexpr auto dot(const base_tensor<LhsTensor, LhsReal, N>& lhs,
                   const base_tensor<RhsTensor, RhsReal, N>& rhs) {
  promote_t<LhsReal, RhsReal> d = 0;
  for (size_t i = 0; i < N; ++i) {
    d += lhs(i) * rhs(i); }
  return d;
}

//------------------------------------------------------------------------------
template <typename Tensor, typename Real>
constexpr Real det(const base_tensor<Tensor, Real, 2, 2>& m) {
  return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Tensor, typename Real>
constexpr Real det(const base_tensor<Tensor, Real, 3, 3>& m) {
  return m(0, 0) * m(1, 1) * m(2, 2) + m(0, 1) * m(1, 2) * m(2, 0) +
         m(0, 2) * m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1) * m(0, 2) -
         m(2, 1) * m(1, 2) * m(0, 0) - m(2, 2) * m(1, 0) * m(0, 2);
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal>
constexpr auto cross(const base_tensor<LhsTensor, LhsReal, 3>& lhs,
                     const base_tensor<RhsTensor, RhsReal, 3>& rhs) {
  return vec<promote_t<LhsReal, RhsReal>, 3>{lhs(1) * rhs(2) - lhs(2) * rhs(1),
                                             lhs(2) * rhs(0) - lhs(0) * rhs(2),
                                             lhs(0) * rhs(1) - lhs(1) * rhs(0)};
}

//------------------------------------------------------------------------------
template <typename F, typename Tensor, typename Real, size_t N>
constexpr auto unary_operation(F&&                                 f,
                               const base_tensor<Tensor, Real, N>& t_in) {
  using RealOut = typename std::result_of<decltype(f)(Real)>::type;
  vec<RealOut, N> t_out = t_in;
  t_out.unary_operation(std::forward<F>(f));
  return t_out;
}
template <typename F, typename Tensor, typename Real, size_t M, size_t N>
constexpr auto unary_operation(F&&                                       f,
                               const base_tensor<Tensor, Real, M,N>& t_in) {
  using RealOut = typename std::result_of<decltype(f)(Real)>::type;
  mat<RealOut, M, N> t_out = t_in;
  t_out.unary_operation(std::forward<F>(f));
  return t_out;
}
template <typename F, typename Tensor, typename Real, size_t... Dims>
constexpr auto unary_operation(F&&                                       f,
                               const base_tensor<Tensor, Real, Dims...>& t_in) {
  using RealOut = typename std::result_of<decltype(f)(Real)>::type;
  tensor<RealOut, Dims...> t_out = t_in;
  t_out.unary_operation(std::forward<F>(f));
  return t_out;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename F, typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t N>
constexpr auto binary_operation(
    F&& f, const base_tensor<LhsTensor, LhsReal, N>& lhs,
    const base_tensor<RhsTensor, RhsReal, N>& rhs) {
  using RealOut = typename std::result_of<decltype(f)(LhsReal, RhsReal)>::type;
  vec<RealOut, N> t_out = lhs;
  t_out.binary_operation(std::forward<F>(f), rhs);
  return t_out;
}
template <typename F, typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t M, size_t N>
constexpr auto binary_operation(
    F&& f, const base_tensor<LhsTensor, LhsReal, M, N>& lhs,
    const base_tensor<RhsTensor, RhsReal, M, N>& rhs) {
  using RealOut = typename std::result_of<decltype(f)(LhsReal, RhsReal)>::type;
  mat<RealOut, M, N> t_out = lhs;
  t_out.binary_operation(std::forward<F>(f), rhs);
  return t_out;
}
template <typename F, typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t... Dims>
constexpr auto binary_operation(
    F&& f, const base_tensor<LhsTensor, LhsReal, Dims...>& lhs,
    const base_tensor<RhsTensor, RhsReal, Dims...>& rhs) {
  using RealOut = typename std::result_of<decltype(f)(LhsReal, RhsReal)>::type;
  tensor<RealOut, Dims...> t_out = lhs;
  t_out.binary_operation(std::forward<F>(f), rhs);
  return t_out;
}

//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t... Dims>
constexpr auto operator-(const base_tensor<Tensor, Real, Dims...>& t) {
  return unary_operation([](const auto& c) { return -c; }, t);
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsReal,
          size_t... Dims, enable_if_arithmetic<RhsReal> = true>
constexpr auto operator+(const base_tensor<LhsTensor, LhsReal, Dims...>& lhs,
                         RhsReal scalar) {
  return unary_operation([scalar](const auto& c) { return c + scalar; }, lhs);
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t M, size_t N, size_t O>
constexpr auto operator*(const base_tensor<LhsTensor, LhsReal, M, N>& lhs,
                         const base_tensor<RhsTensor, RhsReal, N, O>& rhs) {
  mat<promote_t<LhsReal, RhsReal>, M, O> product;
  for (size_t r = 0; r < M; ++r) {
    for (size_t c = 0; c < O; ++c) {
      product(r, c) = dot(lhs.template slice<0>(r), rhs.template slice<1>(c));
    }
  }
  return product;
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t... Dims,
          std::enable_if_t<(sizeof...(Dims) != 2)>...>
constexpr auto operator*(const base_tensor<LhsTensor, LhsReal, Dims...>& lhs,
                         const base_tensor<RhsTensor, RhsReal, Dims...>& rhs) {
  return binary_operation(std::multiplies<promote_t<LhsReal, RhsReal>>{}, lhs,
                          rhs);
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t... Dims>
constexpr auto operator/(const base_tensor<LhsTensor, LhsReal, Dims...>& lhs,
                         const base_tensor<RhsTensor, RhsReal, Dims...>& rhs) {
  return binary_operation(std::divides<promote_t<LhsReal, RhsReal>>{}, lhs,
                          rhs);
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t... Dims>
constexpr auto operator+(const base_tensor<LhsTensor, LhsReal, Dims...>& lhs,
                         const base_tensor<RhsTensor, RhsReal, Dims...>& rhs) {
  return binary_operation(std::plus<promote_t<LhsReal, RhsReal>>{}, lhs, rhs);
}

//------------------------------------------------------------------------------
template <typename Tensor, typename tensor_real_t, typename scalar_real_t,
          size_t... Dims,
          enable_if_arithmetic_complex_or_symbolic<scalar_real_t> = true>
constexpr auto operator*(const base_tensor<Tensor, tensor_real_t, Dims...>& t,
                         const scalar_real_t scalar) {
  return unary_operation(
      [scalar](const auto& component) { return component * scalar; }, t);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Tensor, typename tensor_real_t, typename scalar_real_t,
          size_t... Dims,
          enable_if_arithmetic_complex_or_symbolic<scalar_real_t> = true>
constexpr auto operator*(const scalar_real_t scalar,
                         const base_tensor<Tensor, tensor_real_t, Dims...>& t) {
  return unary_operation(
      [scalar](const auto& component) { return component * scalar; }, t);
}

//------------------------------------------------------------------------------
template <typename Tensor, typename tensor_real_t, typename scalar_real_t,
          size_t... Dims,
          enable_if_arithmetic_complex_or_symbolic<scalar_real_t> = true>
constexpr auto operator/(const base_tensor<Tensor, tensor_real_t, Dims...>& t,
                         const scalar_real_t scalar) {
  return unary_operation(
      [scalar](const auto& component) { return component / scalar; }, t);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Tensor, typename tensor_real_t, typename scalar_real_t,
          size_t... Dims,
          enable_if_arithmetic_complex_or_symbolic<scalar_real_t> = true>
constexpr auto operator/(const scalar_real_t scalar,
                         const base_tensor<Tensor, tensor_real_t, Dims...>& t) {
  return unary_operation(
      [scalar](const auto& component) { return scalar / component; }, t);
}

//------------------------------------------------------------------------------
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t... Dims>
constexpr auto operator-(const base_tensor<LhsTensor, LhsReal, Dims...>& lhs,
                         const base_tensor<RhsTensor, RhsReal, Dims...>& rhs) {
  return binary_operation(std::minus<promote_t<LhsReal, RhsReal>>{}, lhs, rhs);
}

//------------------------------------------------------------------------------
/// matrix-vector-multiplication
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t M, size_t N>
constexpr auto operator*(const base_tensor<LhsTensor, LhsReal, M, N>& lhs,
                         const base_tensor<RhsTensor, RhsReal, N>&    rhs) {
  vec<promote_t<LhsReal, RhsReal>, M> product;
  for (size_t i = 0; i < M; ++i) {
    product(i) = dot(lhs.template slice<0>(i), rhs);
  }
  return product;
}

//------------------------------------------------------------------------------
template <size_t N>
auto gesv(const tensor<float, N, N>& A, const tensor<float, N>& b) {
  vec<float, N> x = b;
  vec<int, N>  ipiv;
  int          nrhs = 1;
  LAPACKE_sgesv(LAPACK_COL_MAJOR, N, nrhs, const_cast<float*>(A.data()), N,
                ipiv.data(), const_cast<float*>(x.data()), N);
  return x;
}
//------------------------------------------------------------------------------
template <size_t N>
auto gesv(const tensor<double, N, N>& A, const tensor<double, N>& b) {
  vec<double, N> x = b;
  vec<int, N>  ipiv;
  int          nrhs = 1;
  LAPACKE_dgesv(LAPACK_COL_MAJOR, N, nrhs, const_cast<double*>(A.data()), N,
                ipiv.data(), const_cast<double*>(x.data()), N);
  return x;
}

//------------------------------------------------------------------------------
template <size_t M, size_t N>
auto gesv(const tensor<float, M, M>& A, const tensor<float, M, N>& B) {
  mat<float, M, N> X = B;
  tensor<int, M>  ipiv;
    LAPACKE_sgesv(LAPACK_COL_MAJOR, M, N, const_cast<float*>(A.data()), M,
                  ipiv.data(), const_cast<float*>(X.data()), M);
  return X;
}
template <size_t M, size_t N>
auto gesv(const tensor<double, M, M>& A, const tensor<double, M, N>& B) {
  mat<double, M, N> X = B;
  tensor<int, M>  ipiv;
    LAPACKE_dgesv(LAPACK_COL_MAJOR, M, N, const_cast<double*>(A.data()), M,
                  ipiv.data(), const_cast<double*>(X.data()), M);
  return X;
}

//------------------------------------------------------------------------------
template <size_t N>
vec<std::complex<float>, N> eigenvalues(tensor<float, N, N> A) {
  [[maybe_unused]] lapack_int info;
  std::array<float, N>         wr;
  std::array<float, N>         wi;
    info = LAPACKE_sgeev(LAPACK_COL_MAJOR, 'N', 'N', N, A.data(), N, wr.data(),
                         wi.data(), nullptr, N, nullptr, N);

  vec<std::complex<float>, N> vals;
  for (size_t i = 0; i < N; ++i) { vals[i] = {wr[i], wi[i]}; }
  return vals;
}
template <size_t N>
vec<std::complex<double>, N> eigenvalues(tensor<double, N, N> A) {
  [[maybe_unused]] lapack_int info;
  std::array<double, N>         wr;
  std::array<double, N>         wi;
    info = LAPACKE_dgeev(LAPACK_COL_MAJOR, 'N', 'N', N, A.data(), N, wr.data(),
                         wi.data(), nullptr, N, nullptr, N);
  vec<std::complex<double>, N> vals;
  for (size_t i = 0; i < N; ++i) { vals[i] = {wr[i], wi[i]}; }
  return vals;
}

//------------------------------------------------------------------------------
template <size_t N>
std::pair<mat<std::complex<float>, N, N>, vec<std::complex<float>, N>>
eigenvectors(tensor<float, N, N> A) {
  [[maybe_unused]] lapack_int info;
  std::array<float, N>         wr;
  std::array<float, N>         wi;
  std::array<float, N * N>     vr;
    info = LAPACKE_sgeev(LAPACK_COL_MAJOR, 'N', 'V', N, A.data(), N, wr.data(),
                         wi.data(), nullptr, N, vr.data(), N);

  vec<std::complex<float>, N>    vals;
  mat<std::complex<float>, N, N> vecs;
  for (size_t i = 0; i < N; ++i) { vals[i] = {wr[i], wi[i]}; }
  for (size_t j = 0; j < N; ++j) {
    for (size_t i = 0; i < N; ++i) {
      if (wi[j] == 0) {
        vecs(i, j) = {vr[i + j * N], 0};
      } else {
        vecs(i, j)     = {vr[i + j * N], vr[i + (j + 1) * N]};
        vecs(i, j + 1) = {vr[i + j * N], -vr[i + (j + 1) * N]};
        if (i == N - 1) { ++j; }
      }
    }
  }

  return {std::move(vecs), std::move(vals)};
}
template <size_t N>
std::pair<mat<std::complex<double>, N, N>, vec<std::complex<double>, N>>
eigenvectors(tensor<double, N, N> A) {
  [[maybe_unused]] lapack_int info;
  std::array<double, N>         wr;
  std::array<double, N>         wi;
  std::array<double, N * N>     vr;
  info = LAPACKE_dgeev(LAPACK_COL_MAJOR, 'N', 'V', N, A.data(), N, wr.data(),
                       wi.data(), nullptr, N, vr.data(), N);

  vec<std::complex<double>, N>    vals;
  mat<std::complex<double>, N, N> vecs;
  for (size_t i = 0; i < N; ++i) { vals[i] = {wr[i], wi[i]}; }
  for (size_t j = 0; j < N; ++j) {
    for (size_t i = 0; i < N; ++i) {
      if (wi[j] == 0) {
        vecs(i, j) = {vr[i + j * N], 0};
      } else {
        vecs(i, j)     = {vr[i + j * N], vr[i + (j + 1) * N]};
        vecs(i, j + 1) = {vr[i + j * N], -vr[i + (j + 1) * N]};
        if (i == N - 1) { ++j; }
      }
    }
  }

  return {std::move(vecs), std::move(vals)};
}

//------------------------------------------------------------------------------
template <size_t N>
auto eigenvalues_sym(tensor<float, N, N> A) {
  vec<float, N>               vals;
  [[maybe_unused]] lapack_int info;
  info = LAPACKE_ssyev(LAPACK_COL_MAJOR, 'N', 'U', N, A.data(), N, vals.data());
  return vals;
}
template <size_t N>
auto eigenvalues_sym(tensor<double, N, N> A) {
  vec<double, N>              vals;
  [[maybe_unused]] lapack_int info;
  info = LAPACKE_dsyev(LAPACK_COL_MAJOR, 'N', 'U', N, A.data(), N, vals.data());

  return vals;
}

//------------------------------------------------------------------------------
template <size_t N>
std::pair<mat<float, N, N>, vec<float, N>> eigenvectors_sym(mat<float, N, N> A) {
  vec<float, N>               vals;
  [[maybe_unused]] lapack_int info;
  info = LAPACKE_ssyev(LAPACK_COL_MAJOR, 'V', 'U', N, A.data(), N, vals.data());
  return {std::move(A), std::move(vals)};
}
template <size_t N>
std::pair<mat<double, N, N>, vec<double, N>> eigenvectors_sym(
    mat<double, N, N> A) {
  vec<double, N>              vals;
  [[maybe_unused]] lapack_int info;
  info = LAPACKE_dsyev(LAPACK_COL_MAJOR, 'V', 'U', N, A.data(), N, vals.data());
  return {std::move(A), std::move(vals)};
}

//------------------------------------------------------------------------------
/// for comparison
template <typename LhsTensor, typename LhsReal, typename RhsTensor,
          typename RhsReal, size_t... Dims>
constexpr bool approx_equal(const base_tensor<LhsTensor, LhsReal, Dims...>& lhs,
                            const base_tensor<RhsTensor, RhsReal, Dims...>& rhs,
                            promote_t<LhsReal, RhsReal> eps = 1e-6) {
  bool equal = true;
  lhs.for_indices([&](const auto... is) {
    if (std::abs(lhs(is...) - rhs(is...)) > eps) { equal = false; }
  });
  return equal;
}

//==============================================================================
// views
//==============================================================================
template <typename Tensor, typename Real, size_t FixedDim, size_t... Dims>
struct tensor_slice : base_tensor<tensor_slice<Tensor, Real, FixedDim, Dims...>,
                                  Real, Dims...> {
  using tensor_t          = Tensor;
  using this_t            = tensor_slice<Tensor, Real, FixedDim, Dims...>;
  using parent_t          = base_tensor<this_t, Real, Dims...>;
  using parent_t::operator=;
  using parent_t::num_components;
  using parent_t::num_dimensions;

  //============================================================================
 private:
  Tensor* m_tensor;
  size_t  m_fixed_index;

  //============================================================================
 public:
  constexpr tensor_slice(Tensor* tensor, size_t fixed_index)
      : m_tensor{tensor}, m_fixed_index{fixed_index} {}

  //----------------------------------------------------------------------------
  template <typename... Is, enable_if_integral<Is...> = true>
  constexpr const auto& at(const Is... is) const {
    if constexpr (FixedDim == 0) {
      return m_tensor->at(m_fixed_index, is...);

    } else if constexpr (FixedDim == num_dimensions()) {
      return m_tensor->at(is..., m_fixed_index);

    } else {
      auto at_ = [this](const auto... is) -> decltype(auto) {
        return m_tensor->at(is...);
      };
      return invoke_unpacked(
          at_, unpack(extract<0, FixedDim - 1>(is...)), m_fixed_index,
          unpack(extract<FixedDim, num_dimensions() - 1>(is...)));
    };
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Is, enable_if_integral<Is...> = true,
            typename _tensor_t = Tensor,
            std::enable_if_t<!std::is_const_v<_tensor_t>>...>
  constexpr auto& at(const Is... is) {
    if constexpr (FixedDim == 0) {
      return m_tensor->at(m_fixed_index, is...);

    } else if constexpr (FixedDim == num_dimensions()) {
      return m_tensor->at(is..., m_fixed_index);

    } else {
      auto at_ = [this](const auto... is) -> decltype(auto) {
        return m_tensor->at(is...);
      };
      return invoke_unpacked(
          at_, unpack(extract<0, FixedDim - 1>(is...)), m_fixed_index,
          unpack(extract<FixedDim, num_dimensions() - 1>(is...)));
    };
  }
};

//==============================================================================
template <typename Tensor, typename Real, size_t... Dims>
struct const_imag_complex_tensor
    : base_tensor<const_imag_complex_tensor<Tensor, Real, Dims...>, Real,
                  Dims...> {
  using this_t = const_imag_complex_tensor<Tensor, Real, Dims...>;
  using parent_t = base_tensor<this_t, Real, Dims...>;
  using parent_t::num_dimensions;

  //============================================================================
 private:
  const Tensor& m_internal_tensor;

  //============================================================================
 public:
  constexpr const_imag_complex_tensor(
      const base_tensor<Tensor, std::complex<Real>, Dims...>& internal_tensor)
      : m_internal_tensor{internal_tensor.as_derived()} {}

  //----------------------------------------------------------------------------
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) operator()(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).imag();
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) at(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).imag();
  }

  //----------------------------------------------------------------------------
  const auto& internal_tensor() const { return m_internal_tensor; }
};

//==============================================================================
template <typename Tensor, typename Real, size_t... Dims>
struct imag_complex_tensor
    : base_tensor<imag_complex_tensor<Tensor, Real, Dims...>, Real, Dims...> {
  using this_t = imag_complex_tensor<Tensor, Real, Dims...>;
  using parent_t = base_tensor<this_t, Real, Dims...>;
  using parent_t::num_dimensions;
  //============================================================================
 private:
  Tensor& m_internal_tensor;

  //============================================================================
 public:
  constexpr imag_complex_tensor(
      base_tensor<Tensor, std::complex<Real>, Dims...>& internal_tensor)
      : m_internal_tensor{internal_tensor.as_derived()} {}

  //----------------------------------------------------------------------------
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) operator()(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).imag();
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) operator()(const Indices... indices) {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).imag();
  }
  //----------------------------------------------------------------------------
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) at(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).imag();
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) at(const Indices... indices) {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).imag();
  }

  //----------------------------------------------------------------------------
  auto&       internal_tensor() { return m_internal_tensor; }
  const auto& internal_tensor() const { return m_internal_tensor; }
};

//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t... Dims>
auto imag(const base_tensor<Tensor, std::complex<Real>, Dims...>& tensor) {
  return const_imag_complex_tensor{tensor.as_derived()};
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Tensor, typename Real, size_t... Dims>
auto imag(base_tensor<Tensor, std::complex<Real>, Dims...>& tensor) {
  return imag_complex_tensor{tensor.as_derived()};
}

//==============================================================================
template <typename Tensor, typename Real, size_t... Dims>
struct const_real_complex_tensor
    : base_tensor<const_real_complex_tensor<Tensor, Real, Dims...>, Real,
                  Dims...> {
  using this_t = const_real_complex_tensor<Tensor, Real, Dims...>;
  using parent_t = base_tensor<this_t, Real, Dims...>;
  using parent_t::num_dimensions;
  //============================================================================
 private:
  const Tensor& m_internal_tensor;

  //============================================================================
 public:
  const_real_complex_tensor(
      const base_tensor<Tensor, std::complex<Real>, Dims...>& internal_tensor)
      : m_internal_tensor{internal_tensor.as_derived()} {}

  //----------------------------------------------------------------------------
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) operator()(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).real();
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) at(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).real();
  }

  //----------------------------------------------------------------------------
  const auto& internal_tensor() const { return m_internal_tensor; }
};

//==============================================================================
template <typename Tensor, typename Real, size_t... Dims>
struct real_complex_tensor
    : base_tensor<real_complex_tensor<Tensor, Real, Dims...>, Real, Dims...> {
  using this_t = real_complex_tensor<Tensor, Real, Dims...>;
  using parent_t = base_tensor<this_t, Real, Dims...>;
  using parent_t::num_dimensions;
  //============================================================================
 private:
  Tensor& m_internal_tensor;

  //============================================================================
 public:
  real_complex_tensor(
      base_tensor<Tensor, std::complex<Real>, Dims...>& internal_tensor)
      : m_internal_tensor{internal_tensor.as_derived()} {}

  //----------------------------------------------------------------------------
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) operator()(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).real();
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) operator()(const Indices... indices) {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).real();
  }
  //----------------------------------------------------------------------------
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) at(const Indices... indices) const {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).real();
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Indices, enable_if_integral<Indices...> = true>
  constexpr decltype(auto) at(const Indices... indices) {
    static_assert(sizeof...(Indices) == num_dimensions());
    return m_internal_tensor(indices...).real();
  }

  //----------------------------------------------------------------------------
  auto&       internal_tensor() { return m_internal_tensor; }
  const auto& internal_tensor() const { return m_internal_tensor; }
};

//------------------------------------------------------------------------------
template <typename Tensor, typename Real, size_t... Dims>
auto real(const base_tensor<Tensor, Real, Dims...>& t) {
  return const_real_complex_tensor{t.as_derived()};
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Tensor, typename Real, size_t... Dims>
auto real(base_tensor<Tensor, Real, Dims...>& t) {
  return real_complex_tensor{t.as_derived()};
}
//==============================================================================
template <typename Matrix, size_t M, size_t N>
struct const_transposed_matrix
    : base_tensor<const_transposed_matrix<Matrix, M, N>,
                  typename Matrix::real_t, M, N> {
  //============================================================================
 private:
  const Matrix& m_internal_matrix;

  //============================================================================
 public:
  const_transposed_matrix(
      const base_tensor<Matrix, typename Matrix::real_t, N, M>& internal_matrix)
      : m_internal_matrix{internal_matrix.as_derived()} {}

  //----------------------------------------------------------------------------
  constexpr const auto& operator()(const size_t r, const size_t c) const {
    return m_internal_matrix(c, r);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr const auto& at(const size_t r, const size_t c) const {
    return m_internal_matrix(c, r);
  }

  //----------------------------------------------------------------------------
  const auto& internal_matrix() const { return m_internal_matrix; }
};

//==============================================================================
template <typename Matrix, size_t M, size_t N>
struct transposed_matrix
    : base_tensor<transposed_matrix<Matrix, M, N>, typename Matrix::real_t, M, N> {
  //============================================================================
 private:
  Matrix& m_internal_matrix;

  //============================================================================
 public:
  transposed_matrix(
      base_tensor<Matrix, typename Matrix::real_t, N, M>& internal_matrix)
      : m_internal_matrix{internal_matrix.as_derived()} {}

  //----------------------------------------------------------------------------
  constexpr const auto& operator()(const size_t r, const size_t c) const {
    return m_internal_matrix(c, r);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr auto& operator()(const size_t r, const size_t c) {
    return m_internal_matrix(c, r);
  }
  //----------------------------------------------------------------------------
  constexpr const auto& at(const size_t r, const size_t c) const {
    return m_internal_matrix(c, r);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr auto& at(const size_t r, const size_t c) {
    return m_internal_matrix(c, r);
  }

  //----------------------------------------------------------------------------
  auto&       internal_matrix() { return m_internal_matrix; }
  const auto& internal_matrix() const { return m_internal_matrix; }
};

//------------------------------------------------------------------------------
template <typename Matrix, typename Real, size_t M, size_t N>
auto transpose(const base_tensor<Matrix, Real, M, N>& matrix) {
  return const_transposed_matrix{matrix.as_derived()};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Matrix, typename Real, size_t M, size_t N>
auto transpose(base_tensor<Matrix, Real, M, N>& matrix) {
  return transposed_matrix{matrix.as_derived()};
}

//------------------------------------------------------------------------------
template <typename Matrix, typename Real, size_t M, size_t N>
auto& transpose(base_tensor<transposed_matrix<Matrix, M, N>, Real, M, N>&
                    transposed_matrix) {
  return transposed_matrix.as_derived().internal_matrix();
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Matrix, typename Real, size_t M, size_t N>
const auto& transpose(const base_tensor<transposed_matrix<Matrix, M, N>, Real,
                                        M, N>& transposed_matrix) {
  return transposed_matrix.as_derived().internal_matrix();
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Matrix, typename Real, size_t M, size_t N>
const auto& transpose(const base_tensor<const_transposed_matrix<Matrix, M, N>,
                                        Real, M, N>& transposed_matrix) {
  return transposed_matrix.as_derived().internal_matrix();
}

//==============================================================================
// symbolic
//==============================================================================
template <typename RealOut = double, typename Tensor, size_t... Dims,
          typename... Relations>
auto evtod(const base_tensor<Tensor, GiNaC::ex, Dims...>& t_in,
           Relations&&... relations) {
  tensor<RealOut, Dims...> t_out;

  t_out.for_indices([&](const auto... is) {
    t_out(is...) = symbolic::evtod<RealOut>(
        t_in(is...), std::forward<Relations>(relations)...);
  });

  return t_out;
}

//------------------------------------------------------------------------------
template <typename RealOut = double, typename Tensor, size_t... Dims,
          typename... Relations>
auto diff(const base_tensor<Tensor, GiNaC::ex, Dims...>& t_in,
          const GiNaC::symbol& symbol, unsigned nth = 1) {
  return unary_operation(
      [&](const auto& component) { return component.diff(symbol, nth); }, t_in);
}

//------------------------------------------------------------------------------
template <typename Tensor, size_t M, size_t N>
auto to_ginac_matrix(const base_tensor<Tensor, GiNaC::ex, M, N>& m_in) {
  GiNaC::matrix m_out(M, N);
  m_in.for_indices([&](const auto... is) { m_out(is...) = m_in(is...); });
  return m_out;
}

//------------------------------------------------------------------------------
template <size_t M, size_t N>
auto to_mat(const GiNaC::matrix& m_in) {
  assert(m_in.rows() == M);
  assert(m_in.cols() == N);
  mat<GiNaC::ex, M, N> m_out;
  m_out.for_indices([&](const auto... is) { m_out(is...) = m_in(is...); });
  return m_out;
}

//------------------------------------------------------------------------------
template <typename Tensor, size_t... Dims>
void eval(base_tensor<Tensor, GiNaC::ex, Dims...>& m) {
  m.for_indices([&m](const auto... is) { m(is...) = m(is...).eval(); });
}
//------------------------------------------------------------------------------
template <typename Tensor, size_t... Dims>
void evalf(base_tensor<Tensor, GiNaC::ex, Dims...>& m) {
  m.for_indices([&m](const auto... is) { m(is...) = m(is...).evalf(); });
}
//------------------------------------------------------------------------------
template <typename Tensor, size_t... Dims>
void evalm(base_tensor<Tensor, GiNaC::ex, Dims...>& m) {
  m.for_indices([&m](const auto... is) { m(is...) = m(is...).evalm(); });
}

//------------------------------------------------------------------------------
template <typename Tensor, size_t... Dims>
void expand(base_tensor<Tensor, GiNaC::ex, Dims...>& m) {
  m.for_indices([&m](const auto... is) { m(is...) = m(is...).expand(); });
}

//------------------------------------------------------------------------------
template <typename Tensor, size_t... Dims>
void normal(base_tensor<Tensor, GiNaC::ex, Dims...>& m) {
  m.for_indices([&m](const auto... is) { m(is...).normal(); });
}

//------------------------------------------------------------------------------
template <typename Tensor, size_t M, size_t N>
auto inverse(const base_tensor<Tensor, GiNaC::ex, M, N>& m_in) {
  return to_mat<M, N>(to_ginac_matrix(m_in).inverse());
}

//==============================================================================
// I/O
//==============================================================================
/// printing vector
template <typename Tensor, typename Real, size_t N>
auto& operator<<(std::ostream& out, const base_tensor<Tensor, Real, N>& v) {
  out << "[ ";
  out << std::scientific;
  for (size_t i = 0; i < N; ++i) {
    if constexpr (!is_complex_v<Real>) {
      if (v(i) >= 0) { out << ' '; }
    }
    out << v(i) << ' ';
  }
  out << "]";
  out << std::defaultfloat;
  return out;
}

template <typename Tensor, typename Real, size_t M, size_t N>
auto& operator<<(std::ostream& out, const base_tensor<Tensor, Real, M, N>& m) {
  out << std::scientific;
  for (size_t j = 0; j < M; ++j) {
    out << "[ ";
    for (size_t i = 0; i < N; ++i) {
      if constexpr (!is_complex_v<Real>) {
        if (m(j, i) >= 0) { out << ' '; }
      }
      out << m(j, i) << ' ';
    }
    out << "]\n";
  }
  out << std::defaultfloat;
  return out;
}

//==============================================================================
// type traits
//==============================================================================
template <typename Tensor, typename Real, size_t n>
struct num_components<base_tensor<Tensor, Real, n>>
    : std::integral_constant<size_t, n> {};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t n>
struct num_components<tensor<Real, n>> : std::integral_constant<size_t, n> {};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t n>
struct num_components<vec<Real, n>> : std::integral_constant<size_t, n> {};

//==============================================================================
template <typename Real, size_t N>
struct internal_data_type<vec<Real, N>> {
  using type = Real;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t M, size_t N>
struct internal_data_type<mat<Real, M, N>> {
  using type = Real;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t... Dims>
struct internal_data_type<tensor<Real, Dims...>> {
  using type = Real;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Tensor, typename Real, size_t... Dims>
struct internal_data_type<base_tensor<Tensor, Real, Dims...>> {
  using type = Real;
};

//==============================================================================
template <typename Tensor, typename Real, size_t N>
struct unpack<base_tensor<Tensor, Real, N>> {
  static constexpr size_t       n = N;
  base_tensor<Tensor, Real, N>& container;

  //----------------------------------------------------------------------------
  constexpr unpack(base_tensor<Tensor, Real, N>& c) : container{c} {}

  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto& get() {
    return container(I);
  }

  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr const auto& get() const {
    return container(I);
  }
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <typename Tensor, typename Real, size_t N>
unpack(base_tensor<Tensor, Real, N>& c)->unpack<base_tensor<Tensor, Real, N>>;

//==============================================================================
template <typename Tensor, typename Real, size_t N>
struct unpack<const base_tensor<Tensor, Real, N>> {
  static constexpr size_t             n = N;
  const base_tensor<Tensor, Real, N>& container;

  //----------------------------------------------------------------------------
  constexpr unpack(const base_tensor<Tensor, Real, N>& c) : container{c} {}

  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr const auto& get() const {
    return container(I);
  }
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <typename Tensor, typename Real, size_t N>
unpack(const base_tensor<Tensor, Real, N>& c)
    ->unpack<const base_tensor<Tensor, Real, N>>;

//==============================================================================
template <typename Real, size_t N>
struct unpack<tensor<Real, N>> {
  static constexpr size_t n = N;
  tensor<Real, N>&        container;

  //----------------------------------------------------------------------------
  constexpr unpack(tensor<Real, N>& c) : container{c} {}

  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto& get() {
    return container[I];
  }

  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr const auto& get() const {
    return container[I];
  }
};
//==============================================================================
template <typename Real, size_t N>
unpack(tensor<Real, N>& c)->unpack<tensor<Real, N>>;

//==============================================================================
template <typename Real, size_t N>
struct unpack<const tensor<Real, N>> {
  static constexpr size_t n = N;
  const tensor<Real, N>&  container;

  //----------------------------------------------------------------------------
  constexpr unpack(const tensor<Real, N>& c) : container{c} {}

  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr const auto& get() const {
    return container[I];
  }
};
//==============================================================================
template <typename Real, size_t N>
unpack(const tensor<Real, N>& c)->unpack<const tensor<Real, N>>;

//==============================================================================
}  // namespace tatooine
//==============================================================================

#endif
