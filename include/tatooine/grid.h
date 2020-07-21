#ifndef TATOOINE_GRID_H
#define TATOOINE_GRID_H
//==============================================================================
#include <tatooine/boundingbox.h>
#include <tatooine/chunked_multidim_array.h>
#include <tatooine/concepts.h>
#include <tatooine/for_loop.h>
#include <tatooine/grid_vertex_container.h>
#include <tatooine/grid_vertex_iterator.h>
#include <tatooine/grid_vertex_property.h>
#include <tatooine/interpolation.h>
#include <tatooine/linspace.h>
#include <tatooine/random.h>
#include <tatooine/vec.h>

#include <map>
#include <memory>
#include <tuple>
//==============================================================================
namespace tatooine {
//==============================================================================
template <indexable_space... Dimensions>
class grid {
  static_assert(sizeof...(Dimensions) > 0, "Grid needs at least one dimension.");

 public:
  static constexpr bool is_regular =
      (is_linspace_v<std::decay_t<Dimensions>> && ...);
  static constexpr auto num_dimensions() { return sizeof...(Dimensions); }
  using this_t = grid<Dimensions...>;
  using real_t = promote_t<typename Dimensions::value_type...>;
  using vec_t  = vec<real_t, num_dimensions()>;
  using pos_t  = vec_t;
  using seq_t  = std::make_index_sequence<num_dimensions()>;

  using dimensions_t = std::tuple<std::decay_t<Dimensions>...>;

  //using vertex_iterator  = grid_vertex_iterator<Dimensions...>;
  //using vertex_container = grid_vertex_container<Dimensions...>;
  //
  //template <typename T>
  //using default_interpolation_kernel_t = interpolation::hermite<T>;
  //
  //// general property types
  //using property_t = multidim_property<this_t>;
  //template <typename T>
  //using typed_property_t     = typed_multidim_property<this_t, T>;
  //using property_ptr_t       = std::unique_ptr<property_t>;
  //using property_container_t = std::map<std::string, property_ptr_t>;
  //
  ////----------------------------------------------------------------------------
  //// vertex properties
  //template <typename Container, typename... InterpolationKernels>
  //using vertex_property_t =
  //    grid_vertex_property<this_t, Container, InterpolationKernels...>;
  //// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //template <typename T, typename Indexing, typename... InterpolationKernels>
  //using chunked_vertex_property_t =
  //    vertex_property_t<chunked_multidim_array<T, Indexing>,
  //                      InterpolationKernels...>;
  //// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //template <typename T, typename Indexing, typename... InterpolationKernels>
  //using contiguous_vertex_property_t =
  //    vertex_property_t<dynamic_multidim_array<T, Indexing>,
  //                      InterpolationKernels...>;

