#ifndef TATOOINE_GRID_H
#define TATOOINE_GRID_H
//==============================================================================
#include <tatooine/amira_file.h>
#include <tatooine/axis_aligned_bounding_box.h>
#include <tatooine/chunked_multidim_array.h>
#include <tatooine/concepts.h>
#include <tatooine/filesystem.h>
#include <tatooine/for_loop.h>
#include <tatooine/grid_vertex_container.h>
#include <tatooine/grid_vertex_property.h>
#include <tatooine/hdf5.h>
#include <tatooine/interpolation.h>
#include <tatooine/lazy_reader.h>
#include <tatooine/linspace.h>
#include <tatooine/netcdf.h>
#include <tatooine/random.h>
#include <tatooine/tags.h>
#include <tatooine/template_helper.h>
#include <tatooine/vec.h>

#include <map>
#include <memory>
#include <mutex>
#include <tuple>
//==============================================================================
namespace tatooine {
//==============================================================================
/// When using GCC you have to specify Dimensions types by hand. This is a known
/// GCC bug (80438)
#ifdef __cpp_concepts
template <indexable_space... Dimensions>
#else
template <typename... Dimensions>
#endif
class grid {
  static_assert(sizeof...(Dimensions) > 0,
                "Grid needs at least one dimension.");

 public:
  static constexpr bool is_uniform =
      (is_linspace<std::decay_t<Dimensions>> && ...);
  static constexpr auto num_dimensions() { return sizeof...(Dimensions); }
  using this_t = grid<Dimensions...>;
  using real_t = common_type<typename Dimensions::value_type...>;
  using vec_t  = vec<real_t, num_dimensions()>;
  using pos_t  = vec_t;
  using seq_t  = std::make_index_sequence<num_dimensions()>;

  using dimensions_t = std::tuple<std::decay_t<Dimensions>...>;

  using vertex_container = grid_vertex_container<Dimensions...>;

  // general property types
  using property_t = grid_vertex_property<this_t>;
  template <typename ValueType, bool HasNonConstReference>
  using typed_vertex_property_interface_t =
      typed_grid_vertex_property_interface<this_t, ValueType,
                                           HasNonConstReference>;
  template <typename Container>
  using typed_vertex_property_t =
      typed_vertex_property<this_t, typename Container::value_type, Container>;
  template <typename F>
  using invoke_result_with_indices =
      std::invoke_result_t<F, decltype(((void)std::declval<Dimensions>(),
                                        std::declval<size_t>()))...>;

  using property_ptr_t       = std::unique_ptr<property_t>;
  using property_container_t = std::map<std::string, property_ptr_t>;
  //============================================================================
  static constexpr size_t min_stencil_size = 2;
  static constexpr size_t max_stencil_size = 11;

 private:
  static constexpr size_t num_stencils =
      max_stencil_size - min_stencil_size + 1;
  mutable std::mutex   m_stencil_mutex;
  dimensions_t         m_dimensions;
  property_container_t m_vertex_properties;
  mutable bool         m_diff_stencil_coefficients_created_once = false;

  using stencil_t      = std::vector<real_t>;
  using stencil_list_t = std::vector<stencil_t>;
  mutable std::array<std::array<std::vector<stencil_list_t>, num_stencils>,
                     num_dimensions()>
         m_diff_stencil_coefficients;
  size_t m_chunk_size_for_lazy_properties = 2;
  //============================================================================
 public:
  constexpr grid() = default;
  constexpr grid(grid const& other)
      : m_dimensions{other.m_dimensions},
        m_diff_stencil_coefficients{other.m_diff_stencil_coefficients} {
    for (auto const& [name, prop] : other.m_vertex_properties) {
      auto& emplaced_prop =
          m_vertex_properties.emplace(name, prop->clone()).first->second;
      emplaced_prop->set_grid(*this);
    }
  }
  constexpr grid(grid&& other) noexcept
      : m_dimensions{std::move(other.m_dimensions)},
        m_vertex_properties{std::move(other.m_vertex_properties)},
        m_diff_stencil_coefficients{
            std::move(other.m_diff_stencil_coefficients)} {
    for (auto const& [name, prop] : m_vertex_properties) {
      prop->set_grid(*this);
    }
    for (auto const& [name, prop] : other.m_vertex_properties) {
      prop->set_grid(other);
    }
  }
  //----------------------------------------------------------------------------
  /// The enable if is needed due to gcc bug 80871. See here:
  /// https://stackoverflow.com/questions/46848129/variadic-deduction-guide-not-taken-by-g-taken-by-clang-who-is-correct
#ifdef __cpp_concepts
  template <typename... _Dimensions>
      requires(sizeof...(_Dimensions) == sizeof...(Dimensions)) &&
      (indexable_space<std::decay_t<_Dimensions>> && ...)
#else
  template <typename... _Dimensions,
            enable_if<(sizeof...(_Dimensions) == sizeof...(Dimensions))> = true,
            enable_if<is_indexable<std::decay_t<_Dimensions>...>>        = true>
#endif
          constexpr grid(_Dimensions&&... dimensions)
      : m_dimensions{std::forward<_Dimensions>(dimensions)...} {
    static_assert(sizeof...(_Dimensions) == num_dimensions(),
                  "Number of given dimensions does not match number of "
                  "specified dimensions.");
    static_assert(
        (std::is_same_v<std::decay_t<_Dimensions>, Dimensions> && ...),
        "Constructor dimension types differ class dimension types.");
  }
  //----------------------------------------------------------------------------
 private:
  template <typename Real, size_t... Seq>
  constexpr grid(axis_aligned_bounding_box<Real, num_dimensions()> const& bb,
                 std::array<size_t, num_dimensions()> const&              res,
                 std::index_sequence<Seq...> /*seq*/)
      : m_dimensions{linspace<real_t>{real_t(bb.min(Seq)), real_t(bb.max(Seq)),
                                      res[Seq]}...} {}
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  template <typename Real>
  constexpr grid(axis_aligned_bounding_box<Real, num_dimensions()> const& bb,
                 std::array<size_t, num_dimensions()> const&              res)
      : grid{bb, res, seq_t{}} {}
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
  template <integral... Size>
#else
  template <typename... Size, enable_if<is_integral<Size...>> = true>
#endif
  constexpr grid(Size const... size)
      : grid{linspace{0.0, 1.0, static_cast<size_t>(size)}...} {
    assert(((size >= 0) && ...));
  }
  //----------------------------------------------------------------------------
  grid(filesystem::path const& path) { read(path); }
  //----------------------------------------------------------------------------
  ~grid() = default;
  //============================================================================
 private:
  template <size_t... Ds>
  constexpr auto copy_without_properties(
      std::index_sequence<Ds...> /*seq*/) const {
    return this_t{std::get<Ds>(m_dimensions)...};
  }

