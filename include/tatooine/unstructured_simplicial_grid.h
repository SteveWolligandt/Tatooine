#ifndef TATOOINE_UNSTRUCTURED_SIMPLICIAL_GRID_H
#define TATOOINE_UNSTRUCTURED_SIMPLICIAL_GRID_H
//==============================================================================
#ifdef TATOOINE_CDT_AVAILABLE
#include <CDT.h>
#endif
#ifdef TATOOINE_HAS_CGAL_SUPPORT
#include <tatooine/cgal.h>
#endif

#include <tatooine/axis_aligned_bounding_box.h>
#include <tatooine/packages.h>
#include <tatooine/detail/unstructured_simplicial_grid/hierarchy.h>
#include <tatooine/pointset.h>
#include <tatooine/property.h>
#include <tatooine/rectilinear_grid.h>
#include <tatooine/vtk/xml/data_array.h>
#include <tatooine/vtk_legacy.h>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <filesystem>
#include <vector>
//==============================================================================
namespace tatooine::detail::unstructured_simplicial_grid {
//==============================================================================
template <floating_point Real, std::size_t NumDimensions,
          std::size_t SimplexDim>
struct simplex_container;
//==============================================================================
template <floating_point Real, std::size_t NumDimensions,
          std::size_t SimplexDim, typename T>
struct vertex_property_sampler;
//==============================================================================
template <typename Mesh, floating_point Real, std::size_t NumDimensions,
          std::size_t SimplexDim>
struct parent;
//==============================================================================
}  // namespace tatooine::detail::unstructured_simplicial_grid
//==============================================================================
namespace tatooine {
//==============================================================================
template <floating_point Real, std::size_t NumDimensions,
          std::size_t SimplexDim = NumDimensions>
struct unstructured_simplicial_grid
    : detail::unstructured_simplicial_grid::parent<
          unstructured_simplicial_grid<Real, NumDimensions, SimplexDim>, Real,
          NumDimensions, SimplexDim> {
  using this_type =
      unstructured_simplicial_grid<Real, NumDimensions, SimplexDim>;
  using real_type = Real;
  using parent_type =
      detail::unstructured_simplicial_grid::parent<this_type, Real,
                                                   NumDimensions, SimplexDim>;
  template <typename T>
  using vertex_property_sampler_type =
      detail::unstructured_simplicial_grid::vertex_property_sampler<
          Real, NumDimensions, SimplexDim, T>;
  friend struct detail::unstructured_simplicial_grid::parent<
      this_type, Real, NumDimensions, SimplexDim>;
  using parent_type::at;
  using parent_type::num_dimensions;
  using typename parent_type::pos_type;
  using typename parent_type::vertex_handle;
  using parent_type::operator[];
  using parent_type::insert_vertex;
  using parent_type::invalid_vertices;
  using parent_type::is_valid;
  using parent_type::vertex_position_data;
  using parent_type::vertex_properties;
  using parent_type::vertices;

  using typename parent_type::const_simplex_at_return_type;
  using typename parent_type::simplex_at_return_type;

  template <typename T>
  using typed_vertex_property_type =
      typename parent_type::template typed_vertex_property_type<T>;
  using hierarchy_type = typename parent_type::hierarchy_type;
  static constexpr auto num_vertices_per_simplex() { return SimplexDim + 1; }
  static constexpr auto simplex_dimension() { return SimplexDim; }
  //----------------------------------------------------------------------------
  struct simplex_handle : handle<simplex_handle> {
    using handle<simplex_handle>::handle;
  };
  //----------------------------------------------------------------------------
  using simplex_container =
      detail::unstructured_simplicial_grid::simplex_container<
          Real, NumDimensions, SimplexDim>;
  friend struct detail::unstructured_simplicial_grid::simplex_container<
      Real, NumDimensions, SimplexDim>;
  //----------------------------------------------------------------------------
  template <typename T>
  using simplex_property_type = typed_vector_property<simplex_handle, T>;
  using simplex_property_container_type =
      std::map<std::string, std::unique_ptr<vector_property<simplex_handle>>>;
  //============================================================================
 private:
  std::vector<vertex_handle>              m_simplex_index_data;
  std::set<simplex_handle>                m_invalid_simplices;
  simplex_property_container_type         m_simplex_properties;
  mutable std::unique_ptr<hierarchy_type> m_hierarchy;

 protected:
  auto simplex_index_data() const -> auto const& {
    return m_simplex_index_data;
  }
  auto simplex_index_data() -> auto& { return m_simplex_index_data; }

  auto invalid_simplices() const -> auto const& { return m_invalid_simplices; }
  auto invalid_simplices() -> auto& { return m_invalid_simplices; }

  auto simplex_properties() const -> auto const& {
    return m_simplex_properties;
  }
  auto simplex_properties() -> auto& { return m_simplex_properties; }

 public:
  //============================================================================
  constexpr unstructured_simplicial_grid() = default;
  //============================================================================
  unstructured_simplicial_grid(unstructured_simplicial_grid const& other)
      : parent_type{other}, m_simplex_index_data{other.m_simplex_index_data} {
    for (auto const& [key, prop] : other.simplex_properties()) {
      simplex_properties().insert(std::pair{key, prop->clone()});
    }
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  unstructured_simplicial_grid(unstructured_simplicial_grid&& other) noexcept =
      default;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto operator=(unstructured_simplicial_grid const& other)
      -> unstructured_simplicial_grid& {
    parent_type::operator=(other);
    simplex_properties().clear();
    m_simplex_index_data = other.m_simplex_index_data;
    for (auto const& [key, prop] : other.simplex_properties()) {
      simplex_properties().insert(std::pair{key, prop->clone()});
    }
    return *this;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto operator=(unstructured_simplicial_grid&& other) noexcept
      -> unstructured_simplicial_grid& = default;
  //----------------------------------------------------------------------------
  explicit unstructured_simplicial_grid(std::filesystem::path const& path) {
    read(path);
  }
  //----------------------------------------------------------------------------
  unstructured_simplicial_grid(std::initializer_list<pos_type>&& vertices)
      : parent_type{std::move(vertices)} {}
  //----------------------------------------------------------------------------
  explicit unstructured_simplicial_grid(
      std::vector<vec<Real, NumDimensions>> const& positions)
      : parent_type{positions} {}
  //----------------------------------------------------------------------------
  explicit unstructured_simplicial_grid(
      std::vector<vec<Real, NumDimensions>>&& positions)
      : parent_type{std::move(positions)} {}
  //----------------------------------------------------------------------------
 private:
  template <typename... TypesToCheck, typename Prop, typename Grid>
  auto copy_prop(std::string const& name, Prop const& prop,
                 Grid const& other_grid) {
    (([&]() {
       if (prop->type() == typeid(TypesToCheck)) {
         auto const& other_prop =
             other_grid.template vertex_property<TypesToCheck>(name);
         auto& prop = this->template vertex_property<TypesToCheck>(name);
         auto  vi   = vertex_handle{0};
         other_grid.vertices().iterate_indices(
             [&](auto const... is) { prop[vi++] = other_prop(is...); });
       }
     }()),
     ...);
  }

 public:
  template <detail::rectilinear_grid::dimension DimX,
            detail::rectilinear_grid::dimension DimY>
  requires(NumDimensions == 2) &&
      (SimplexDim == 2) explicit unstructured_simplicial_grid(
          rectilinear_grid<DimX, DimY> const& g) {
    auto const gv = g.vertices();
    for (auto v : gv) {
      insert_vertex(gv[v]);
    }
    auto const gc = g.vertices();
    auto const s0 = g.size(0);
    gc.iterate_indices([&](auto const i, auto const j) {
      auto const le_bo = vertex_handle{i + j * s0};
      auto const ri_bo = vertex_handle{(i + 1) + j * s0};
      auto const le_to = vertex_handle{i + (j + 1) * s0};
      auto const ri_to = vertex_handle{(i + 1) + (j + 1) * s0};
      insert_simplex(le_bo, ri_bo, le_to);
      insert_simplex(ri_bo, ri_to, le_to);
    });
    for (auto const& [name, prop] : g.vertex_properties()) {
      copy_prop<mat4d, mat3d, mat2d, mat4f, mat3f, mat2f, vec4d, vec3d, vec2d,
                vec4f, vec3f, vec2f, double, float, std::int8_t, std::uint8_t,
                std::int16_t, std::uint16_t, std::int32_t, std::uint32_t,
                std::int64_t, std::uint64_t>(name, prop, g);
    }
  }

  template <detail::rectilinear_grid::dimension DimX,
            detail::rectilinear_grid::dimension DimY,
            detail::rectilinear_grid::dimension DimZ>
  requires(NumDimensions == 3) &&
      (SimplexDim == 3) explicit unstructured_simplicial_grid(
          rectilinear_grid<DimX, DimY, DimZ> const& g) {
    constexpr auto turned = [](std::size_t const ix, std::size_t const iy,
                               std::size_t const iz) -> bool {
      bool const xodd = ix % 2 == 0;
      bool const yodd = iy % 2 == 0;
      bool const zodd = iz % 2 == 0;

      bool turned = xodd;
      if (yodd) {
        turned = !turned;
      }
      if (zodd) {
        turned = !turned;
      }
      return turned;
    };

    auto const gv = g.vertices();
    for (auto v : gv) {
      insert_vertex(gv[v]);
    }
    auto const gc   = g.simplices();
    auto const s0   = g.size(0);
    auto const s1   = g.size(1);
    auto const s0s1 = s0 * s1;
    gc.iterate_vertices([&](auto const ix, auto const iy, auto const iz) {
      auto const le_bo_fr = vertex_handle{ix + iy * s0 + iz * s0s1};
      auto const ri_bo_fr = vertex_handle{(ix + 1) + iy * s0 + iz * s0s1};
      auto const le_to_fr = vertex_handle{ix + (iy + 1) * s0 + iz * s0s1};
      auto const ri_to_fr = vertex_handle{(ix + 1) + (iy + 1) * s0 + iz * s0s1};
      auto const le_bo_ba = vertex_handle{ix + iy * s0 + (iz + 1) * s0s1};
      auto const ri_bo_ba = vertex_handle{(ix + 1) + iy * s0 + (iz + 1) * s0s1};
      auto const le_to_ba = vertex_handle{ix + (iy + 1) * s0 + (iz + 1) * s0s1};
      auto const ri_to_ba =
          vertex_handle{(ix + 1) + (iy + 1) * s0 + (iz + 1) * s0s1};
      if (turned(ix, iy, iz)) {
        insert_simplex(le_bo_fr, ri_bo_ba, ri_to_fr,
                       le_to_ba);  // inner
        insert_simplex(le_bo_fr, ri_bo_fr, ri_to_fr,
                       ri_bo_ba);  // right front
        insert_simplex(le_bo_fr, ri_to_fr, le_to_fr,
                       le_to_ba);  // left front
        insert_simplex(ri_to_fr, ri_bo_ba, ri_to_ba,
                       le_to_ba);  // right back
        insert_simplex(le_bo_fr, le_bo_ba, ri_bo_ba,
                       le_to_ba);  // left back
      } else {
        insert_simplex(le_to_fr, ri_bo_fr, le_bo_ba,
                       ri_to_ba);  // inner
        insert_simplex(le_bo_fr, ri_bo_fr, le_to_fr,
                       le_bo_ba);  // left front
        insert_simplex(ri_bo_fr, ri_to_fr, le_to_fr,
                       ri_to_ba);  // right front
        insert_simplex(le_to_fr, le_to_ba, ri_to_ba,
                       le_bo_ba);  // left back
        insert_simplex(ri_bo_fr, ri_bo_ba, ri_to_ba,
                       le_bo_ba);  // right back
      }
    });
    for (auto const& [name, prop] : g.vertex_properties()) {
      copy_prop<mat4d, mat3d, mat2d, mat4f, mat3f, mat2f, vec4d, vec3d, vec2d,
                vec4f, vec3f, vec2f, double, float, std::int8_t, std::uint8_t,
                std::int16_t, std::uint16_t, std::int32_t, std::uint32_t,
                std::int64_t, std::uint64_t>(name, prop, g);
    }
  }
  //============================================================================
  auto operator[](simplex_handle t) const -> auto {
    return simplex_at(t.index());
  }
  auto operator[](simplex_handle t) -> auto { return simplex_at(t.index()); }
  //----------------------------------------------------------------------------
  auto at(simplex_handle t) const -> auto { return simplex_at(t.index()); }
  auto at(simplex_handle t) -> auto { return simplex_at(t.index()); }
  //----------------------------------------------------------------------------
  auto simplex_at(simplex_handle t) const -> auto {
    return simplex_at(t.index());
  }
  auto simplex_at(simplex_handle t) -> auto { return simplex_at(t.index()); }
  //----------------------------------------------------------------------------
  template <std::size_t... Seq>
  auto simplex_at(std::size_t const i) const {
    return simplex_at(i,
                      std::make_index_sequence<num_vertices_per_simplex()>{});
  }
  template <std::size_t... Seq>
  auto simplex_at(std::size_t const i) {
    return simplex_at(i,
                      std::make_index_sequence<num_vertices_per_simplex()>{});
  }
  //----------------------------------------------------------------------------
 private:
  template <std::size_t... Seq>
  auto simplex_at(std::size_t const i,
                  std::index_sequence<Seq...> /*seq*/) const
      -> const_simplex_at_return_type {
    return {simplex_index_data()[i * num_vertices_per_simplex() + Seq]...};
  }
  template <std::size_t... Seq>
  auto simplex_at(std::size_t const i, std::index_sequence<Seq...> /*seq*/)
      -> simplex_at_return_type {
    return {simplex_index_data()[i * num_vertices_per_simplex() + Seq]...};
  }
  //----------------------------------------------------------------------------
 public:
  auto insert_vertex(arithmetic auto const... comps) requires(
      sizeof...(comps) == NumDimensions) {
    auto const vi = parent_type::insert_vertex(comps...);
    // if (m_hierarchy != nullptr) {
    //  if (!m_hierarchy->insert_vertex(vi.index())) {
    //    build_hierarchy();
    //  }
    //}
    return vi;
  }
  //----------------------------------------------------------------------------
  auto insert_vertex(pos_type const& v) {
    auto const vi = parent_type::insert_vertex(v);
    // if (m_hierarchy != nullptr) {
    //  if (!m_hierarchy->insert_vertex(vi.index())) {
    //    build_hierarchy();
    //  }
    //}
    return vi;
  }
  //----------------------------------------------------------------------------
  auto insert_vertex(pos_type&& v) {
    auto const vi = parent_type::insert_vertex(std::move(v));
    // if (m_hierarchy != nullptr) {
    //  if (!m_hierarchy->insert_vertex(vi.index())) {
    //    build_hierarchy();
    //  }
    //}
    return vi;
  }
  //----------------------------------------------------------------------------
  auto remove(vertex_handle const vh) {
    using namespace std::ranges;
    parent_type::remove(vh);
    auto simplex_contains_vertex = [this, vh](auto const ch) {
      return contains(ch, vh);
    };
    copy_if(simplices(),
            std::inserter(invalid_simplices(), end(invalid_simplices())),
            simplex_contains_vertex);
  }
  //----------------------------------------------------------------------------
  auto remove(simplex_handle const ch) { invalid_simplices().insert(ch); }
  //----------------------------------------------------------------------------
  template <typename... Handles>
  auto insert_simplex(Handles const... handles) requires(
      is_same<Handles, vertex_handle>&&...) {
    static_assert(sizeof...(Handles) == num_vertices_per_simplex(),
                  "wrong number of vertices for simplex");
    (simplex_index_data().push_back(handles), ...);
    for (auto& [key, prop] : simplex_properties()) {
      prop->push_back();
    }
    return simplex_handle{simplices().size() - 1};
  }
  //----------------------------------------------------------------------------
 private:
  template <std::size_t... Seq>
  auto barycentric_coordinate(simplex_handle const& s, pos_type const& q,
                              std::index_sequence<Seq...> /*seq*/) const {
    auto A           = mat<Real, NumDimensions + 1, NumDimensions + 1>::ones();
    auto b           = vec<Real, NumDimensions + 1>::zeros();
    b(NumDimensions) = 1;

    (
        [&](auto const v) {
          for (std::size_t i = 0; i < NumDimensions; ++i) {
            A(i, v.index()) = v(i);
          }
        }(std::get<Seq>(at(s))),
        ...);

    return *solve(A,b);
  }
  //----------------------------------------------------------------------------
 public:
  auto barycentric_coordinate(simplex_handle const& s, pos_type const& q) const
      requires(NumDimensions == SimplexDim) {
    if constexpr (NumDimensions == 2) {
      auto const [v0, v1, v2] = at(s);
      auto const p0 = at(v0) - q;
      auto const p1 = at(v1) - q;
      auto const p2 = at(v2) - q;
      return vec{p1.x() * p2.y() - p2.x() * p1.y(),
                 p2.x() * p0.y() - p0.x() * p2.y(),
                 p0.x() * p1.y() - p1.x() * p0.y()} /
             ((p1.x() - p0.x()) * p2.y() + (p0.x() - p2.x()) * p1.y() +
              (p2.x() - p1.x()) * p0.y());
    } else {
      return barycentric_coordinate(
          s, q, std::make_index_sequence<NumDimensions + 1>{});
    }
  }
  //----------------------------------------------------------------------------
  /// tidies up invalid vertices
 private:
  template <std::size_t... Is>
  auto reindex_simplices_vertex_handles(std::index_sequence<Is...> /*seq*/) {
    auto dec     = [](auto i) { return ++i; };
    auto offsets = std::vector<std::size_t>(size(vertex_position_data()), 0);
    for (auto const v : invalid_vertices()) {
      auto i = begin(offsets) + v.index();
      std::ranges::transform(i, end(offsets), i, dec);
    }
    for (auto& i : m_simplex_index_data) {
      i -= offsets[i.index()];
    }
  }
  //----------------------------------------------------------------------------
 public:
  auto tidy_up() {
    reindex_simplices_vertex_handles(
        std::make_index_sequence<num_dimensions()>{});
    parent_type::tidy_up();
    auto correction = std::size_t{};
    for (auto const c : invalid_simplices()) {
      auto simplex_begin = begin(simplex_index_data()) +
                           c.index() * num_vertices_per_simplex() - correction;
      simplex_index_data().erase(simplex_begin,
                                 simplex_begin + num_vertices_per_simplex());
      for (auto const& [key, prop] : simplex_properties()) {
        prop->erase(c.index());
      }
      correction += num_vertices_per_simplex();
    }
    invalid_simplices().clear();
  }
  //----------------------------------------------------------------------------
  auto clear() {
    parent_type::clear();
    simplex_index_data().clear();
  }
  //----------------------------------------------------------------------------
  auto simplices() const { return simplex_container{this}; }
  //----------------------------------------------------------------------------
 private:
  template <std::size_t... Is>
  auto contains(simplex_handle const ch, vertex_handle const vh,
                std::index_sequence<Is...> /*seq*/) const {
    auto simplices_vertices = at(ch);
    return ((std::get<Is>(simplices_vertices) == vh) || ...);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  auto contains(simplex_handle const ch, vertex_handle const vh) const {
    return contains(ch, vh,
                    std::make_index_sequence<num_vertices_per_simplex()>{});
  }
  //----------------------------------------------------------------------------
#ifdef TATOOINE_HAS_CGAL_SUPPORT
  auto build_delaunay_mesh()
  requires (NumDimensions == 2) || (NumDimensions == 3) {
    build_delaunay_mesh(std::make_index_sequence<NumDimensions>{});
  }

 private:
  template <std::size_t... Seq>
  auto build_delaunay_mesh(std::index_sequence<Seq...> /*seq*/) -> void
  requires (NumDimensions == 2) || (NumDimensions == 3) {
    simplex_index_data().clear();
    using kernel_type = CGAL::Exact_predicates_inexact_constructions_kernel;
    using triangulation_type = 
        cgal::delaunay_triangulation_with_info<NumDimensions, kernel_type, vertex_handle>;
    using point_type = typename triangulation_type::Point;
    auto points      = std::vector<std::pair<point_type, vertex_handle>>{};
    points.reserve(vertices().size());
    for (auto v : vertices()) {
      points.emplace_back(point_type{at(v)(Seq)...}, v);
    }

    auto triangulation = triangulation_type{begin(points), end(points)};
    if constexpr (NumDimensions == 2) {
      for (auto it = triangulation.finite_faces_begin();
           it != triangulation.finite_faces_end(); ++it) {
        insert_simplex(vertex_handle{it->vertex(0)->info()},
                       vertex_handle{it->vertex(Seq + 1)->info()}...);
      }
    } else if constexpr (NumDimensions == 3) {
      for (auto it = triangulation.finite_simplices_begin();
           it != triangulation.finite_simplices_end(); ++it) {
        insert_simplex(vertex_handle{it->vertex(0)->info()},
                       vertex_handle{it->vertex(Seq + 1)->info()}...);
      }
    }
  }
 public:
  auto build_sub_delaunay_mesh(std::vector<vertex_handle> const& vertices)
  requires (NumDimensions == 2) || (NumDimensions == 3) {
    build_sub_delaunay_mesh(vertices, std::make_index_sequence<NumDimensions>{});
  }
 private:
  template <std::size_t... Seq>
  auto build_sub_delaunay_mesh(std::vector<vertex_handle> const& vertices,
                               std::index_sequence<Seq...> /*seq*/) -> void
  requires (NumDimensions == 2) || (NumDimensions == 3) {
    simplex_index_data().clear();
    using kernel_type = CGAL::Exact_predicates_inexact_constructions_kernel;
    using triangulation_type = 
        cgal::delaunay_triangulation_with_info<NumDimensions, kernel_type, vertex_handle>;
    using point_type = typename triangulation_type::Point;
    auto points      = std::vector<std::pair<point_type, vertex_handle>>{};
    points.reserve(vertices.size());
    for (auto v : vertices) {
      points.emplace_back(point_type{at(v)(Seq)...}, v);
    }

    auto triangulation = triangulation_type{begin(points), end(points)};
    if constexpr (NumDimensions == 2) {
      for (auto it = triangulation.finite_faces_begin();
           it != triangulation.finite_faces_end(); ++it) {
        insert_simplex(vertex_handle{it->vertex(0)->info()},
                       vertex_handle{it->vertex(Seq + 1)->info()}...);
      }
    } else if constexpr (NumDimensions == 3) {
      for (auto it = triangulation.finite_simplices_begin();
           it != triangulation.finite_simplices_end(); ++it) {
        insert_simplex(vertex_handle{it->vertex(0)->info()},
                       vertex_handle{it->vertex(Seq + 1)->info()}...);
      }
    }
  }
#endif

#if TATOOINE_CDT_AVAILABLE
 public:
  auto build_delaunay_mesh(
      std::vector<std::pair<vertex_handle, vertex_handle>> const& constraints)
          -> void requires(NumDimensions == 2) ||
      (NumDimensions == 3) {
    build_delaunay_mesh(constraints, std::make_index_sequence<NumDimensions>{});
  }

 private:
  template <std::size_t... Seq>
  requires(NumDimensions == 2) /*|| (NumDimensions == 3)*/
      auto build_delaunay_mesh(
          std::vector<std::pair<vertex_handle, vertex_handle>> const&
              constraints,
          std::index_sequence<Seq...> /*seq*/) -> void {
    simplex_index_data().clear();
    std::vector<CDT::Edge> edges;
    edges.reserve(size(constraints));
    boost::transform(constraints, std::back_inserter(edges),
                     [](auto const& c) -> CDT::Edge {
                       return {c.first.index(), c.second.index()};
                     });
    auto triangulation =
        CDT::Triangulation<Real>{CDT::FindingClosestPoint::BoostRTree};

    triangulation.insertVertices(
        vertices().begin(), vertices().end(),
        [this](auto const& v) { return this->vertex_at(v)(0); },
        [this](auto const& v) { return this->vertex_at(v)(1); });
    auto const duplicates_info = CDT::RemoveDuplicatesAndRemapEdges<Real>(
        triangulation.vertices, edges,
        [this](auto const& v) { return v.pos.x; },
        [this](auto const& v) { return v.pos.y; });

    triangulation.insertEdges(edges);
    triangulation.eraseSuperTriangle();
    // triangulation.eraseOuterTrianglesAndHoles();
    for (auto const& tri : triangulation.triangles) {
      insert_simplex(vertex_handle{tri.vertices[0]},
                     vertex_handle{tri.vertices[1]},
                     vertex_handle{tri.vertices[2]});
    }
  }
#endif
  //----------------------------------------------------------------------------
 public:
  template <typename T>
  auto simplex_property(std::string const& name) -> auto& {
    auto it = simplex_properties().find(name);
    if (it == end(simplex_properties())) {
      return insert_simplex_property<T>(name);
    }
    if (typeid(T) != it->second->type()) {
      throw std::runtime_error{
          "type of property \"" + name + "\"(" +
          boost::core::demangle(it->second->type().name()) +
          ") does not match specified type " + type_name<T>() + "."};
    }
    return *dynamic_cast<simplex_property_type<T>*>(
        simplex_properties().at(name).get());
  }
  //----------------------------------------------------------------------------
  template <typename T>
  auto simplex_property(std::string const& name) const -> const auto& {
    auto it = simplex_properties().find(name);
    if (it == end(simplex_properties())) {
      throw std::runtime_error{"property \"" + name + "\" not found"};
    }
    if (typeid(T) != it->second->type()) {
      throw std::runtime_error{
          "type of property \"" + name + "\"(" +
          boost::core::demangle(it->second->type().name()) +
          ") does not match specified type " + type_name<T>() + "."};
    }
    return *dynamic_cast<simplex_property_type<T>*>(
        simplex_properties().at(name).get());
  }
  //----------------------------------------------------------------------------
  auto scalar_simplex_property(std::string const& name) const -> auto const& {
    return simplex_property<tatooine::real_number>(name);
  }
  //----------------------------------------------------------------------------
  auto scalar_simplex_property(std::string const& name) -> auto& {
    return simplex_property<tatooine::real_number>(name);
  }
  //----------------------------------------------------------------------------
  auto vec2_simplex_property(std::string const& name) const -> auto const& {
    return simplex_property<vec2>(name);
  }
  //----------------------------------------------------------------------------
  auto vec2_simplex_property(std::string const& name) -> auto& {
    return simplex_property<vec2>(name);
  }
  //----------------------------------------------------------------------------
  auto vec3_simplex_property(std::string const& name) const -> auto const& {
    return simplex_property<vec3>(name);
  }
  //----------------------------------------------------------------------------
  auto vec3_simplex_property(std::string const& name) -> auto& {
    return simplex_property<vec3>(name);
  }
  //----------------------------------------------------------------------------
  auto vec4_simplex_property(std::string const& name) const -> auto const& {
    return simplex_property<vec4>(name);
  }
  //----------------------------------------------------------------------------
  auto vec4_simplex_property(std::string const& name) -> auto& {
    return simplex_property<vec4>(name);
  }
  //----------------------------------------------------------------------------
  auto mat2_simplex_property(std::string const& name) const -> auto const& {
    return simplex_property<mat2>(name);
  }
  //----------------------------------------------------------------------------
  auto mat2_simplex_property(std::string const& name) -> auto& {
    return simplex_property<mat2>(name);
  }
  //----------------------------------------------------------------------------
  auto mat3_simplex_property(std::string const& name) const -> auto const& {
    return simplex_property<mat3>(name);
  }
  //----------------------------------------------------------------------------
  auto mat3_simplex_property(std::string const& name) -> auto& {
    return simplex_property<mat3>(name);
  }
  //----------------------------------------------------------------------------
  auto mat4_simplex_property(std::string const& name) const -> auto const& {
    return simplex_property<mat4>(name);
  }
  //----------------------------------------------------------------------------
  auto mat4_simplex_property(std::string const& name) -> auto& {
    return simplex_property<mat4>(name);
  }
  //----------------------------------------------------------------------------
  template <typename T>
  auto insert_simplex_property(std::string const& name, T const& value = T{})
      -> auto& {
    auto [it, suc] = simplex_properties().insert(
        std::pair{name, std::make_unique<simplex_property_type<T>>(value)});
    auto prop = dynamic_cast<simplex_property_type<T>*>(it->second.get());
    prop->resize(simplices().size());
    return *prop;
  }
  //----------------------------------------------------------------------------
  auto write(filesystem::path const& path) const {
    auto const ext = path.extension();
    if constexpr (NumDimensions == 2 || NumDimensions == 3) {
      if (ext == ".vtk") {
        write_vtk(path);
        return;
      } else if (ext == ".vtp") {
        write_vtp(path);
        return;
      }
    }
    throw std::runtime_error(
        "Could not write unstructured_simplicial_grid. Unknown file extension: "
        "\"" +
        ext.string() + "\".");
  }
  //----------------------------------------------------------------------------
  auto write_vtk(std::filesystem::path const& path,
                 std::string const&           title = "tatooine grid") const {
    if constexpr (SimplexDim == 2 || SimplexDim == 3) {
      // tidy_up();
      write_unstructured_triangular_grid_vtk(path, title);
    }
  }
  //----------------------------------------------------------------------------
  auto write_vtp(filesystem::path const& path) const
      requires((NumDimensions == 2 || NumDimensions == 3) &&
               (SimplexDim == 1 || SimplexDim == 2)) {
    if constexpr (SimplexDim == 1) {
      write_vtp_edges(path);
    } else if constexpr (SimplexDim == 2) {
      write_vtp_triangular(path);
    }
  }

 private : auto write_vtp_edges(filesystem::path const& path) const
           requires((NumDimensions == 2 || NumDimensions == 3) &&
                    SimplexDim == 1) {
    auto file = std::ofstream{path, std::ios::binary};
    if (!file.is_open()) {
      throw std::runtime_error{"Could not write " + path.string()};
    }
    auto offset                    = std::size_t{};
    using header_type              = std::uint64_t;
    using lines_connectivity_int_type = std::int64_t;
    using lines_offset_int_type       = lines_connectivity_int_type;
    auto const num_bytes_points =
        header_type(sizeof(Real) * 3 * vertices().size());
    auto const num_bytes_lines_connectivity = simplices().size() *
                                              num_vertices_per_simplex() *
                                              sizeof(lines_connectivity_int_type);
    auto const num_bytes_lines_offsets =
        sizeof(lines_offset_int_type) * simplices().size();
    file << "<VTKFile"
         << " type=\"PolyData\""
         << " version=\"1.0\""
         << " byte_order=\"LittleEndian\""
         << " header_type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<header_type>())
         << "\">\n"
         << "<PolyData>\n"
         << "<Piece"
         << " NumberOfPoints=\"" << vertices().size() << "\""
         << " NumberOfPolys=\"0\""
         << " NumberOfVerts=\"0\""
         << " NumberOfLines=\"" << simplices().size() << "\""
         << " NumberOfStrips=\"0\""
         << ">\n"
         // Points
         << "<Points>"
         << "<DataArray"
         << " format=\"appended\""
         << " offset=\"" << offset << "\""
         << " type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<Real>())
         << "\" NumberOfComponents=\"3\"/>"
         << "</Points>\n";
    offset += num_bytes_points + sizeof(header_type);
    // Lines
    file << "<Lines>\n"
         // Lines - connectivity
         << "<DataArray format=\"appended\" offset=\"" << offset << "\" type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<lines_connectivity_int_type>())
         << "\" Name=\"connectivity\"/>\n";
    offset += num_bytes_lines_connectivity + sizeof(header_type);
    // Lines - offsets
    file << "<DataArray format=\"appended\" offset=\"" << offset << "\" type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<lines_offset_int_type>())
         << "\" Name=\"offsets\"/>\n";
    offset += num_bytes_lines_offsets + sizeof(header_type);
    file << "</Lines>\n"
         << "</Piece>\n"
         << "</PolyData>\n"
         << "<AppendedData encoding=\"raw\">\n_";
    // Writing vertex data to appended data section

    using namespace std::ranges;
    {
      file.write(reinterpret_cast<char const*>(&num_bytes_points),
                 sizeof(header_type));
      if constexpr (NumDimensions == 2) {
        auto point_data      = std::vector<vec<Real, 3>>(vertices().size());
        auto position        = [this](auto const v) -> auto& { return at(v); };
        constexpr auto to_3d = [](auto const& p) {
          return vec{p.x(), p.y(), Real(0)};
        };
        copy(vertices() | views::transform(position) | views::transform(to_3d),
             begin(point_data));
        file.write(reinterpret_cast<char const*>(point_data.data()),
                   num_bytes_points);
      } else if constexpr (NumDimensions == 3) {
        file.write(reinterpret_cast<char const*>(vertices().data()),
                   num_bytes_points);
      }
    }

    // Writing lines connectivity data to appended data section
    {
      auto connectivity_data = std::vector<lines_connectivity_int_type>(
          simplices().size() * num_vertices_per_simplex());
      auto index = [](auto const x) -> lines_connectivity_int_type {
        return x.index();
      };
      copy(simplices().data_container() | views::transform(index),
           begin(connectivity_data));
      file.write(reinterpret_cast<char const*>(&num_bytes_lines_connectivity),
                 sizeof(header_type));
      file.write(reinterpret_cast<char const*>(connectivity_data.data()),
                 num_bytes_lines_connectivity);
    }

    // Writing lines offsets to appended data section
    {
      auto offsets = std::vector<lines_offset_int_type>(
          simplices().size(), num_vertices_per_simplex());
      for (std::size_t i = 1; i < size(offsets); ++i) {
        offsets[i] += offsets[i - 1];
      };
      file.write(reinterpret_cast<char const*>(&num_bytes_lines_offsets),
                 sizeof(header_type));
      file.write(reinterpret_cast<char const*>(offsets.data()),
                 num_bytes_lines_offsets);
    }
    file << "\n</AppendedData>\n"
         << "</VTKFile>";
  }
  //----------------------------------------------------------------------------
  auto write_vtp_triangular(filesystem::path const& path) const
      requires((NumDimensions == 2 || NumDimensions == 3) && SimplexDim == 2) {
    auto file = std::ofstream{path, std::ios::binary};
    if (!file.is_open()) {
      throw std::runtime_error{"Could not write " + path.string()};
    }
    auto offset                    = std::size_t{};
    using header_type              = std::uint64_t;
    using polys_connectivity_int_type = std::int64_t;
    using polys_offset_int_type       = polys_connectivity_int_type;
    auto const num_bytes_points =
        header_type(sizeof(Real) * 3 * vertices().size());
    auto const num_bytes_polys_connectivity = simplices().size() *
                                              num_vertices_per_simplex() *
                                              sizeof(polys_connectivity_int_type);
    auto const num_bytes_polys_offsets =
        sizeof(polys_offset_int_type) * simplices().size();
    file << "<VTKFile"
         << " type=\"PolyData\""
         << " version=\"1.0\""
         << " byte_order=\"LittleEndian\""
         << " header_type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<header_type>())
         << "\">\n"
         << "<PolyData>\n"
         << "<Piece"
         << " NumberOfPoints=\"" << vertices().size() << "\""
         << " NumberOfPolys=\"" << simplices().size() << "\""
         << " NumberOfVerts=\"0\""
         << " NumberOfLines=\"0\""
         << " NumberOfStrips=\"0\""
         << ">\n"
         // Points
         << "<Points>"
         << "<DataArray"
         << " format=\"appended\""
         << " offset=\"" << offset << "\""
         << " type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<Real>())
         << "\" NumberOfComponents=\"3\"/>"
         << "</Points>\n";
    offset += num_bytes_points + sizeof(header_type);
    // Polys
    file << "<Polys>\n"
         // Polys - connectivity
         << "<DataArray format=\"appended\" offset=\"" << offset << "\" type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<polys_connectivity_int_type>())
         << "\" Name=\"connectivity\"/>\n";
    offset += num_bytes_polys_connectivity + sizeof(header_type);
    // Polys - offsets
    file << "<DataArray format=\"appended\" offset=\"" << offset << "\" type=\""
         << vtk::xml::data_array::to_string(
                vtk::xml::data_array::to_type<polys_offset_int_type>())
         << "\" Name=\"offsets\"/>\n";
    offset += num_bytes_polys_offsets + sizeof(header_type);
    file << "</Polys>\n"
         << "</Piece>\n"
         << "</PolyData>\n"
         << "<AppendedData encoding=\"raw\">\n_";
    // Writing vertex data to appended data section

    using namespace std::ranges;
    {
      file.write(reinterpret_cast<char const*>(&num_bytes_points),
                 sizeof(header_type));
      if constexpr (NumDimensions == 2) {
        auto point_data      = std::vector<vec<Real, 3>>(vertices().size());
        auto position        = [this](auto const v) -> auto& { return at(v); };
        auto it              = begin(point_data);
        for (auto const v : vertices()) {
          if constexpr (NumDimensions == 2) {
            *it = vec{at(v).x(), at(v).y(), Real(0)};
          } else {
            *it = at(v);
          }
          ++it;
        }
        file.write(reinterpret_cast<char const*>(point_data.data()),
                   num_bytes_points);
      } else if constexpr (NumDimensions == 3) {
        file.write(reinterpret_cast<char const*>(vertices().data()),
                   num_bytes_points);
      }
    }

    // Writing polys connectivity data to appended data section
    {
      auto connectivity_data = std::vector<polys_connectivity_int_type>(
          simplices().size() * num_vertices_per_simplex());
      auto index = [](auto const x) -> polys_connectivity_int_type {
        return x.index();
      };
      copy(simplices().data_container() | views::transform(index),
           begin(connectivity_data));
      file.write(reinterpret_cast<char const*>(&num_bytes_polys_connectivity),
                 sizeof(header_type));
      file.write(reinterpret_cast<char const*>(connectivity_data.data()),
                 num_bytes_polys_connectivity);
    }

    // Writing polys offsets to appended data section
    {
      auto offsets = std::vector<polys_offset_int_type>(
          simplices().size(), num_vertices_per_simplex());
      for (std::size_t i = 1; i < size(offsets); ++i) {
        offsets[i] += offsets[i - 1];
      };
      file.write(reinterpret_cast<char const*>(&num_bytes_polys_offsets),
                 sizeof(header_type));
      file.write(reinterpret_cast<char const*>(offsets.data()),
                 num_bytes_polys_offsets);
    }
    file << "\n</AppendedData>\n"
         << "</VTKFile>";
  }
  //----------------------------------------------------------------------------
  auto write_unstructured_triangular_grid_vtk(std::filesystem::path const& path,
                                              std::string const& title) const
      -> bool requires(SimplexDim == 2) {
    using namespace std::ranges;
    auto writer =
        vtk::legacy_file_writer{path, vtk::dataset_type::unstructured_grid};
    if (writer.is_open()) {
      writer.set_title(title);
      writer.write_header();
      if constexpr (NumDimensions == 2) {
        auto three_dims = [](vec<Real, 2> const& v2) {
          return vec<Real, 3>{v2(0), v2(1), 0};
        };
        auto v3s               = std::vector<vec<Real, 3>>(vertices().size());
        auto three_dimensional = views::transform(three_dims);
        copy(vertex_position_data() | three_dimensional, begin(v3s));
        writer.write_points(v3s);

      } else if constexpr (NumDimensions == 3) {
        writer.write_points(vertex_position_data());
      }

      auto vertices_per_simplex = std::vector<std::vector<std::size_t>>{};
      vertices_per_simplex.reserve(simplices().size());
      auto cell_types = std::vector<vtk::cell_type>(simplices().size(),
                                                    vtk::cell_type::triangle);
      for (auto const c : simplices()) {
        auto const [v0, v1, v2] = at(c);
        vertices_per_simplex.push_back(
            std::vector{v0.index(), v1.index(), v2.index()});
      }
      writer.write_cells(vertices_per_simplex);
      writer.write_cell_types(cell_types);

      // write vertex_handle data
      writer.write_point_data(vertices().size());
      for (auto const& [name, prop] : vertex_properties()) {
        if (prop->type() == typeid(vec<Real, 4>)) {
          auto const& casted_prop =
              *dynamic_cast<typed_vertex_property_type<vec<Real, 4>> const*>(
                  prop.get());
          writer.write_scalars(name, casted_prop.internal_container());
        } else if (prop->type() == typeid(vec<Real, 3>)) {
          auto const& casted_prop =
              *dynamic_cast<typed_vertex_property_type<vec<Real, 3>> const*>(
                  prop.get());
          writer.write_scalars(name, casted_prop.internal_container());
        } else if (prop->type() == typeid(vec<Real, 2>)) {
          auto const& casted_prop =
              *dynamic_cast<typed_vertex_property_type<vec<Real, 2>> const*>(
                  prop.get());
          writer.write_scalars(name, casted_prop.internal_container());
        } else if (prop->type() == typeid(Real)) {
          auto const& casted_prop =
              *dynamic_cast<typed_vertex_property_type<Real> const*>(prop.get());
          writer.write_scalars(name, casted_prop.internal_container());
        }
      }

      writer.close();
      return true;
    }
    return false;
  }
  //----------------------------------------------------------------------------
  auto write_unstructured_tetrahedral_grid_vtk(
      std::filesystem::path const& path, std::string const& title) const
      -> bool requires(SimplexDim == 2) {
    using boost::copy;
    using boost::adaptors::transformed;
    vtk::legacy_file_writer writer(path, vtk::dataset_type::unstructured_grid);
    if (writer.is_open()) {
      writer.set_title(title);
      writer.write_header();
      writer.write_points(vertex_position_data());

      auto vertices_per_simplex = std::vector<std::vector<std::size_t>>{};
      vertices_per_simplex.reserve(simplices().size());
      auto cell_types = std::vector<vtk::cell_type>(simplices().size(),
                                                    vtk::cell_type::tetra);
      for (auto const t : simplices()) {
        auto const [v0, v1, v2, v3] = at(t);
        vertices_per_simplex.push_back(
            std::vector{v0.index(), v1.index(), v2.index(), v3.index()});
      }
      writer.write_cells(vertices_per_simplex);
      writer.write_cell_types(cell_types);

      // write vertex_handle data
      writer.write_point_data(vertices().size());
      for (auto const& [name, prop] : vertex_properties()) {
        if (prop->type() == typeid(vec<Real, 4>)) {
        } else if (prop->type() == typeid(vec<Real, 3>)) {
        } else if (prop->type() == typeid(vec<Real, 2>)) {
          auto const& casted_prop =
              *dynamic_cast<typed_vertex_property_type<vec<Real, 2>> const*>(
                  prop.get());
          writer.write_scalars(name, casted_prop.internal_container());
        } else if (prop->type() == typeid(Real)) {
          auto const& casted_prop =
              *dynamic_cast<typed_vertex_property_type<Real> const*>(prop.get());
          writer.write_scalars(name, casted_prop.internal_container());
        }
      }

      writer.close();
      return true;
    }
    return false;
  }
  //----------------------------------------------------------------------------
 public : auto read(std::filesystem::path const& path) {
    auto ext = path.extension();
    if constexpr (NumDimensions == 2 || NumDimensions == 3) {
      if (ext == ".vtk") {
        read_vtk(path);
      }
    }
  }
  //----------------------------------------------------------------------------
  auto read_vtk(std::filesystem::path const& path) requires(
      NumDimensions == 2 || NumDimensions == 3) {
    struct listener_type : vtk::legacy_file_listener {
      unstructured_simplicial_grid& grid;
      std::vector<int>              simplices;

      explicit listener_type(unstructured_simplicial_grid& _grid) : grid{_grid} {}
      auto add_simplices(std::vector<int> const& simplices) -> void {
        std::size_t i = 0;
        while (i < size(simplices)) {
          auto const num_vertices = simplices[i++];
          if (num_vertices != num_vertices_per_simplex()) {
            throw std::runtime_error{
                "Number of vertices in file does not match number of vertices "
                "per simplex."};
          }
          for (std::size_t j = 0; j < static_cast<std::size_t>(num_vertices);
               ++j) {
            grid.simplex_index_data().push_back(vertex_handle{simplices[i++]});
          }
          for (auto& [key, prop] : grid.simplex_properties()) {
            prop->push_back();
          }
        }
      }
      auto on_simplices(std::vector<int> const& simplices) -> void override {
        add_simplices(simplices);
      }
      auto on_dataset_type(vtk::dataset_type t) -> void override {
        if (t != vtk::dataset_type::unstructured_grid &&
            t != vtk::dataset_type::polydata) {
          throw std::runtime_error{
              "[unstructured_simplicial_grid] need polydata or "
              "unstructured_grid "
              "when reading vtk legacy"};
        }
      }

      auto on_points(std::vector<std::array<float, 3>> const& ps)
          -> void override {
        for (const auto& p : ps) {
          if constexpr (NumDimensions == 2) {
            grid.insert_vertex(static_cast<Real>(p[0]),
                               static_cast<Real>(p[1]));
          }
          if constexpr (NumDimensions == 3) {
            grid.insert_vertex(static_cast<Real>(p[0]), static_cast<Real>(p[1]),
                               static_cast<Real>(p[2]));
          }
        }
      }
      auto on_points(std::vector<std::array<double, 3>> const& ps)
          -> void override {
        for (const auto& p : ps) {
          if constexpr (NumDimensions == 2) {
            grid.insert_vertex(static_cast<Real>(p[0]),
                               static_cast<Real>(p[1]));
          }
          if constexpr (NumDimensions == 3) {
            grid.insert_vertex(static_cast<Real>(p[0]), static_cast<Real>(p[1]),
                               static_cast<Real>(p[2]));
          }
        }
      }
      auto on_polygons(std::vector<int> const& ps) -> void override {
        add_simplices(ps);
      }
      auto on_scalars(std::string const& data_name,
                      std::string const& /*lookup_table_name*/,
                      std::size_t num_comps, std::vector<double> const& scalars,
                      vtk::reader_data data) -> void override {
        if (data == vtk::reader_data::point_data) {
          if (num_comps == 1) {
            auto& prop =
                grid.template insert_vertex_property<double>(data_name);
            for (auto v = vertex_handle{0}; v < vertex_handle{prop.size()};
                 ++v) {
              prop[v] = scalars[v.index()];
            }
          } else if (num_comps == 2) {
            auto& prop =
                grid.template insert_vertex_property<vec<double, 2>>(data_name);

            for (auto v = vertex_handle{0}; v < vertex_handle{prop.size()};
                 ++v) {
              for (std::size_t j = 0; j < num_comps; ++j) {
                prop[v][j] = scalars[v.index() * num_comps + j];
              }
            }
          } else if (num_comps == 3) {
            auto& prop =
                grid.template insert_vertex_property<vec<double, 3>>(data_name);
            for (auto v = vertex_handle{0}; v < vertex_handle{prop.size()};
                 ++v) {
              for (std::size_t j = 0; j < num_comps; ++j) {
                prop[v][j] = scalars[v.index() * num_comps + j];
              }
            }
          } else if (num_comps == 4) {
            auto& prop =
                grid.template insert_vertex_property<vec<double, 4>>(data_name);
            for (auto v = vertex_handle{0}; v < vertex_handle{prop.size()};
                 ++v) {
              for (std::size_t j = 0; j < num_comps; ++j) {
                prop[v][j] = scalars[v.index() * num_comps + j];
              }
            }
          }
        } else if (data == vtk::reader_data::cell_data) {
          if (num_comps == 1) {
            auto& prop =
                grid.template insert_simplex_property<double>(data_name);
            for (auto c = simplex_handle{0}; c < simplex_handle{prop.size()};
                 ++c) {
              prop[c] = scalars[c.index()];
            }
          } else if (num_comps == 2) {
            auto& prop = grid.template insert_simplex_property<vec<double, 2>>(
                data_name);

            for (auto c = simplex_handle{0}; c < simplex_handle{prop.size()};
                 ++c) {
              for (std::size_t j = 0; j < num_comps; ++j) {
                prop[c][j] = scalars[c.index() * num_comps + j];
              }
            }
          } else if (num_comps == 3) {
            auto& prop = grid.template insert_simplex_property<vec<double, 3>>(
                data_name);
            for (auto c = simplex_handle{0}; c < simplex_handle{prop.size()};
                 ++c) {
              for (std::size_t j = 0; j < num_comps; ++j) {
                prop[c][j] = scalars[c.index() * num_comps + j];
              }
            }
          } else if (num_comps == 4) {
            auto& prop = grid.template insert_simplex_property<vec<double, 4>>(
                data_name);
            for (auto c = simplex_handle{0}; c < simplex_handle{prop.size()};
                 ++c) {
              for (std::size_t j = 0; j < num_comps; ++j) {
                prop[c][j] = scalars[c.index() * num_comps + j];
              }
            }
          }
        }
      }
    } listener{*this};
    auto f = vtk::legacy_file{path};
    f.add_listener(listener);
    f.read();
  }
  //----------------------------------------------------------------------------
  constexpr auto is_valid(simplex_handle t) const {
    return std::ranges::find(invalid_simplices(), t) ==
           end(invalid_simplices());
  }
  //----------------------------------------------------------------------------
  auto build_hierarchy() const {
    clear_hierarchy();
    auto& h = hierarchy();
    if constexpr (is_uniform_tree_hierarchy<hierarchy_type>()) {
      for (auto v : vertices()) {
        h.insert_vertex(v);
      }
      for (auto c : simplices()) {
        h.insert_simplex(c);
      }
    }
  }
  //----------------------------------------------------------------------------
  auto clear_hierarchy() const { m_hierarchy.reset(); }
  //----------------------------------------------------------------------------
  auto hierarchy() const -> auto& {
    if (m_hierarchy == nullptr) {
      auto const bb = bounding_box();
      m_hierarchy   = std::make_unique<hierarchy_type>(bb.min(), bb.max(), *this);
    }
    return *m_hierarchy;
  }
  //----------------------------------------------------------------------------
  template <typename T>
  auto sampler(typed_vertex_property_type<T> const& prop) const {
    if (m_hierarchy == nullptr) {
      build_hierarchy();
    }
    return vertex_property_sampler_type<T>{*this, prop};
  }
  //--------------------------------------------------------------------------
  template <typename T>
  auto vertex_property_sampler(std::string const& name) const {
    return sampler<T>(this->template vertex_property<T>(name));
  }
  //--------------------------------------------------------------------------
  constexpr auto bounding_box() const {
    auto bb = axis_aligned_bounding_box<Real, num_dimensions()>{};
    for (auto const v : vertices()) {
      bb += at(v);
    }
    return bb;
  }
};
//==============================================================================
// unstructured_simplicial_grid()->unstructured_simplicial_grid<double, 3>;
unstructured_simplicial_grid(std::string const&)
    ->unstructured_simplicial_grid<double, 3>;
template <typename... Dims>
unstructured_simplicial_grid(rectilinear_grid<Dims...> const& g)
    -> unstructured_simplicial_grid<
        typename rectilinear_grid<Dims...>::real_type, sizeof...(Dims)>;
//==============================================================================
// namespace detail {
// template <typename MeshCont>
// auto write_grid_container_to_vtk(MeshContc onst& grids, std::string const&
// path,
//                                 std::string const& title) {
//  vtk::legacy_file_writer writer(path, vtk::POLYDATA);
//  if (writer.is_open()) {
//    std::size_t num_pts = 0;
//    std::size_t cur_first = 0;
//    for (auto const& m : grids) { num_pts += m.vertices().size(); }
//    std::vector<std::array<typename MeshCont::value_type::real_type, 3>>
//    points; std::vector<std::vector<std::size_t>> simplices;
//    points.reserve(num_pts); simplices.reserve(grids.size());
//
//    for (auto const& m : grids) {
//      // add points
//      for (auto const& v : m.vertices()) {
//        points.push_back(std::array{m[v](0), m[v](1), m[v](2)});
//      }
//
//      // add simplices
//      for (auto t : m.simplices()) {
//        simplices.emplace_back();
//        simplices.back().push_back(cur_first + m[t][0].index());
//        simplices.back().push_back(cur_first + m[t][1].index());
//        simplices.back().push_back(cur_first + m[t][2].index());
//      }
//      cur_first += m.num_vertices();
//    }
//
//    // write
//    writer.set_title(title);
//    writer.write_header();
//    writer.write_points(points);
//    writer.write_polygons(simplices);
//    //writer.write_point_data(num_pts);
//    writer.close();
//  }
//}
//}  // namespace detail
////==============================================================================
// template <floating_point Real>
// auto write_vtk(std::vector<unstructured_simplicial_grid<Real, 3>> const&
// grids, std::string const& path,
//               std::string const& title = "tatooine grids") {
//  detail::write_grid_container_to_vtk(grids, path, title);
//}
//------------------------------------------------------------------------------
static constexpr inline auto constrained_delaunay_available(
    std::size_t const NumDimensions) -> bool {
  if (NumDimensions == 2) {
    return cdt_available();
  }
  return false;
}
//==============================================================================
}  // namespace tatooine
//==============================================================================
#include <tatooine/detail/unstructured_simplicial_grid/parent.h>
#include <tatooine/detail/unstructured_simplicial_grid/simplex_container.h>
#include <tatooine/detail/unstructured_simplicial_grid/vertex_property_sampler.h>
//==============================================================================
#endif