  //============================================================================
 private:
  dimensions_t         m_dimensions;
 // property_container_t m_vertex_properties;
 // std::array<std::vector<std::vector<double>>, num_dimensions()>
 //     m_diff_stencil_coefficients_n1_0_p1, m_diff_stencil_coefficients_n2_n1_0,
 //     m_diff_stencil_coefficients_0_p1_p2, m_diff_stencil_coefficients_0_p1,
 //     m_diff_stencil_coefficients_n1_0;
 // //============================================================================
 public:
  //constexpr grid() = default;
  //constexpr grid(grid const& other)
  //    : m_dimensions{other.m_dimensions},
  //      m_diff_stencil_coefficients_n1_0_p1{
  //          other.m_diff_stencil_coefficients_n1_0_p1},
  //      m_diff_stencil_coefficients_n2_n1_0{
  //          other.m_diff_stencil_coefficients_n2_n1_0},
  //      m_diff_stencil_coefficients_0_p1_p2{
  //          other.m_diff_stencil_coefficients_0_p1_p2},
  //      m_diff_stencil_coefficients_0_p1{
  //          other.m_diff_stencil_coefficients_0_p1},
  //      m_diff_stencil_coefficients_n1_0{
  //          other.m_diff_stencil_coefficients_n1_0} {
  //  for (auto const& [name, prop] : other.m_vertex_properties) {
  //    m_vertex_properties.emplace(name, prop->clone());
  //  }
  //}
  //constexpr grid(grid&& other) noexcept = default;
  //----------------------------------------------------------------------------
  template <typename... _Dimensions>
  constexpr grid(_Dimensions&&... dimensions)
      : m_dimensions{std::forward<_Dimensions>(dimensions)...} {
    static_assert(sizeof...(Dimensions) == num_dimensions(),
                  "Number of given dimensions does not match number of "
                  "specified dimensions.");
    static_assert(
        (std::is_same_v<std::decay_t<_Dimensions>, Dimensions> && ...),
        "Constructor dimension types differ class dimension types.");
  }
 // //----------------------------------------------------------------------------
 //private:
 // template <typename Real, size_t... Is>
 // constexpr grid(boundingbox<Real, num_dimensions()> const&  bb,
 //                std::array<size_t, num_dimensions()> const& res,
 //                std::index_sequence<Is...> [>is<])
 //     : m_dimensions{linspace<real_t>{real_t(bb.min(Is)), real_t(bb.max(Is)),
 //                                     res[Is]}...} {}
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // template <typename Real>
 // constexpr grid(boundingbox<Real, num_dimensions()> const&  bb,
 //                std::array<size_t, num_dimensions()> const& res)
 //     : grid{bb, res, seq_t{}} {}
 // //----------------------------------------------------------------------------
  //~grid() = default;
  //============================================================================
 //private:
 // template <size_t... Ds>
 // constexpr auto copy_without_properties(
 //     std::index_sequence<Ds...> [>seq<]) const {
 //   return this_t{std::get<Ds>(m_dimensions)...};
 // }
 //
 //public:
  //constexpr auto copy_without_properties() const {
  //  return copy_without_properties(
  //      std::make_index_sequence<num_dimensions()>{});
  //}
  //============================================================================
  //constexpr auto operator=(grid const& other) -> grid& = default;
  //constexpr auto operator=(grid&& other) noexcept -> grid& = default;
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
  ////----------------------------------------------------------------------------
  //constexpr auto dimensions() -> auto& { return m_dimensions; }
  //// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //constexpr auto dimensions() const -> auto const& { return m_dimensions; }
  ////----------------------------------------------------------------------------
  //constexpr auto front_dimension() -> auto& {
  //  return std::get<0>(m_dimensions);
  //}
  //// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //constexpr auto front_dimension() const -> auto const& {
  //  return std::get<0>(m_dimensions);
  //}
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto min(std::index_sequence<Is...> [>is<]) const {
 //   return vec<real_t, num_dimensions()>{static_cast<real_t>(front<Is>())...};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // constexpr auto min() const { return min(seq_t{}); }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto max(std::index_sequence<Is...> [>is<]) const {
 //   return vec<real_t, num_dimensions()>{static_cast<real_t>(back<Is>())...};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // constexpr auto max() const { return max(seq_t{}); }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto resolution(std::index_sequence<Is...> [>is<]) const {
 //   return vec<size_t, num_dimensions()>{size<Is>()...};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // constexpr auto resolution() const { return resolution(seq_t{}); }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto boundingbox(std::index_sequence<Is...> [>is<]) const {
 //   static_assert(sizeof...(Is) == num_dimensions());
 //   return tatooine::boundingbox<real_t, num_dimensions()>{
 //       vec<real_t, num_dimensions()>{static_cast<real_t>(front<Is>())...},
 //       vec<real_t, num_dimensions()>{static_cast<real_t>(back<Is>())...}};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // constexpr auto boundingbox() const { return boundingbox(seq_t{}); }
 // //----------------------------------------------------------------------------
 //protected:
 // template <size_t... Is>
 // constexpr auto size(std::index_sequence<Is...> [>is<]) const {
 //   static_assert(sizeof...(Is) == num_dimensions());
 //   return vec<size_t, num_dimensions()>{size<Is>()...};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // constexpr auto size() const { return size(seq_t{}); }
 // //----------------------------------------------------------------------------
 // template <size_t I>
 // constexpr auto size() const {
 //   return dimension<I>().size();
 // }
 // //----------------------------------------------------------------------------
 // template <size_t I>
 // constexpr auto front() const {
 //   return dimension<I>().front();
 // }
 // //----------------------------------------------------------------------------
 // template <size_t I>
 // constexpr auto back() const {
 //   return dimension<I>().back();
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto in_domain(std::index_sequence<Is...> [>is<],
 //                          real_number auto const... xs) const {
 //   static_assert(sizeof...(xs) == num_dimensions(),
 //                 "number of components does not match number of dimensions");
 //   static_assert(sizeof...(Is) == num_dimensions(),
 //                 "number of indices does not match number of dimensions");
 //   return ((front<Is>() <= xs) && ...) && ((xs <= back<Is>()) && ...);
 // }
 // //----------------------------------------------------------------------------
 //public:
 // constexpr auto in_domain(real_number auto const... xs) const {
 //   static_assert(sizeof...(xs) == num_dimensions(),
 //                 "number of components does not match number of dimensions");
 //   return in_domain(seq_t{}, xs...);
 // }
 //
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto in_domain(std::array<real_t, num_dimensions()> const& x,
 //                          std::index_sequence<Is...> [>is<]) const {
 //   return in_domain(x[Is]...);
 // }
 // //----------------------------------------------------------------------------
 //public:
 // constexpr auto in_domain(
 //     std::array<real_t, num_dimensions()> const& x) const {
 //   return in_domain(x, seq_t{});
 // }
 // //----------------------------------------------------------------------------
 // /// returns cell index and factor for interpolation
 // template <size_t I>
 // auto cell_index(real_number auto const x) const -> std::pair<size_t, double> {
 //   auto const& dim = dimension<I>();
 //   if constexpr (is_linspace_v<decltype(dimension<I>())>) {
 //     // calculate
 //     auto const pos =
 //         (x - dim.front()) / (dim.back() - dim.front()) * (size() - 1);
 //     auto const quantized_pos = static_cast<size_t>(std::floor(pos));
 //     return {quantized_pos, pos - quantized_pos};
 //   } else {
 //     // binary search
 //     size_t left  = 0;
 //     size_t right = dim.size() - 1;
 //     while (right - left > 1) {
 //       auto const center = (right + left) / 2;
 //       if (x < dim[center]) {
 //         right = center;
 //       } else {
 //         left = center;
 //       }
 //     }
 //     return {left, (x - dim[left]) / (dim[left + 1] - dim[left])};
 //   }
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // /// returns cell indices and factors for each dimension for interpolaton
 // template <size_t... DimensionIndex>
 // auto cell_index(std::index_sequence<DimensionIndex...>,
 //                 real_number auto const... xs) const
 //     -> std::array<std::pair<size_t, double>, num_dimensions()> {
 //   return std::array{cell_index<DimensionIndex>(static_cast<double>(xs))...};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // auto cell_index(real_number auto const... xs) const {
 //   return cell_index(seq_t{}, xs...);
 // }
 // //----------------------------------------------------------------------------
 // auto diff_stencil_coefficients_n1_0_p1(size_t dim_index, size_t i) const
 //     -> auto const& {
 //   return m_diff_stencil_coefficients_n1_0_p1[dim_index][i];
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // auto diff_stencil_coefficients_n2_n1_0(size_t dim_index, size_t i) const
 //     -> auto const& {
 //   return m_diff_stencil_coefficients_n1_0[dim_index][i];
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // auto diff_stencil_coefficients_n1_0(size_t dim_index, size_t i) const
 //     -> auto const& {
 //   return m_diff_stencil_coefficients_n1_0[dim_index][i];
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // auto diff_stencil_coefficients_0_p1(size_t dim_index, size_t i) const
 //     -> auto const& {
 //   return m_diff_stencil_coefficients_0_p1[dim_index][i];
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // auto diff_stencil_coefficients_0_p1_p2(size_t dim_index, size_t i) const
 //     -> auto const& {
 //   return m_diff_stencil_coefficients_0_p1_p2[dim_index][i];
 // }
 // //----------------------------------------------------------------------------
 // template <size_t... Ds>
 // auto update_diff_stencil_coefficients(std::index_sequence<Ds...> [>seq<]) {
 //   (update_diff_stencil_coefficients_n1_0_p1<Ds>(), ...);
 //   (update_diff_stencil_coefficients_0_p1_p2<Ds>(), ...);
 //   (update_diff_stencil_coefficients_n2_n1_0<Ds>(), ...);
 //   (update_diff_stencil_coefficients_0_p1<Ds>(), ...);
 //   (update_diff_stencil_coefficients_n1_0<Ds>(), ...);
 // }
 //
 // auto update_diff_stencil_coefficients() {
 //   update_diff_stencil_coefficients(
 //       std::make_index_sequence<num_dimensions()>{});
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // template <size_t D>
 // auto update_diff_stencil_coefficients_n1_0_p1() {
 //   auto const& dim = dimension<D>();
 //   m_diff_stencil_coefficients_n1_0_p1[D].resize(dim.size());
 //
 //   for (size_t i = 1; i < dim.size() - 1; ++i) {
 //     vec<double, 3> xs;
 //     for (size_t j = 0; j < 3; ++j) { xs(j) = dim[i - 1 + j] - dim[i]; }
 //     auto const cs = finite_differences_coefficients(1, xs);
 //     m_diff_stencil_coefficients_n1_0_p1[D][i].reserve(3);
 //     std::copy(begin(cs.data()), end(cs.data()),
 //               std::back_inserter(m_diff_stencil_coefficients_n1_0_p1[D][i]));
 //   }
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // template <size_t D>
 // auto update_diff_stencil_coefficients_0_p1_p2() {
 //   auto const& dim = dimension<D>();
 //   m_diff_stencil_coefficients_0_p1_p2[D].resize(dim.size());
 //
 //   for (size_t i = 0; i < dim.size() - 2; ++i) {
 //     vec<double, 3> xs;
 //     for (size_t j = 0; j < 3; ++j) { xs(j) = dim[i + j] - dim[i]; }
 //     auto const cs = finite_differences_coefficients(1, xs);
 //     m_diff_stencil_coefficients_0_p1_p2[D][i].reserve(3);
 //     std::copy(begin(cs.data()), end(cs.data()),
 //               std::back_inserter(m_diff_stencil_coefficients_0_p1_p2[D][i]));
 //   }
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // template <size_t D>
 // auto update_diff_stencil_coefficients_n2_n1_0() {
 //   auto const& dim = dimension<D>();
 //   m_diff_stencil_coefficients_n2_n1_0[D].resize(dim.size());
 //
 //   for (size_t i = 2; i < dim.size(); ++i) {
 //     vec<double, 3> xs;
 //     for (size_t j = 0; j < 3; ++j) { xs(j) = dim[i - 2 + j] - dim[i]; }
 //     auto const cs = finite_differences_coefficients(1, xs);
 //     m_diff_stencil_coefficients_n2_n1_0[D][i].reserve(3);
 //     std::copy(begin(cs.data()), end(cs.data()),
 //               std::back_inserter(m_diff_stencil_coefficients_n2_n1_0[D][i]));
 //   }
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // template <size_t D>
 // auto update_diff_stencil_coefficients_0_p1() {
 //   auto const& dim = dimension<D>();
 //   m_diff_stencil_coefficients_0_p1[D].resize(dim.size());
 //
 //   for (size_t i = 0; i < dim.size() - 1; ++i) {
 //     vec<double, 2> xs;
 //     for (size_t j = 0; j < 2; ++j) { xs(j) = dim[i + j] - dim[i]; }
 //     auto const cs = finite_differences_coefficients(1, xs);
 //     m_diff_stencil_coefficients_0_p1[D][i].reserve(2);
 //     std::copy(begin(cs.data()), end(cs.data()),
 //               std::back_inserter(m_diff_stencil_coefficients_0_p1[D][i]));
 //   }
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 // template <size_t D>
 // auto update_diff_stencil_coefficients_n1_0() {
 //   auto const& dim = dimension<D>();
 //   m_diff_stencil_coefficients_n1_0[D].resize(dim.size());
 //
 //   for (size_t i = 1; i < dim.size(); ++i) {
 //     vec<double, 2> xs;
 //     for (size_t j = 0; j < 2; ++j) { xs(j) = dim[i - 1 + j] - dim[i]; }
 //     auto const cs = finite_differences_coefficients(1, xs);
 //     m_diff_stencil_coefficients_n1_0[D][i].reserve(3);
 //     std::copy(begin(cs.data()), end(cs.data()),
 //               std::back_inserter(m_diff_stencil_coefficients_n1_0[D][i]));
 //   }
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... DIs>
 // auto vertex_at(std::index_sequence<DIs...>, integral auto const... is) const
 //     -> vec<real_t, num_dimensions()> {
 //   static_assert(sizeof...(DIs) == sizeof...(is));
 //   static_assert(sizeof...(is) == num_dimensions());
 //   return pos_t{static_cast<real_t>((std::get<DIs>(m_dimensions)[is]))...};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // auto vertex_at(integral auto const... is) const {
 //   static_assert(sizeof...(is) == num_dimensions());
 //   return vertex_at(seq_t{}, is...);
 // }
 // //----------------------------------------------------------------------------
 // auto operator()(integral auto const... is) const {
 //   static_assert(sizeof...(is) == num_dimensions());
 //   return vertex_at(is...);
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t FaceDimensionIndex, size_t... DimensionIndex>
 // auto face_center_at(std::index_sequence<DimensionIndex...>,
 //                     integral auto const... is) const
 //     -> vec<real_t, num_dimensions()> {
 //   static_assert(sizeof...(is) == num_dimensions());
 //   pos_t pos{
 //       (FaceDimensionIndex == DimensionIndex
 //            ? static_cast<real_t>(dimension<DimensionIndex>()[is])
 //            : (static_cast<real_t>(dimension<DimensionIndex>()[is]) +
 //               static_cast<real_t>(dimension<DimensionIndex>()[is + 1])) /
 //                  2)...};
 //   return pos;
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // template <size_t FaceDimensionIndex>
 // auto face_center_at(integral auto const... is) const {
 //   static_assert(sizeof...(is) == num_dimensions());
 //   return face_center_at<FaceDimensionIndex>(seq_t{}, is...);
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto num_vertices(std::index_sequence<Is...> [>seq<]) const {
 //   return (size<Is>() * ...);
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // constexpr auto num_vertices() const { return num_vertices(seq_t{}); }
 // //----------------------------------------------------------------------------
 // /// \return number of dimensions for one dimension dim
 // // constexpr auto edges() const { return grid_edge_container{this}; }
 //
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto vertex_begin(std::index_sequence<Is...> [>is<]) const {
 //   return vertex_iterator{this, std::array{((void)Is, size_t(0))...}};
 // }
 // //----------------------------------------------------------------------------
 //public:
 // constexpr auto vertex_begin() const { return vertex_begin(seq_t{}); }
 // //----------------------------------------------------------------------------
 //private:
 // template <size_t... Is>
 // constexpr auto vertex_end(std::index_sequence<Is...> [>is<]) const {
 //   return vertex_iterator{this, std::array{((void)Is, size_t(0))...,
 //                                           size<num_dimensions() - 1>()}};
 // }
 // //----------------------------------------------------------------------------
 //public:
 // constexpr auto vertex_end() const {
 //   return vertex_end(std::make_index_sequence<num_dimensions() - 1>());
 // }
 // //----------------------------------------------------------------------------
 // auto vertices() const { return vertex_container{*this}; }
 // //----------------------------------------------------------------------------
 //private:
 // template <regular_invocable<decltype(((void)std::declval<Dimensions>(),
 //                                       size_t{}))...>
 //               Iteration,
 //           size_t... Ds>
 // auto loop_over_vertex_indices(Iteration&& iteration,
 //                               std::index_sequence<Ds...>) const
 //     -> decltype(auto) {
 //   return for_loop(std::forward<Iteration>(iteration), size<Ds>()...);
 // }
 // //----------------------------------------------------------------------------
 //public:
 // template <regular_invocable<decltype(((void)std::declval<Dimensions>(),
 //                                       size_t{}))...>
 //               Iteration,
 //           size_t... Ds>
 // auto loop_over_vertex_indices(Iteration&& iteration) const -> decltype(auto) {
 //   return loop_over_vertex_indices(
 //       std::forward<Iteration>(iteration),
 //       std::make_index_sequence<num_dimensions()>{});
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <indexable_space AdditionalDimension, size_t... Is>
 // auto add_dimension(AdditionalDimension&& additional_dimension,
 //                    std::index_sequence<Is...> [>is<]) const {
 //   return grid<Dimensions..., std::decay_t<AdditionalDimension>>{
 //       dimension<Is>()...,
 //       std::forward<AdditionalDimension>(additional_dimension)};
 // }
 // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 //public:
 // template <indexable_space AdditionalDimension>
 // auto add_dimension(indexable_space auto&& additional_dimension) const {
 //   return add_dimension(
 //       std::forward<AdditionalDimension>(additional_dimension), seq_t{});
 // }
  //----------------------------------------------------------------------------
 //private:
 // template <typename Container,
 //           template <typename> typename... InterpolationKernels,
 //           size_t... DimensionIndex, typename... Args>
 // auto add_vertex_property(std::string const& name,
 //                          std::index_sequence<DimensionIndex...>,
 //                          Args&&... args)
 //     -> typed_property_t<typename Container::value_type>& {
 //   update_diff_stencil_coefficients();
 //   if (auto it = m_vertex_properties.find(name);
 //       it == end(m_vertex_properties)) {
 //     auto [newit, new_prop] = [&]() {
 //       if constexpr (sizeof...(InterpolationKernels) == 0) {
 //         using prop_t = vertex_property_t<
 //             Container, decltype(((void)DimensionIndex,
 //                                  default_interpolation_kernel_t<
 //                                      typename Container::value_type>{}))...>;
 //         auto new_prop = new prop_t{*this, std::forward<Args>(args)...};
 //         new_prop->container().resize(size());
 //         return m_vertex_properties.emplace(
 //             name, std::unique_ptr<property_t>{new_prop});
 //       } else {
 //         using prop_t = vertex_property_t<
 //             Container,
 //             InterpolationKernels<typename Container::value_type>...>;
 //         auto new_prop = new prop_t{*this, std::forward<Args>(args)...};
 //         new_prop->container().resize(size());
 //         return m_vertex_properties.emplace(
 //             name, std::unique_ptr<property_t>{new_prop});
 //       }
 //     }();
 //
 //     return *dynamic_cast<typed_property_t<typename Container::value_type>*>(
 //         newit->second.get());
 //   } else {
 //     return *dynamic_cast<typed_property_t<typename Container::value_type>*>(
 //         it->second.get());
 //   }
 // }
 // //----------------------------------------------------------------------------
 //public:
 // template <typename Container,
 //           template <typename> typename... InterpolationKernels,
 //           typename... Args>
 // auto add_vertex_property(std::string const& name, Args&&... args) -> auto& {
 //   static_assert(
 //       sizeof...(InterpolationKernels) == num_dimensions() ||
 //           sizeof...(InterpolationKernels) == 0,
 //       "Number of interpolation kernels does not match number of dimensions.");
 //   return add_vertex_property<Container, InterpolationKernels...>(
 //       name, seq_t{}, std::forward<Args>(args)...);
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <typename T, typename Indexing = x_fastest,
 //           template <typename> typename... InterpolationKernels,
 //           size_t... DimensionIndex>
 // auto add_contiguous_vertex_property(std::string const& name,
 //                                     std::index_sequence<DimensionIndex...>)
 //     -> typed_property_t<T>& {
 //   update_diff_stencil_coefficients();
 //   if (auto it = m_vertex_properties.find(name);
 //       it == end(m_vertex_properties)) {
 //     auto [newit, new_prop] = [&]() {
 //       if constexpr (sizeof...(InterpolationKernels) == 0) {
 //         using prop_t = contiguous_vertex_property_t<
 //             T, Indexing,
 //             decltype(((void)DimensionIndex,
 //                       default_interpolation_kernel_t<T>{}))...>;
 //         return m_vertex_properties.emplace(
 //             name, new prop_t{*this, std::vector{size<DimensionIndex>()...}});
 //       } else {
 //         using prop_t =
 //             contiguous_vertex_property_t<T, Indexing,
 //                                          InterpolationKernels<T>...>;
 //         return m_vertex_properties.emplace(
 //             name, new prop_t{*this, std::vector{size<DimensionIndex>()...}});
 //       }
 //     }();
 //     return *dynamic_cast<typed_property_t<T>*>(newit->second.get());
 //   } else {
 //     return *dynamic_cast<typed_property_t<T>*>(it->second.get());
 //   }
 // }
 // //----------------------------------------------------------------------------
 //public:
 // template <typename T, typename Indexing = x_fastest,
 //           template <typename> typename... InterpolationKernels>
 // auto add_contiguous_vertex_property(std::string const& name) -> auto& {
 //   static_assert(sizeof...(InterpolationKernels) == num_dimensions() ||
 //                     sizeof...(InterpolationKernels) == 0,
 //                 "Number of interpolation kernels does not match number of "
 //                 "dimensions.");
 //   return add_contiguous_vertex_property<T, Indexing, InterpolationKernels...>(
 //       name, seq_t{});
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <typename T, typename Indexing = x_fastest,
 //           template <typename> typename... InterpolationKernels,
 //           size_t... DimensionIndex>
 // auto add_chunked_vertex_property(std::string const& name,
 //                                  std::index_sequence<DimensionIndex...>,
 //                                  std::vector<size_t> const& chunk_size)
 //     -> typed_property_t<T>& {
 //   update_diff_stencil_coefficients();
 //   if (auto it = m_vertex_properties.find(name);
 //       it == end(m_vertex_properties)) {
 //     auto [newit, new_prop] = [&]() {
 //       if constexpr (sizeof...(InterpolationKernels) == 0) {
 //         using prop_t = chunked_vertex_property_t<
 //             T, Indexing,
 //             decltype(((void)DimensionIndex,
 //                       default_interpolation_kernel_t<T>{}))...>;
 //         return m_vertex_properties.emplace(
 //             name, new prop_t{*this, std::vector{size<DimensionIndex>()...},
 //                              chunk_size});
 //       } else {
 //         using prop_t = chunked_vertex_property_t<T, Indexing,
 //                                                  InterpolationKernels<T>...>;
 //         return m_vertex_properties.emplace(
 //             name, new prop_t{*this, std::vector{size<DimensionIndex>()...},
 //                              chunk_size});
 //       }
 //     }();
 //     return *dynamic_cast<typed_property_t<T>*>(newit->second.get());
 //   } else {
 //     return *dynamic_cast<typed_property_t<T>*>(it->second.get());
 //   }
 // }
 // //----------------------------------------------------------------------------
 //public:
 // template <typename T, typename Indexing = x_fastest,
 //           template <typename> typename... InterpolationKernels>
 // auto add_chunked_vertex_property(std::string const&         name,
 //                                  std::vector<size_t> const& chunk_size)
 //     -> auto& {
 //   static_assert(sizeof...(InterpolationKernels) == num_dimensions() ||
 //                     sizeof...(InterpolationKernels) == 0,
 //                 "Number of interpolation kernels does not match number of "
 //                 "dimensions.");
 //   return add_chunked_vertex_property<T, Indexing, InterpolationKernels...>(
 //       name, seq_t{}, chunk_size);
 // }
 // //----------------------------------------------------------------------------
 // template <typename T>
 // auto vertex_property(std::string const& name) -> auto& {
 //   if (auto it = m_vertex_properties.find(name);
 //       it == end(m_vertex_properties)) {
 //     throw std::runtime_error{"property \"" + name + "\" not found"};
 //   } else {
 //     if (typeid(T) != it->second->type()) {
 //       throw std::runtime_error{
 //           "type of property \"" + name + "\"(" +
 //           boost::core::demangle(it->second->type().name()) +
 //           ") does not match specified type " + type_name<T>() + "."};
 //     }
 //     return *dynamic_cast<typed_property_t<T>*>(it->second.get());
 //   }
 // }
 // //----------------------------------------------------------------------------
 // template <typename T, size_t _N = num_dimensions(),
 //           std::enable_if_t<_N == 3, bool> = true>
 // void write_amira(std::string const& file_path,
 //                  std::string const& vertex_property_name) const {
 //   write_amira(file_path, vertex_property<T>(vertex_property_name));
 // }
 // //----------------------------------------------------------------------------
 // template <typename T, bool R = is_regular, size_t _N = num_dimensions(),
 //           std::enable_if_t<R, bool>       = true,
 //           std::enable_if_t<_N == 3, bool> = true>
 // void write_amira(std::string const&         file_path,
 //                  typed_property_t<T> const& prop) const {
 //   std::ofstream     outfile{file_path, std::ofstream::binary};
 //   std::stringstream header;
 //
 //   header << "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1\n\n";
 //   header << "define Lattice " << size<0>() << " " << size<1>() << " "
 //          << size<2>() << "\n\n";
 //   header << "Parameters {\n";
 //   header << "    BoundingBox " << front<0>() << " " << back<0>() << " "
 //          << front<1>() << " " << back<1>() << " " << front<2>() << " "
 //          << back<2>() << ",\n";
 //   header << "    CoordType \"uniform\"\n";
 //   header << "}\n";
 //   if constexpr (num_components_v<T> > 1) {
 //     header << "Lattice { " << type_name<internal_data_type_t<T>>() << "["
 //            << num_components_v<T> << "] Data } @1\n\n";
 //   } else {
 //     header << "Lattice { " << type_name<internal_data_type_t<T>>()
 //            << " Data } @1\n\n";
 //   }
 //   header << "# Data section follows\n@1\n";
 //   auto const header_string = header.str();
 //
 //   std::vector<T> data;
 //   data.reserve(size<0>() * size<1>() * size<2>());
 //   auto back_inserter = [&](auto const... is) {
 //     data.push_back(prop.data_at(is...));
 //   };
 //   for_loop(back_inserter, size<0>(), size<1>(), size<2>());
 //   outfile.write((char*)header_string.c_str(),
 //                 header_string.size() * sizeof(char));
 //   outfile.write((char*)data.data(), data.size() * sizeof(T));
 // }
 // //----------------------------------------------------------------------------
 //private:
 // template <typename T>
 // void write_prop_vtk(vtk::legacy_file_writer& writer, std::string const& name,
 //                     typed_property_t<T> const& prop) const {
 //   std::vector<T> data;
 //   auto           back_inserter = [&](auto const... is) {
 //     data.push_back(prop.data_at(is...));
 //   };
 //   if constexpr (num_dimensions() == 1) {
 //     for_loop(back_inserter, size<0>());
 //   } else if constexpr (num_dimensions() == 2) {
 //     for_loop(back_inserter, size<0>(), size<1>());
 //   } else if constexpr (num_dimensions() == 3) {
 //     for_loop(back_inserter, size<0>(), size<1>(), size<2>());
 //   }
 //
 //   writer.write_scalars(name, data);
 // }
 //
 //public:
 // template <size_t _N = num_dimensions(),
 //           std::enable_if_t<(_N == 1 || _N == 2 || _N == 3), bool> = true>
 // void write_vtk(std::string const& file_path) const {
 //   auto writer = [this, &file_path] {
 //     if constexpr (is_regular) {
 //       vtk::legacy_file_writer writer{file_path, vtk::STRUCTURED_POINTS};
 //       writer.set_title("tatooine grid");
 //       writer.write_header();
 //       if constexpr (num_dimensions() == 1) {
 //         writer.write_dimensions(size<0>(), 1, 1);
 //         writer.write_origin(front<0>(), 0, 0);
 //         writer.write_spacing(dimension<0>().spacing(), 0, 0);
 //       } else if constexpr (num_dimensions() == 2) {
 //         writer.write_dimensions(size<0>(), size<1>(), 1);
 //         writer.write_origin(front<0>(), front<1>(), 0);
 //         writer.write_spacing(dimension<0>().spacing(),
 //                              dimension<1>().spacing(), 0);
 //       } else if constexpr (num_dimensions() == 3) {
 //         writer.write_dimensions(size<0>(), size<1>(), size<2>());
 //         writer.write_origin(front<0>(), front<1>(), front<2>());
 //         writer.write_spacing(dimension<0>().spacing(),
 //                              dimension<1>().spacing(),
 //                              dimension<2>().spacing());
 //       }
 //       return writer;
 //     } else {
 //       vtk::legacy_file_writer writer{file_path, vtk::RECTILINEAR_GRID};
 //       writer.set_title("tatooine grid");
 //       writer.write_header();
 //       return writer;
 //     }
 //   }();
 //   // write vertex data
 //   writer.write_point_data(num_vertices());
 //   for (const auto& [name, prop] : this->m_vertex_properties) {
 //     if (prop->type() == typeid(int)) {
 //       write_prop_vtk(writer, name,
 //                      *dynamic_cast<const typed_property_t<int>*>(prop.get()));
 //     } else if (prop->type() == typeid(float)) {
 //       write_prop_vtk(
 //           writer, name,
 //           *dynamic_cast<const typed_property_t<float>*>(prop.get()));
 //     } else if (prop->type() == typeid(double)) {
 //       write_prop_vtk(
 //           writer, name,
 //           *dynamic_cast<const typed_property_t<double>*>(prop.get()));
 //     }
 //   }
 // }
};
//==============================================================================
// free functions
//==============================================================================
template <indexable_space... Dimensions>
auto vertices(grid<Dimensions...> const& g) {
  return g.vertices();
}
//==============================================================================
// deduction guides
//==============================================================================
template <typename... Dimensions>
grid(Dimensions&&...) -> grid<std::decay_t<Dimensions>...>;
//// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//template <typename Real, size_t N, size_t... Is>
//grid(boundingbox<Real, N> const& bb, std::array<size_t, N> const& res,
//     std::index_sequence<Is...>)
//    -> grid<decltype(((void)Is, std::declval<linspace<Real>()>))...>;
////==============================================================================
//// operators
////==============================================================================
//template <indexable_space... Dimensions, indexable_space AdditionalDimension>
//auto operator+(grid<Dimensions...> const& grid,
//               AdditionalDimension&&      additional_dimension) {
//  return grid.add_dimension(
//      std::forward<AdditionalDimension>(additional_dimension));
//}
//// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//template <indexable_space... Dimensions, indexable_space AdditionalDimension>
//auto operator+(AdditionalDimension&&      additional_dimension,
//               grid<Dimensions...> const& grid) {
//  return grid.add_dimension(
//      std::forward<AdditionalDimension>(additional_dimension));
//}
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