 public:
  constexpr auto copy_without_properties() const {
    return copy_without_properties(
        std::make_index_sequence<num_dimensions()>{});
  }
  //============================================================================
  constexpr auto operator=(grid const& other) -> grid& = default;
  constexpr auto operator=(grid&& other) noexcept -> grid& = default;
  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto dimension() -> auto& {
    static_assert(I < num_dimensions());
    return std::get<I>(m_dimensions);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t I>
  constexpr auto dimension() const -> auto const& {
    static_assert(I < num_dimensions());
    return std::get<I>(m_dimensions);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <bool Cond = is_same<Dimensions...>, enable_if<Cond> = true>
  constexpr auto dimension(size_t const i) -> auto& {
    if (i == 0) {
      return dimension<0>();
    }
    if constexpr (num_dimensions() > 1) {
      if (i == 1) {
        return dimension<1>();
      }
    }
    if constexpr (num_dimensions() > 2) {
      if (i == 2) {
        return dimension<2>();
      }
    }
    if constexpr (num_dimensions() > 3) {
      if (i == 3) {
        return dimension<3>();
      }
    }
    if constexpr (num_dimensions() > 4) {
      if (i == 4) {
        return dimension<4>();
      }
    }
    if constexpr (num_dimensions() > 5) {
      if (i == 5) {
        return dimension<5>();
      }
    }
    if constexpr (num_dimensions() > 6) {
      if (i == 6) {
        return dimension<6>();
      }
    }
    if constexpr (num_dimensions() > 7) {
      if (i == 7) {
        return dimension<7>();
      }
    }
    if constexpr (num_dimensions() > 8) {
      if (i == 8) {
        return dimension<8>();
      }
    }
    if constexpr (num_dimensions() > 9) {
      if (i == 9) {
        return dimension<9>();
      }
    }
    if constexpr (num_dimensions() > 10) {
      if (i == 10) {
        return dimension<10>();
      }
    }
    return dimension<0>();
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <bool Cond = is_same<Dimensions...>, enable_if<Cond> = true>
  constexpr auto dimension(size_t const i) const -> auto const& {
    if (i == 0) {
      return dimension<0>();
    }
    if constexpr (num_dimensions() > 1) {
      if (i == 1) {
        return dimension<1>();
      }
    }
    if constexpr (num_dimensions() > 2) {
      if (i == 2) {
        return dimension<2>();
      }
    }
    if constexpr (num_dimensions() > 3) {
      if (i == 3) {
        return dimension<3>();
      }
    }
    if constexpr (num_dimensions() > 4) {
      if (i == 4) {
        return dimension<4>();
      }
    }
    if constexpr (num_dimensions() > 5) {
      if (i == 5) {
        return dimension<5>();
      }
    }
    if constexpr (num_dimensions() > 6) {
      if (i == 6) {
        return dimension<6>();
      }
    }
    if constexpr (num_dimensions() > 7) {
      if (i == 7) {
        return dimension<7>();
      }
    }
    if constexpr (num_dimensions() > 8) {
      if (i == 8) {
        return dimension<8>();
      }
    }
    if constexpr (num_dimensions() > 9) {
      if (i == 9) {
        return dimension<9>();
      }
    }
    if constexpr (num_dimensions() > 10) {
      if (i == 10) {
        return dimension<10>();
      }
    }
    return dimension<0>();
  }
  //----------------------------------------------------------------------------
  constexpr auto dimensions() -> auto& { return m_dimensions; }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr auto dimensions() const -> auto const& { return m_dimensions; }
  //----------------------------------------------------------------------------
  constexpr auto front_dimension() -> auto& {
    return std::get<0>(m_dimensions);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr auto front_dimension() const -> auto const& {
    return std::get<0>(m_dimensions);
  }
  //----------------------------------------------------------------------------
 private:
  template <size_t... Seq>
  constexpr auto min(std::index_sequence<Seq...> /*seq*/) const {
    return vec<real_t, num_dimensions()>{static_cast<real_t>(front<Seq>())...};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  constexpr auto min() const { return min(seq_t{}); }
  //----------------------------------------------------------------------------
 private:
  template <size_t... Seq>
  constexpr auto max(std::index_sequence<Seq...> /*seq*/) const {
    return vec<real_t, num_dimensions()>{static_cast<real_t>(back<Seq>())...};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  constexpr auto max() const { return max(seq_t{}); }
  //----------------------------------------------------------------------------
 private:
  template <size_t... Seq>
  constexpr auto resolution(std::index_sequence<Seq...> /*seq*/) const {
    return vec<size_t, num_dimensions()>{size<Seq>()...};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  constexpr auto resolution() const { return resolution(seq_t{}); }
  //----------------------------------------------------------------------------
 private:
  template <size_t... Seq>
  constexpr auto bounding_box(std::index_sequence<Seq...> /*seq*/) const {
    static_assert(sizeof...(Seq) == num_dimensions());
    return axis_aligned_bounding_box<real_t, num_dimensions()>{
        vec<real_t, num_dimensions()>{static_cast<real_t>(front<Seq>())...},
        vec<real_t, num_dimensions()>{static_cast<real_t>(back<Seq>())...}};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  constexpr auto bounding_box() const { return bounding_box(seq_t{}); }
  //----------------------------------------------------------------------------
 private:
  template <size_t... Seq>
  constexpr auto size(std::index_sequence<Seq...> /*seq*/) const {
    static_assert(sizeof...(Seq) == num_dimensions());
    return std::array{size<Seq>()...};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  constexpr auto size() const { return size(seq_t{}); }
  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto size() const {
    return dimension<I>().size();
  }
  //----------------------------------------------------------------------------
  constexpr auto size(size_t const i) const {
    if (i == 0) {
      return size<0>();
    }
    if constexpr (num_dimensions() > 1) {
      if (i == 1) {
        return size<1>();
      }
    }
    if constexpr (num_dimensions() > 2) {
      if (i == 2) {
        return size<2>();
      }
    }
    if constexpr (num_dimensions() > 3) {
      if (i == 3) {
        return size<3>();
      }
    }
    if constexpr (num_dimensions() > 4) {
      if (i == 4) {
        return size<4>();
      }
    }
    if constexpr (num_dimensions() > 5) {
      if (i == 5) {
        return size<5>();
      }
    }
    if constexpr (num_dimensions() > 6) {
      if (i == 6) {
        return size<6>();
      }
    }
    if constexpr (num_dimensions() > 7) {
      if (i == 7) {
        return size<7>();
      }
    }
    if constexpr (num_dimensions() > 8) {
      if (i == 8) {
        return size<8>();
      }
    }
    if constexpr (num_dimensions() > 9) {
      if (i == 9) {
        return size<9>();
      }
    }
    if constexpr (num_dimensions() > 10) {
      if (i == 10) {
        return size<10>();
      }
    }
    return std::numeric_limits<size_t>::max();
  }
  //----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <size_t I>
  requires std::is_reference_v<template_helper::get_t<I, Dimensions...>>
#else
  template <size_t I, enable_if<std::is_reference_v<
                          template_helper::get_t<I, Dimensions...>>> = true>
#endif
      constexpr auto size() -> auto& {
    return dimension<I>().size();
  }
  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto front() const {
    return dimension<I>().front();
  }
  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto front() -> auto& {
    return dimension<I>().front();
  }
  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto back() const {
    return dimension<I>().back();
  }
  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto back() -> auto& {
    return dimension<I>().back();
  }
  //----------------------------------------------------------------------------
  template <size_t I>
  constexpr auto dim_center() {
    return (dimension<I>().back() + dimension<I>().front()) / 2;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 private:
  template <size_t... Is>
  constexpr auto center(std::index_sequence<Is...> /*seq*/) {
    return vec{dim_center<Is>()...};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  constexpr auto center() {
    return center(std::make_index_sequence<num_dimensions()>{});
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr auto center(size_t const i) {
    if (i == 0) {
      return dim_center<0>();
    }
    if constexpr (num_dimensions() > 1) {
      if (i == 1) {
        return dim_center<1>();
      }
    }
    if constexpr (num_dimensions() > 2) {
      if (i == 2) {
        return dim_center<2>();
      }
    }
    if constexpr (num_dimensions() > 3) {
      if (i == 3) {
        return dim_center<3>();
      }
    }
    if constexpr (num_dimensions() > 4) {
      if (i == 4) {
        return dim_center<4>();
      }
    }
    if constexpr (num_dimensions() > 5) {
      if (i == 5) {
        return dim_center<5>();
      }
    }
    if constexpr (num_dimensions() > 6) {
      if (i == 6) {
        return dim_center<6>();
      }
    }
    if constexpr (num_dimensions() > 7) {
      if (i == 7) {
        return dim_center<7>();
      }
    }
    if constexpr (num_dimensions() > 8) {
      if (i == 8) {
        return dim_center<8>();
      }
    }
    if constexpr (num_dimensions() > 9) {
      if (i == 9) {
        return dim_center<9>();
      }
    }
    if constexpr (num_dimensions() > 10) {
      if (i == 10) {
        return dim_center<10>();
      }
    }
    return std::numeric_limits<size_t>::max();
  }
  //----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <arithmetic... Comps, size_t... Seq>
  requires(num_dimensions() == sizeof...(Comps))
#else
  template <typename... Comps, size_t... Seq,
            enable_if<is_arithmetic<Comps...>>                = true,
            enable_if<(num_dimensions() == sizeof...(Comps))> = true>
#endif
      constexpr auto is_inside(std::index_sequence<Seq...> /*seq*/,
                               Comps const... comps) const {
    return ((front<Seq>() <= comps && comps <= back<Seq>()) && ...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
  template <arithmetic... Comps>
  requires(num_dimensions() == sizeof...(Comps))
#else
  template <typename... Comps, enable_if<is_arithmetic<Comps...>> = true,
            enable_if<(num_dimensions() == sizeof...(Comps))> = true>
#endif
      constexpr auto is_inside(Comps const... comps) const {
    return is_inside(std::make_index_sequence<num_dimensions()>{}, comps...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t... Seq>
  constexpr auto is_inside(pos_t const& p,
                           std::index_sequence<Seq...> /*seq*/) const {
    return ((front<Seq>() <= p(Seq) && p(Seq) <= back<Seq>()) && ...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr auto is_inside(pos_t const& p) const {
    return is_inside(p, std::make_index_sequence<num_dimensions()>{});
  }
  //----------------------------------------------------------------------------
 private:
#ifdef __cpp_concepts
  template <size_t... Seq, arithmetic... Xs>
#else
  template <size_t... Seq, typename... Xs,
            enable_if<is_arithmetic<Xs...>> = true>
#endif
  constexpr auto in_domain(std::index_sequence<Seq...> /*seq*/,
                           Xs const... xs) const {
    static_assert(sizeof...(xs) == num_dimensions(),
                  "number of components does not match number of dimensions");
    static_assert(sizeof...(Seq) == num_dimensions(),
                  "number of indices does not match number of dimensions");
    return ((front<Seq>() <= xs) && ...) && ((xs <= back<Seq>()) && ...);
  }
  //----------------------------------------------------------------------------
 public:
#ifdef __cpp_concepts
  template <size_t... Is, arithmetic... Xs>
#else
  template <size_t... Is, typename... Xs,
            enable_if<is_arithmetic<Xs...>> = true>
#endif
  constexpr auto in_domain(Xs const... xs) const {
    static_assert(sizeof...(xs) == num_dimensions(),
                  "number of components does not match number of dimensions");
    return in_domain(seq_t{}, xs...);
  }

  //----------------------------------------------------------------------------
 private:
  template <size_t... Seq>
  constexpr auto in_domain(std::array<real_t, num_dimensions()> const& x,
                           std::index_sequence<Seq...> /*seq*/) const {
    return in_domain(x[Seq]...);
  }
  //----------------------------------------------------------------------------
 public:
  constexpr auto in_domain(
      std::array<real_t, num_dimensions()> const& x) const {
    return in_domain(x, seq_t{});
  }
  //----------------------------------------------------------------------------
  /// returns cell index and factor for interpolation
#ifdef __cpp_concepts
  template <arithmetic X>
#else
  template <typename X, enable_if<is_arithmetic<X>> = true>
#endif
  auto cell_index(size_t const dimension_index, X const x) const {
    if (dimension_index == 0) {
      return cell_index<0>(x);
    }
    if constexpr (num_dimensions() > 1) {
      if (dimension_index == 1) {
        return cell_index<1>(x);
      }
    }
    if constexpr (num_dimensions() > 2) {
      if (dimension_index == 2) {
        return cell_index<2>(x);
      }
    }
    if constexpr (num_dimensions() > 3) {
      if (dimension_index == 3) {
        return cell_index<3>(x);
      }
    }
    if constexpr (num_dimensions() > 4) {
      if (dimension_index == 4) {
        return cell_index<4>(x);
      }
    }
    if constexpr (num_dimensions() > 5) {
      if (dimension_index == 5) {
        return cell_index<5>(x);
      }
    }
    if constexpr (num_dimensions() > 6) {
      if (dimension_index == 6) {
        return cell_index<6>(x);
      }
    }
    if constexpr (num_dimensions() > 7) {
      if (dimension_index == 7) {
        return cell_index<7>(x);
      }
    }
    if constexpr (num_dimensions() > 8) {
      if (dimension_index == 8) {
        return cell_index<8>(x);
      }
    }
    if constexpr (num_dimensions() > 9) {
      if (dimension_index == 9) {
        return cell_index<9>(x);
      }
    }
    if constexpr (num_dimensions() > 10) {
      if (dimension_index == 10) {
        return cell_index<10>(x);
      }
    }
    return cell_index<0>(x);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// returns cell index and factor for interpolation
#ifdef __cpp_concepts
  template <size_t DimensionIndex, arithmetic X>
#else
  template <size_t DimensionIndex, typename X,
            enable_if<is_arithmetic<X>> = true>
#endif
  auto cell_index(X x) const -> std::pair<size_t, real_t> {
    auto const& dim = dimension<DimensionIndex>();
    if (std::abs(x - dim.front()) < 1e-10) {
      x = dim.front();
    }
    if (std::abs(x - dim.back()) < 1e-10) {
      x = dim.back();
    }
    if constexpr (is_linspace<std::decay_t<decltype(dim)>>) {
      // calculate
      auto pos =
          (x - dim.front()) / (dim.back() - dim.front()) * (dim.size() - 1);
      auto quantized_pos = static_cast<size_t>(std::floor(pos));
      if (quantized_pos == dim.size() - 1) {
        --quantized_pos;
      }
      auto cell_position = pos - quantized_pos;
      if (quantized_pos == dim.size() - 1) {
        --quantized_pos;
        cell_position = 1;
      }
      return {quantized_pos, cell_position};
    } else {
      // binary search
      size_t left  = 0;
      size_t right = dim.size() - 1;
      while (right - left > 1) {
        auto const center = (right + left) / 2;
        if (x < dim[center]) {
          right = center;
        } else {
          left = center;
        }
      }
      return {left, (x - dim[left]) / (dim[left + 1] - dim[left])};
    }
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// returns cell indices and factors for each dimension for interpolaton
#ifdef __cpp_concepts
  template <size_t... DimensionIndex, arithmetic... Xs>
#else
  template <size_t... DimensionIndex, typename... Xs,
            enable_if<is_arithmetic<Xs...>> = true>
#endif
  auto cell_index(std::index_sequence<DimensionIndex...>, Xs const... xs) const
      -> std::array<std::pair<size_t, double>, num_dimensions()> {
    return std::array{cell_index<DimensionIndex>(static_cast<double>(xs))...};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
  template <arithmetic... Xs>
#else
  template <typename... Xs, enable_if<is_arithmetic<Xs...>> = true>
#endif
  auto cell_index(Xs const... xs) const {
    return cell_index(seq_t{}, xs...);
  }
  //----------------------------------------------------------------------------
  auto diff_stencil_coefficients(size_t const dim_index,
                                 size_t const stencil_size,
                                 size_t const stencil_center,
                                 size_t const i) const -> auto const& {
    return m_diff_stencil_coefficients[dim_index]
                                      [stencil_size - min_stencil_size]
                                      [stencil_center][i];
  }
  //----------------------------------------------------------------------------
  auto diff_stencil_coefficients_created_once() const {
    std::lock_guard lock{m_stencil_mutex};
    return m_diff_stencil_coefficients_created_once;
  }
  //----------------------------------------------------------------------------
  auto update_diff_stencil_coefficients() const {
    std::lock_guard lock{m_stencil_mutex};
    for (size_t dim = 0; dim < num_dimensions(); ++dim) {
      for (size_t i = 0; i < num_stencils; ++i) {
        m_diff_stencil_coefficients[dim][i].clear();
        m_diff_stencil_coefficients[dim][i].shrink_to_fit();
      }
    }
    update_diff_stencil_coefficients(
        std::make_index_sequence<num_dimensions()>{});
    m_diff_stencil_coefficients_created_once = true;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t... DimSeq>
  auto update_diff_stencil_coefficients(
      std::index_sequence<DimSeq...> /*seq*/) const {
    [[maybe_unused]] auto const dim_seqs = std::array{DimSeq...};
    (update_diff_stencil_coefficients_dim<DimSeq>(), ...);
  }
  //----------------------------------------------------------------------------
  template <size_t Dim>
  auto update_diff_stencil_coefficients_dim() const {
    // if constexpr (!is_linspace<std::decay_t<decltype(dimension<Dim>())>>) {
    auto const& dim                   = dimension<Dim>();
    auto&       stencils_of_dimension = m_diff_stencil_coefficients[Dim];
    for (size_t stencil_size = min_stencil_size;
         stencil_size <= std::min(max_stencil_size, dim.size());
         ++stencil_size) {
      auto& stencils_of_cur_size =
          stencils_of_dimension[stencil_size - min_stencil_size];
      stencils_of_cur_size.resize(stencil_size);
      for (size_t stencil_center = 0; stencil_center < stencil_size;
           ++stencil_center) {
        update_diff_stencil_coefficients(dim,
                                         stencils_of_cur_size[stencil_center],
                                         stencil_size, stencil_center);
      }
    }
    //}
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename Dim>
  auto update_diff_stencil_coefficients(
      Dim const& dim, std::vector<std::vector<real_t>>& stencils,
      size_t const stencil_size, size_t const stencil_center) const {
    stencils.reserve(dim.size());

    for (size_t i = 0; i < stencil_center; ++i) {
      stencils.emplace_back();
    }
    for (size_t i = stencil_center;
         i < dim.size() - (stencil_size - stencil_center - 1); ++i) {
      std::vector<real_t> xs(stencil_size);
      for (size_t j = 0; j < stencil_size; ++j) {
        xs[j] = dim[i - stencil_center + j] - dim[i];
      }
      stencils.push_back(finite_differences_coefficients(1, xs));
    }
    // for (size_t i = 0; i < stencil_size - stencil_center - 1; ++i) {
    //  stencils.emplace_back();
    //}
  }
  //----------------------------------------------------------------------------
  /// \return number of dimensions for one dimension dim
  // constexpr auto edges() const { return grid_edge_container{this}; }

  //----------------------------------------------------------------------------
  auto vertices() const { return vertex_container{*this}; }
  //----------------------------------------------------------------------------
 private:
#ifdef __cpp_concepts
  template <indexable_space AdditionalDimension, size_t... Is>
#else
  template <typename AdditionalDimension, size_t... Is>
#endif
  auto add_dimension(AdditionalDimension&& additional_dimension,
                     std::index_sequence<Is...> /*seq*/) const {
    return grid<Dimensions..., std::decay_t<AdditionalDimension>>{
        dimension<Is>()...,
        std::forward<AdditionalDimension>(additional_dimension)};
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
#ifdef __cpp_concepts
  template <indexable_space AdditionalDimension>
#else
  template <typename AdditionalDimension>
#endif
  auto add_dimension(AdditionalDimension&& additional_dimension) const {
    return add_dimension(
        std::forward<AdditionalDimension>(additional_dimension), seq_t{});
  }
  //----------------------------------------------------------------------------
  auto remove_vertex_property(std::string const& name) -> void {
    if (auto it = m_vertex_properties.find(name);
        it != end(m_vertex_properties)) {
      m_vertex_properties.erase(it);
    }
  }
  //----------------------------------------------------------------------------
  auto rename_vertex_property(std::string const& current_name,
                              std::string const& new_name) -> void {
    if (auto it = m_vertex_properties.find(current_name);
        it != end(m_vertex_properties)) {
      auto handler  = m_vertex_properties.extract(it);
      handler.key() = new_name;
      m_vertex_properties.insert(std::move(handler));
    }
  }
  //----------------------------------------------------------------------------
  template <typename Container, typename... Args>
  auto create_vertex_property(std::string const& name, Args&&... args)
      -> auto& {
    if (auto it = m_vertex_properties.find(name);
        it == end(m_vertex_properties)) {
      auto new_prop = new typed_vertex_property_t<Container>{
          *this, std::forward<Args>(args)...};
      m_vertex_properties.emplace(name, std::unique_ptr<property_t>{new_prop});
      if constexpr (sizeof...(Args) == 0) {
        new_prop->resize(size());
      }
      return *new_prop;
    }
    throw std::runtime_error{"There is already a vertex property named \"" +
                             name + "\"."};
  }
  //----------------------------------------------------------------------------
  auto vertex_properties() const -> auto const& { return m_vertex_properties; }
  auto vertex_properties() -> auto& { return m_vertex_properties; }
  //----------------------------------------------------------------------------
  template <typename T, typename IndexOrder = x_fastest>
  auto insert_vertex_property(std::string const& name) -> auto& {
    return insert_contiguous_vertex_property<T, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto insert_scalar_vertex_property(std::string const& name) -> auto& {
    return insert_vertex_property<tatooine::real_t, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto insert_vec2_vertex_property(std::string const& name) -> auto& {
    return insert_vertex_property<vec2, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto insert_vec3_vertex_property(std::string const& name) -> auto& {
    return insert_vertex_property<vec3, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto insert_vec4_vertex_property(std::string const& name) -> auto& {
    return insert_vertex_property<vec4, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto insert_mat2_vertex_property(std::string const& name) -> auto& {
    return insert_vertex_property<mat2, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto insert_mat3_vertex_property(std::string const& name) -> auto& {
    return insert_vertex_property<mat3, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto insert_mat4_vertex_property(std::string const& name) -> auto& {
    return insert_vertex_property<mat4, IndexOrder>(name);
  }
  //----------------------------------------------------------------------------
  /// @}
  //----------------------------------------------------------------------------
  template <typename T, typename IndexOrder = x_fastest>
  auto insert_contiguous_vertex_property(std::string const& name) -> auto& {
    return create_vertex_property<dynamic_multidim_array<T, IndexOrder>>(
        name, size());
  }
  //----------------------------------------------------------------------------
  template <typename T, typename IndexOrder = x_fastest>
  auto insert_chunked_vertex_property(std::string const&         name,
                                      std::vector<size_t> const& chunk_size)
      -> auto& {
    return create_vertex_property<chunked_multidim_array<T, IndexOrder>>(
        name, size(), chunk_size);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename T, typename IndexOrder = x_fastest>
  auto insert_chunked_vertex_property(
      std::string const&                          name,
      std::array<size_t, num_dimensions()> const& chunk_size) -> auto& {
    return create_vertex_property<chunked_multidim_array<T, IndexOrder>>(
        name, size(), chunk_size);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
  template <typename T, typename IndexOrder = x_fastest, integral... ChunkSize>
  requires(sizeof...(ChunkSize) == num_dimensions())
#else
  template <typename T, typename IndexOrder = x_fastest, typename... ChunkSize,
            enable_if<is_integral<ChunkSize...>>                  = true,
            enable_if<(sizeof...(ChunkSize) == num_dimensions())> = true>
#endif
      auto insert_chunked_vertex_property(std::string const& name,
                                          ChunkSize const... chunk_size)
          -> auto& {
    return create_vertex_property<chunked_multidim_array<T, IndexOrder>>(
        name, size(), std::vector<size_t>{static_cast<size_t>(chunk_size)...});
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename T, typename IndexOrder = x_fastest>
  auto insert_chunked_vertex_property(std::string const& name) -> auto& {
    return create_vertex_property<chunked_multidim_array<T, IndexOrder>>(
        name, size(), make_array<num_dimensions()>(size_t(10)));
  }
  //----------------------------------------------------------------------------
  template <typename T, bool HasNonConstReference = true>
  auto vertex_property(std::string const& name)
      -> typed_vertex_property_interface_t<T, HasNonConstReference>& {
    if (auto it = m_vertex_properties.find(name);
        it == end(m_vertex_properties)) {
      return insert_vertex_property<T>(name);
    } else {
      if (typeid(T) != it->second->type()) {
        throw std::runtime_error{
            "type of property \"" + name + "\"(" +
            boost::core::demangle(it->second->type().name()) +
            ") does not match specified type " + type_name<T>() + "."};
      }
      return *dynamic_cast<
          typed_vertex_property_interface_t<T, HasNonConstReference>*>(
          it->second.get());
    }
  }
  //----------------------------------------------------------------------------
  template <typename T, bool HasNonConstReference = true>
  auto vertex_property(std::string const& name) const
      -> typed_vertex_property_interface_t<T, HasNonConstReference> const& {
    if (auto it = m_vertex_properties.find(name);
        it == end(m_vertex_properties)) {
      throw std::runtime_error{"property \"" + name + "\" not found"};
    } else {
      if (typeid(T) != it->second->type()) {
        throw std::runtime_error{
            "type of property \"" + name + "\"(" +
            boost::core::demangle(it->second->type().name()) +
            ") does not match specified type " + type_name<T>() + "."};
      }
      return *dynamic_cast<
          typed_vertex_property_interface_t<T, HasNonConstReference> const*>(
          it->second.get());
    }
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto scalar_vertex_property(std::string const& name) const -> auto const& {
    return vertex_property<tatooine::real_t, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto scalar_vertex_property(std::string const& name) -> auto& {
    return vertex_property<tatooine::real_t, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto vec2_vertex_property(std::string const& name) const -> auto const& {
    return vertex_property<vec2, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto vec2_vertex_property(std::string const& name) -> auto& {
    return vertex_property<vec2, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto vec3_vertex_property(std::string const& name) const -> auto const& {
    return vertex_property<vec3, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto vec3_vertex_property(std::string const& name) -> auto& {
    return vertex_property<vec3, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto vec4_vertex_property(std::string const& name) const -> auto const& {
    return vertex_property<vec4, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto vec4_vertex_property(std::string const& name) -> auto& {
    return vertex_property<vec4, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto mat2_vertex_property(std::string const& name) const -> auto const& {
    return vertex_property<mat2, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto mat2_vertex_property(std::string const& name) -> auto& {
    return vertex_property<mat2, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto mat3_vertex_property(std::string const& name) const -> auto const& {
    return vertex_property<mat3, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  template <bool HasNonConstReference = true>
  auto mat3_vertex_property(std::string const& name) -> auto& {
    return vertex_property<mat3, HasNonConstReference>(name);
  }
  //----------------------------------------------------------------------------
  // template <invocable<decltype(((void)std::declval<Dimensions>(),
  //                              std::declval<size_t>()))...>
  //              F>
  // auto insert_vertex_property(std::string const& name, F&& f) -> auto& {
  //  return create_vertex_property<
  //      typed_grid_vertex_property_lambda<this_t, std::decay_t<F>>>(
  //      name, std::forward<F>(f));
  //}
  //----------------------------------------------------------------------------
  template <typename T, typename GlobalIndexOrder = x_fastest,
            typename LocalIndexOrder = GlobalIndexOrder>
  auto insert_lazy_vertex_property(filesystem::path const& path,
                                   std::string const&      dataset_name)
      -> typed_vertex_property_interface_t<T, false>& {
    auto const ext = path.extension();
#ifdef TATOOINE_HDF5_AVAILABLE
    if (ext == ".h5") {
      return insert_hdf5_lazy_vertex_property<T, GlobalIndexOrder,
                                              LocalIndexOrder>(path,
                                                               dataset_name);
    }
#endif
#ifdef TATOOINE_NETCDF_AVAILABLE
    if (ext == ".nc") {
      return insert_netcdf_lazy_vertex_property<T, GlobalIndexOrder,
                                                LocalIndexOrder>(path,
                                                                 dataset_name);
    }
#endif
    throw std::runtime_error{
        "[grid::insert_lazy_vertex_property] - unknown file extension"};
  }
  //----------------------------------------------------------------------------
#ifdef TATOOINE_HDF5_AVAILABLE
  template <typename IndexOrder = x_fastest, typename T>
  auto insert_vertex_property(hdf5::dataset<T> const& dataset) -> auto& {
    return insert_vertex_property<IndexOrder>(dataset, dataset.name());
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest, typename T>
  auto insert_vertex_property(hdf5::dataset<T> const& dataset,
                              std::string const&      name) -> auto& {
    auto num_dims_dataset = dataset.num_dimensions();
    if (num_dimensions() != num_dims_dataset) {
      throw std::runtime_error{
          "Number of dimensions do not match for HDF5 dataset and grid."};
    }
    auto size_dataset = dataset.size();
    for (size_t i = 0; i < num_dimensions(); ++i) {
      if (size_dataset[i] != size(i)) {
        std::stringstream ss;
        ss << "Resolution of grid and HDF5 DataSet do not match. grid("
           << size(0);
        for (size_t i = 1; i < num_dimensions(); ++i) {
          ss << ", " << size(i);
        }
        ss << ") and hdf5 dataset(" << size_dataset[i];
        for (size_t i = 1; i < num_dimensions(); ++i) {
          ss << ", " << size_dataset[i];
        }
        ss << ")";
        throw std::runtime_error{ss.str()};
      }
    }
    auto& prop = insert_vertex_property<T, IndexOrder>(name);
    dataset.read(prop);
    return prop;
  }
  //----------------------------------------------------------------------------
  template <typename T, typename GlobalIndexOrder = x_fastest,
            typename LocalIndexOrder = GlobalIndexOrder>
  auto insert_hdf5_lazy_vertex_property(filesystem::path const& path,
                                        std::string const&      dataset_name)
      -> auto& {
    hdf5::file f{path};
    return insert_lazy_vertex_property<GlobalIndexOrder, LocalIndexOrder>(
        f.dataset<T>(dataset_name));
  }
  //----------------------------------------------------------------------------
  template <typename GlobalIndexOrder = x_fastest,
            typename LocalIndexOrder  = GlobalIndexOrder, typename T>
  auto insert_lazy_vertex_property(hdf5::dataset<T> const& dataset) -> auto& {
    return insert_lazy_vertex_property<GlobalIndexOrder, LocalIndexOrder>(
        dataset, dataset.name());
  }
  //----------------------------------------------------------------------------
  template <typename GlobalIndexOrder = x_fastest,
            typename LocalIndexOrder  = GlobalIndexOrder, typename T>
  auto insert_lazy_vertex_property(hdf5::dataset<T> const& dataset,
                                   std::string const&      name) -> auto& {
    auto num_dims_dataset = dataset.num_dimensions();
    if (num_dimensions() != num_dims_dataset) {
      throw std::runtime_error{
          "Number of dimensions do not match for HDF5 dataset and grid."};
    }
    auto size_dataset = dataset.size();
    for (size_t i = 0; i < num_dimensions(); ++i) {
      if (size_dataset[i] != size(i)) {
        std::stringstream ss;
        ss << "Resolution of grid and HDF5 DataSet (\"" << name
           << "\")do not match. grid(" << size(0);
        for (size_t i = 1; i < num_dimensions(); ++i) {
          ss << ", " << size(i);
        }
        ss << ") and hdf5 dataset(" << size_dataset[i];
        for (size_t i = 1; i < num_dimensions(); ++i) {
          ss << ", " << size_dataset[i];
        }
        ss << ")";
        throw std::runtime_error{ss.str()};
      }
    }
    return create_vertex_property<
        lazy_reader<hdf5::dataset<T>, GlobalIndexOrder, LocalIndexOrder>>(
        name, dataset,
        std::vector<size_t>(num_dimensions(),
                            m_chunk_size_for_lazy_properties));
  }
#endif
#ifdef TATOOINE_NETCDF_AVAILABLE
  //----------------------------------------------------------------------------
  template <typename T, typename GlobalIndexOrder = x_fastest,
            typename LocalIndexOrder = GlobalIndexOrder>
  auto insert_netcdf_lazy_vertex_property(filesystem::path const& path,
                                          std::string const&      dataset_name)
      -> auto& {
    netcdf::file f{path, netCDF::NcFile::read};
    return insert_lazy_vertex_property<GlobalIndexOrder, LocalIndexOrder, T>(
        f.variable<T>(dataset_name));
  }
  //----------------------------------------------------------------------------
  template <typename GlobalIndexOrder = x_fastest,
            typename LocalIndexOrder  = GlobalIndexOrder, typename T>
  auto insert_lazy_vertex_property(netcdf::variable<T> const& dataset)
      -> auto& {
    return create_vertex_property<
        lazy_reader<netcdf::variable<T>, GlobalIndexOrder, LocalIndexOrder>>(
        dataset.name(), dataset,
        std::vector<size_t>(num_dimensions(),
                            m_chunk_size_for_lazy_properties));
  }
#endif
  //============================================================================
#ifdef __cpp_concepts
  template <invocable<pos_t> F>
#else
  template <typename F, enable_if<is_invocable<F, pos_t>> = true>
#endif
  auto sample_to_vertex_property(F&& f, std::string const& name) -> auto& {
    using T    = std::invoke_result_t<F, pos_t>;
    auto& prop = insert_vertex_property<T>(name);
    iterate_over_vertex_indices([&](auto const... is) {
      try {
        prop(is...) = f(vertex_at(is...));
      } catch (std::exception&) {
        if constexpr (num_components<T> == 1) {
          prop(is...) = T{0.0 / 0.0};
        } else {
          prop(is...) = T{tag::fill{0.0 / 0.0}};
        }
      }
    });
    return prop;
  }
  //============================================================================
  auto read(filesystem::path const& path) {
#ifdef TATOOINE_NETCDF_AVAILABLE
    if (path.extension() == ".nc") {
      read_netcdf(path);
      return;
    }
#endif
    if constexpr (num_dimensions() == 2 || num_dimensions() == 3) {
      if (path.extension() == ".vtk") {
        read_vtk(path);
        return;
      }
      if (path.extension() == ".am") {
        read_amira(path);
        return;
      }
    }
    throw std::runtime_error{"[grid::read] Unknown file extension."};
  }
  //----------------------------------------------------------------------------
  struct vtk_listener : vtk::legacy_file_listener {
    this_t& gr;
    bool&   is_structured_points;
    vec3&   spacing;
    vtk_listener(this_t& gr_, bool& is_structured_points_, vec3& spacing_)
        : gr{gr_},
          is_structured_points{is_structured_points_},
          spacing{spacing_} {}
    // header data
    auto on_dataset_type(vtk::dataset_type t) -> void override {
      if (t == vtk::dataset_type::structured_points && !is_uniform) {
        is_structured_points = true;
      }
    }

    // coordinate data
    auto on_origin(double x, double y, double z) -> void override {
      gr.front<0>() = x;
      gr.front<1>() = y;
      if (num_dimensions() < 3 && z > 1) {
        throw std::runtime_error{
            "[grid::read_vtk] number of dimensions is < 3 but got third "
            "dimension."};
      }
      if constexpr (num_dimensions() > 3) {
        gr.front<2>() = z;
      }
    }
    auto on_spacing(double x, double y, double z) -> void override {
      spacing = {x, y, z};
    }
    auto on_dimensions(size_t x, size_t y, size_t z) -> void override {
      gr.dimension<0>().resize(x);
      gr.dimension<1>().resize(y);
      if (num_dimensions() < 3 && z > 1) {
        throw std::runtime_error{
            "[grid::read_vtk] number of dimensions is < 3 but got third "
            "dimension."};
      }
      if constexpr (num_dimensions() > 2) {
        gr.dimension<2>().resize(z);
      }
    }
    auto on_x_coordinates(std::vector<float> const & /*xs*/) -> void override {}
    auto on_x_coordinates(std::vector<double> const & /*xs*/) -> void override {
    }
    auto on_y_coordinates(std::vector<float> const & /*ys*/) -> void override {}
    auto on_y_coordinates(std::vector<double> const & /*ys*/) -> void override {
    }
    auto on_z_coordinates(std::vector<float> const & /*zs*/) -> void override {}
    auto on_z_coordinates(std::vector<double> const & /*zs*/) -> void override {
    }

    // index data
    auto on_cells(std::vector<int> const&) -> void override {}
    auto on_cell_types(std::vector<vtk::cell_type> const&) -> void override {}
    auto on_vertices(std::vector<int> const&) -> void override {}
    auto on_lines(std::vector<int> const&) -> void override {}
    auto on_polygons(std::vector<int> const&) -> void override {}
    auto on_triangle_strips(std::vector<int> const&) -> void override {}

    // cell- / pointdata
    auto on_vectors(std::string const& /*name*/,
                    std::vector<std::array<float, 3>> const& /*vectors*/,
                    vtk::reader_data) -> void override {}
    auto on_vectors(std::string const& /*name*/,
                    std::vector<std::array<double, 3>> const& /*vectors*/,
                    vtk::reader_data) -> void override {}
    auto on_normals(std::string const& /*name*/,
                    std::vector<std::array<float, 3>> const& /*normals*/,
                    vtk::reader_data) -> void override {}
    auto on_normals(std::string const& /*name*/,
                    std::vector<std::array<double, 3>> const& /*normals*/,
                    vtk::reader_data) -> void override {}
    auto on_texture_coordinates(
        std::string const& /*name*/,
        std::vector<std::array<float, 2>> const& /*texture_coordinates*/,
        vtk::reader_data) -> void override {}
    auto on_texture_coordinates(
        std::string const& /*name*/,
        std::vector<std::array<double, 2>> const& /*texture_coordinates*/,
        vtk::reader_data) -> void override {}
    auto on_tensors(std::string const& /*name*/,
                    std::vector<std::array<float, 9>> const& /*tensors*/,
                    vtk::reader_data) -> void override {}
    auto on_tensors(std::string const& /*name*/,
                    std::vector<std::array<double, 9>> const& /*tensors*/,
                    vtk::reader_data) -> void override {}

    template <typename T>
    auto insert_prop(std::string const& prop_name, std::vector<T> const& data,
                     size_t const num_comps) {
      size_t i = 0;
      if (num_comps == 1) {
        auto& prop = gr.insert_vertex_property<T>(prop_name);
        gr.iterate_over_vertex_indices(
            [&](auto const... is) { prop(is...) = data[i++]; });
      }
      if (num_comps == 2) {
        auto& prop = gr.insert_vertex_property<vec<T, 2>>(prop_name);
        gr.iterate_over_vertex_indices([&](auto const... is) {
          prop(is...) = {data[i], data[i + 1]};
          i += num_comps;
        });
      }
      if (num_comps == 3) {
        auto& prop = gr.insert_vertex_property<vec<T, 3>>(prop_name);
        gr.iterate_over_vertex_indices([&](auto const... is) {
          prop(is...) = {data[i], data[i + 1], data[i + 2]};
          i += num_comps;
        });
      }
      if (num_comps == 4) {
        auto& prop = gr.insert_vertex_property<vec<T, 4>>(prop_name);
        gr.iterate_over_vertex_indices([&](auto const... is) {
          prop(is...) = {data[i], data[i + 1], data[i + 2], data[i + 3]};
          i += num_comps;
        });
      }
    }
    auto on_scalars(std::string const& data_name,
                    std::string const& /*lookup_table_name*/,
                    size_t const num_comps, std::vector<float> const& data,
                    vtk::reader_data) -> void override {
      insert_prop<float>(data_name, data, num_comps);
    }
    auto on_scalars(std::string const& data_name,
                    std::string const& /*lookup_table_name*/,
                    size_t const num_comps, std::vector<double> const& data,
                    vtk::reader_data) -> void override {
      insert_prop<double>(data_name, data, num_comps);
    }
    auto on_point_data(size_t) -> void override {}
    auto on_cell_data(size_t) -> void override {}
    auto on_field_array(std::string const /*field_name*/,
                        std::string const       field_array_name,
                        std::vector<int> const& data, size_t num_comps,
                        size_t /*num_tuples*/) -> void override {
      insert_prop<int>(field_array_name, data, num_comps);
    }
    auto on_field_array(std::string const /*field_name*/,
                        std::string const         field_array_name,
                        std::vector<float> const& data, size_t num_comps,
                        size_t /*num_tuples*/) -> void override {
      insert_prop<float>(field_array_name, data, num_comps);
    }
    auto on_field_array(std::string const /*field_name*/,
                        std::string const          field_array_name,
                        std::vector<double> const& data, size_t num_comps,
                        size_t /*num_tuples*/) -> void override {
      insert_prop<double>(field_array_name, data, num_comps);
    }
  };
#ifdef __cpp_concepts
  template <typename = void>
      requires(num_dimensions() == 2) ||
      (num_dimensions() == 3)
#else
  template <size_t _N                         = num_dimensions(),
            enable_if<(_N == 2) || (_N == 3)> = true>
#endif
          auto read_vtk(filesystem::path const& path) {
    bool             is_structured_points = false;
    vec3             spacing;
    vtk_listener     listener{*this, is_structured_points, spacing};
    vtk::legacy_file f{path};
    f.add_listener(listener);
    f.read();

    if (is_structured_points) {
      if constexpr (std::is_same_v<std::decay_t<decltype(dimension<0>())>,
                                   linspace<double>>) {
        dimension<0>().back() = front<0>() + (size<0>() - 1) * spacing(0);
      } else {
        size_t i = 0;
        for (auto& d : dimension<0>()) {
          d = front<0>() + (i++) * spacing(0);
        }
      }
      if constexpr (std::is_same_v<std::decay_t<decltype(dimension<1>())>,
                                   linspace<double>>) {
        dimension<1>().back() = front<1>() + (size<1>() - 1) * spacing(1);
      } else {
        size_t i = 0;
        for (auto& d : dimension<1>()) {
          d = front<1>() + (i++) * spacing(1);
        }
      }
      if constexpr (num_dimensions() == 3) {
        if constexpr (std::is_same_v<std::decay_t<decltype(dimension<2>())>,
                                     linspace<double>>) {
          dimension<2>().back() = front<2>() + (size<2>() - 1) * spacing(2);
        } else {
          size_t i = 0;
          for (auto& d : dimension<2>()) {
            d = front<2>() + (i++) * spacing(2);
          }
        }
      }
    }
  }
  //----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <typename = void>
      requires(num_dimensions() == 2) ||
      (num_dimensions() == 3)
#else
  template <size_t _N                         = num_dimensions(),
            enable_if<(_N == 2) || (_N == 3)> = true>
#endif
          auto read_amira(filesystem::path const& path) {
    auto const  am        = amira::read<real_t>(path);
    auto const& data      = std::get<0>(am);
    auto const& dims      = std::get<1>(am);
    auto const& aabb      = std::get<2>(am);
    auto const& num_comps = std::get<3>(am);
    if (dims[2] == 1 && num_dimensions() == 3) {
      throw std::runtime_error{
          "[grid::read_amira] file contains 2-dimensional data. Cannot "
          "read "
          "into 3-dimensional grid"};
    }
    if (dims[2] > 1 && num_dimensions() == 2) {
      throw std::runtime_error{
          "[grid::read_amira] file contains 3-dimensional data. Cannot "
          "read "
          "into 2-dimensional grid"};
    }
    // set dimensions

    if constexpr (std::is_same_v<std::decay_t<decltype(dimension<0>())>,
                                 linspace<double>>) {
      dimension<0>() = linspace<double>{aabb.min(0), aabb.max(0), dims[0]};
    } else if constexpr (std::is_same_v<std::decay_t<decltype(dimension<0>())>,
                                        linspace<real_t>>) {
      dimension<0>() = linspace<real_t>{aabb.min(0), aabb.max(0), dims[0]};
    } else {
      linspace<double> d{aabb.min(0), aabb.max(0), dims[0]};
      dimension<0>().resize(dims[0]);
      std::copy(begin(d), end(d), begin(dimension<0>()));
    }
    if constexpr (std::is_same_v<std::decay_t<decltype(dimension<1>())>,
                                 linspace<double>>) {
      dimension<1>() = linspace<double>{aabb.min(1), aabb.max(1), dims[1]};
    } else if constexpr (std::is_same_v<std::decay_t<decltype(dimension<0>())>,
                                        linspace<real_t>>) {
      dimension<1>() = linspace<real_t>{aabb.min(1), aabb.max(1), dims[1]};
    } else {
      linspace<double> d{aabb.min(1), aabb.max(1), dims[1]};
      dimension<1>().resize(dims[1]);
      std::copy(begin(d), end(d), begin(dimension<1>()));
    }
    if constexpr (num_dimensions() == 3) {
      if constexpr (std::is_same_v<std::decay_t<decltype(dimension<1>())>,
                                   linspace<double>>) {
        dimension<2>() = linspace<double>{aabb.min(2), aabb.max(2), dims[2]};
      } else if constexpr (std::is_same_v<
                               std::decay_t<decltype(dimension<1>())>,
                               linspace<real_t>>) {
        dimension<2>() = linspace<real_t>{aabb.min(2), aabb.max(2), dims[2]};
      } else {
        linspace<double> d{aabb.min(2), aabb.max(2), dims[2]};
        dimension<2>().resize(dims[2]);
        std::copy(begin(d), end(d), begin(dimension<2>()));
      }
    }
    // copy data
    size_t i = 0;
    if (num_comps == 1) {
      auto& prop = insert_vertex_property<real_t>(path.string());
      iterate_over_vertex_indices(
          [&](auto const... is) { prop(is...) = data[i++]; });
    } else if (num_comps == 2) {
      auto& prop = insert_vertex_property<vec<real_t, 2>>(path.string());
      iterate_over_vertex_indices([&](auto const... is) {
        prop(is...) = {data[i], data[i + 1]};
        i += num_comps;
      });
    } else if (num_comps == 3) {
      auto& prop = insert_vertex_property<vec<real_t, 3>>(path.string());
      iterate_over_vertex_indices([&](auto const... is) {
        prop(is...) = {data[i], data[i + 1], data[i + 2]};
        i += num_comps;
      });
    }
  }
  //----------------------------------------------------------------------------
#ifdef TATOOINE_NETCDF_AVAILABLE
  auto read_netcdf(filesystem::path const& path) {
    read_netcdf(path, std::make_index_sequence<num_dimensions()>{});
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename T, size_t... Seq>
  auto insert_variables_of_type(netcdf::file& f, bool& first,
                                std::index_sequence<Seq...> /*seq*/) {
    for (auto v : f.variables<T>()) {
      if (v.name() == "x" || v.name() == "y" || v.name() == "z" ||
          v.name() == "t" || v.name() == "X" || v.name() == "Y" ||
          v.name() == "Z" || v.name() == "T" || v.name() == "xdim" ||
          v.name() == "ydim" || v.name() == "zdim" || v.name() == "tdim" ||
          v.name() == "Xdim" || v.name() == "Ydim" || v.name() == "Zdim" ||
          v.name() == "Tdim" || v.name() == "XDim" || v.name() == "YDim" ||
          v.name() == "ZDim" || v.name() == "TDim") {
        continue;
      }
      if (v.num_dimensions() != num_dimensions() &&
          v.size()[0] != vertices().size()) {
        throw std::runtime_error{
            "[grid::read_netcdf] variable's number of dimensions does "
            "not "
            "match grid's number of dimensions:\nnumber of grid "
            "dimensions: " +
            std::to_string(num_dimensions()) + "\nnumber of data dimensions: " +
            std::to_string(v.num_dimensions()) +
            "\nvariable name: " + v.name()};
      }
      if (!first) {
        auto check = [this, &v](size_t i) {
          if (v.size(i) != size(i)) {
            throw std::runtime_error{"[grid::read_netcdf] variable's size(" +
                                     std::to_string(i) +
                                     ") does not "
                                     "match grid's size(" +
                                     std::to_string(i) + ")"};
          }
        };
        (check(Seq), ...);
      } else {
        ((f.variable<
               typename std::decay_t<decltype(dimension<Seq>())>::value_type>(
               v.dimension_name(Seq))
              .read(dimension<Seq>())),
         ...);
        first = false;
      }
      create_vertex_property<lazy_reader<netcdf::variable<T>>>(
          v.name(), v,
          std::vector<size_t>(num_dimensions(),
                              m_chunk_size_for_lazy_properties));
    }
  }
  /// this only reads scalar types
  template <size_t... Seq>
  auto read_netcdf(filesystem::path const&     path,
                   std::index_sequence<Seq...> seq) {
    netcdf::file f{path, netCDF::NcFile::read};
    bool         first = true;
    insert_variables_of_type<double>(f, first, seq);
    insert_variables_of_type<float>(f, first, seq);
    insert_variables_of_type<int>(f, first, seq);
  }
#endif
  //----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <typename T>
  requires(num_dimensions() == 3)
#else
  template <typename T, size_t _N = num_dimensions(), enable_if<_N == 3> = true>
#endif
      void write_amira(std::string const& path,
                       std::string const& vertex_property_name) const {
    write_amira(path, vertex_property<T>(vertex_property_name));
  }
  //----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <typename T, bool HasNonConstReference>
      requires is_uniform &&
      (num_dimensions() == 3)
#else
  template <typename T, bool HasNonConstReference, bool U = is_uniform,
            size_t _N = num_dimensions(), enable_if<(U && (_N == 3))> = true>
#endif
          void write_amira(
              std::string const& path,
              typed_vertex_property_interface_t<T, HasNonConstReference> const&
                  prop) const {
    std::ofstream     outfile{path, std::ofstream::binary};
    std::stringstream header;

    header << "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1\n\n";
    header << "define Lattice " << size<0>() << " " << size<1>() << " "
           << size<2>() << "\n\n";
    header << "Parameters {\n";
    header << "    BoundingBox " << front<0>() << " " << back<0>() << " "
           << front<1>() << " " << back<1>() << " " << front<2>() << " "
           << back<2>() << ",\n";
    header << "    CoordType \"uniform\"\n";
    header << "}\n";
    if constexpr (num_components<T> > 1) {
      header << "Lattice { " << type_name<internal_data_type_t<T>>() << "["
             << num_components<T> << "] Data } @1\n\n";
    } else {
      header << "Lattice { " << type_name<internal_data_type_t<T>>()
             << " Data } @1\n\n";
    }
    header << "# Data section follows\n@1\n";
    auto const header_string = header.str();

    std::vector<T> data;
    data.reserve(size<0>() * size<1>() * size<2>());
    auto back_inserter = [&](auto const... is) { data.push_back(prop(is...)); };
    for_loop(back_inserter, size<0>(), size<1>(), size<2>());
    outfile.write((char*)header_string.c_str(),
                  header_string.size() * sizeof(char));
    outfile.write((char*)data.data(), data.size() * sizeof(T));
  }
  //----------------------------------------------------------------------------
 private:
  template <typename T, bool HasNonConstReference>
  void write_prop_vtk(
      vtk::legacy_file_writer& writer, std::string const& name,
      typed_vertex_property_interface_t<T, HasNonConstReference> const& prop)
      const {
    std::vector<T> data;
    vertices().iterate_indices(
        [&](auto const... is) { data.push_back(prop(is...)); });
    writer.write_scalars(name, data);
  }

 public:
  auto write(filesystem::path const& path) const {
    auto const ext = path.extension();

    if constexpr (num_dimensions() == 1 || num_dimensions() == 2 ||
                  num_dimensions() == 3) {
      if (ext == ".vtk") {
        write_vtk(path);
        return;
      }
    }
  }
  //----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <typename = void>
      requires(num_dimensions() == 1) || (num_dimensions() == 2) ||
      (num_dimensions() == 3)
#else
  template <size_t _N                          = num_dimensions(),
            enable_if<(num_dimensions() == 1) || (num_dimensions() == 2) ||
                      (num_dimensions() == 3)> = true>
#endif
          void write_vtk(
              filesystem::path const& path,
              std::string const&      description = "tatooine grid") const {
    auto writer = [this, &path, &description] {
      if constexpr (is_uniform) {
        vtk::legacy_file_writer writer{path,
                                       vtk::dataset_type::structured_points};
        writer.set_title(description);
        writer.write_header();
        if constexpr (num_dimensions() == 1) {
          writer.write_dimensions(size<0>(), 1, 1);
          writer.write_origin(front<0>(), 0, 0);
          writer.write_spacing(dimension<0>().spacing(), 0, 0);
        } else if constexpr (num_dimensions() == 2) {
          writer.write_dimensions(size<0>(), size<1>(), 1);
          writer.write_origin(front<0>(), front<1>(), 0);
          writer.write_spacing(dimension<0>().spacing(),
                               dimension<1>().spacing(), 0);
        } else if constexpr (num_dimensions() == 3) {
          writer.write_dimensions(size<0>(), size<1>(), size<2>());
          writer.write_origin(front<0>(), front<1>(), front<2>());
          writer.write_spacing(dimension<0>().spacing(),
                               dimension<1>().spacing(),
                               dimension<2>().spacing());
        }
        return writer;
      } else {
        vtk::legacy_file_writer writer{path,
                                       vtk::dataset_type::rectilinear_grid};
        writer.set_title(description);
        writer.write_header();
        if constexpr (num_dimensions() == 1) {
          writer.write_dimensions(size<0>(), 1, 1);
          writer.write_x_coordinates(
              std::vector<double>(begin(dimension<0>()), end(dimension<0>())));
          writer.write_y_coordinates(std::vector<double>{0});
          writer.write_z_coordinates(std::vector<double>{0});
        } else if constexpr (num_dimensions() == 2) {
          writer.write_dimensions(size<0>(), size<1>(), 1);
          writer.write_x_coordinates(
              std::vector<double>(begin(dimension<0>()), end(dimension<0>())));
          writer.write_y_coordinates(
              std::vector<double>(begin(dimension<1>()), end(dimension<1>())));
          writer.write_z_coordinates(std::vector<double>{0});
        } else if constexpr (num_dimensions() == 3) {
          writer.write_dimensions(size<0>(), size<1>(), size<2>());
          writer.write_x_coordinates(
              std::vector<double>(begin(dimension<0>()), end(dimension<0>())));
          writer.write_y_coordinates(
              std::vector<double>(begin(dimension<1>()), end(dimension<1>())));
          writer.write_z_coordinates(
              std::vector<double>(begin(dimension<2>()), end(dimension<2>())));
        }
        return writer;
      }
    }();
    // write vertex data
    writer.write_point_data(vertices().size());
    for (const auto& [name, prop] : this->m_vertex_properties) {
      if (prop->type() == typeid(int)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<const typed_vertex_property_interface_t<int, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(float)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<float, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(double)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<double, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(vec2f)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<vec2f, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(vec3f)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<vec3f, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(vec4f)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<vec4f, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(vec2d)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<vec2d, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(vec3d)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<vec3d, true>*>(
                prop.get()));
      } else if (prop->type() == typeid(vec4d)) {
        write_prop_vtk(
            writer, name,
            *dynamic_cast<
                const typed_vertex_property_interface_t<vec4d, true>*>(
                prop.get()));
      }
    }
  }

 private:
  template <size_t I>
  auto print_dim(std::ostream& out) const {
    auto const& dim = dimension<I>();
    if constexpr (is_linspace<std::decay_t<decltype(dim)>>) {
      out << dim;
    } else {
      out << dim.front() << ", " << dim[1] << ", ..., " << dim.back();
    }
    out << " [" << dim.size() << "]\n";
  }
  template <size_t... Seq>
  auto print(std::ostream& out, std::index_sequence<Seq...> /*seq*/) const
      -> auto& {
    (print_dim<Seq>(out), ...);
    return out;
  }

 public:
  auto print(std::ostream& out = std::cout) const -> auto& {
    return print(out, std::make_index_sequence<num_dimensions()>{});
  }
  //----------------------------------------------------------------------------
  auto chunk_size_for_lazy_properties() {
    return m_chunk_size_for_lazy_properties;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto set_chunk_size_for_lazy_properties(size_t const val) -> void {
    m_chunk_size_for_lazy_properties = val;
  }
};
//==============================================================================
// free functions
//==============================================================================
template <typename... Dimensions>
auto operator<<(std::ostream& out, grid<Dimensions...> const& g) -> auto& {
  return g.print(out);
}
#ifdef __cpp_concepts
template <indexable_space... Dimensions>
#else
template <typename... Dimensions>
#endif
auto vertices(grid<Dimensions...> const& g) {
  return g.vertices();
}
//==============================================================================
// deduction guides
//==============================================================================
template <typename... Dimensions>
grid(Dimensions&&...) -> grid<std::decay_t<Dimensions>...>;
// additional, for g++
template <typename Dim0, typename... Dims>
grid(Dim0&&, Dims&&...) -> grid<std::decay_t<Dim0>, std::decay_t<Dims>...>;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t N, size_t... Seq>
grid(axis_aligned_bounding_box<Real, N> const& bb,
     std::array<size_t, N> const&              res, std::index_sequence<Seq...>)
    -> grid<decltype(((void)Seq, std::declval<linspace<Real>()>))...>;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
template <integral... Size>
#else
template <typename... Size, enable_if<is_integral<Size...>> = true>
#endif
grid(Size const...)
    -> grid<linspace<std::conditional_t<true, double, Size>>...>;
//==============================================================================
// operators
//==============================================================================
#ifdef __cpp_concepts
template <indexable_space... Dimensions, indexable_space AdditionalDimension>
#else
template <typename... Dimensions, typename AdditionalDimension>
#endif
auto operator+(grid<Dimensions...> const& grid,
               AdditionalDimension&&      additional_dimension) {
  return grid.add_dimension(
      std::forward<AdditionalDimension>(additional_dimension));
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
template <indexable_space... Dimensions, indexable_space AdditionalDimension>
#else
template <typename... Dimensions, typename AdditionalDimension>
#endif
auto operator+(AdditionalDimension&&      additional_dimension,
               grid<Dimensions...> const& grid) {
  return grid.add_dimension(
      std::forward<AdditionalDimension>(additional_dimension));
}
//==============================================================================
// typedefs
//==============================================================================
#ifdef __cpp_concepts
template <indexable_space IndexableSpace, size_t N>
#else
template <typename IndexableSpace, size_t N>
#endif
struct grid_creator {
 private:
  template <typename... Args, size_t... Seq>
  static constexpr auto create(std::index_sequence<Seq...> /*seq*/,
                               Args&&... args) {
    return grid<decltype((static_cast<void>(Seq), IndexableSpace{}))...>{
        std::forward<Args>(args)...};
  }
  template <typename... Args>
  static constexpr auto create(Args&&... args) {
    return create(std::make_index_sequence<N>{}, std::forward<Args>(args)...);
  }

 public:
  using type = decltype(create());
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
template <indexable_space IndexableSpace, size_t N>
#else
template <typename IndexableSpace, size_t N>
#endif
using grid_creator_t = typename grid_creator<IndexableSpace, N>::type;
//==============================================================================
#ifdef __cpp_concepts
template <arithmetic Real, size_t N>
#else
template <typename Real, size_t N>
#endif
using uniform_grid = grid_creator_t<linspace<Real>, N>;
//------------------------------------------------------------------------------
#ifdef __cpp_concepts
template <arithmetic Real, size_t N>
#else
template <typename Real, size_t N>
#endif
using non_uniform_grid = grid_creator_t<std::vector<Real>, N>;
//------------------------------------------------------------------------------
#ifdef __cpp_concepts
template <arithmetic Real, size_t... N>
#else
template <typename Real, size_t... N>
#endif
using static_non_uniform_grid = grid<std::array<Real, N>...>;
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
