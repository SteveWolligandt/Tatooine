#ifndef TATOOINE_RENDERING_PERSPECTIVE_CAMERA_H
#define TATOOINE_RENDERING_PERSPECTIVE_CAMERA_H
//==============================================================================
#include <tatooine/ray.h>
#include <tatooine/rendering/camera.h>
#include <tatooine/rendering/matrices.h>

#include <cassert>
//==============================================================================
namespace tatooine::rendering {
//==============================================================================
/// \brief Perspective cameras are able to cast rays from one point called 'eye'
/// through an image plane.
///
/// Based on the eye position, a look-at point and a field of view angle the
/// image plane gets constructed.
/// This camera class constructs a right-handed coordinate system.
template <floating_point Real>
class perspective_camera : public camera<Real> {
 public:
  using real_t   = Real;
  using parent_t = camera<Real>;
  using this_t   = perspective_camera<Real>;
  using parent_t::eye;
  using parent_t::far;
  using parent_t::lookat;
  using parent_t::near;
  using parent_t::up;
  using parent_t::d;
  using parent_t::aspect_ratio;
  using parent_t::n;
  using parent_t::f;
  using typename parent_t::mat4;
  using typename parent_t::vec3;

 private:
  //----------------------------------------------------------------------------
  // member variables
  //----------------------------------------------------------------------------
  vec3 m_bottom_left;
  vec3 m_plane_base_x, m_plane_base_y;
  Real m_fov;

 public:
  //----------------------------------------------------------------------------
  // constructors / destructor
  //----------------------------------------------------------------------------
  /// Constructor generates bottom left image plane pixel position and pixel
  /// offset size.
  perspective_camera(vec3 const& eye, vec3 const& lookat, vec3 const& up,
                     Real const fov, Real const near, Real const far,
                     size_t const res_x, size_t const res_y)
      : parent_t{eye, lookat, up, near, far, res_x, res_y}, m_fov{fov} {
    setup();
  }
  //----------------------------------------------------------------------------
  /// Constructor generates bottom left image plane pixel position and pixel
  /// offset size.
  perspective_camera(vec3 const& eye, vec3 const& lookat, Real fov,
                     Real const near, Real const far, size_t const res_x,
                     size_t const res_y)
      : perspective_camera(eye, lookat, vec3{0, 1, 0}, fov, near, far, res_x,
                           res_y) {}
  //----------------------------------------------------------------------------
  /// Constructor generates bottom left image plane pixel position and pixel
  /// offset size.
  perspective_camera(vec3 const& eye, vec3 const& lookat, Real fov,
                     size_t const res_x, size_t const res_y)
      : perspective_camera(eye, lookat, vec3{0, 1, 0}, fov, 0.001, 1000, res_x,
                           res_y) {}
  //----------------------------------------------------------------------------
  /// Constructor generates bottom left image plane pixel position and pixel
  /// offset size.
  template <typename EyeReal, typename LookatReal, typename UpReal,
            typename FovReal>
  perspective_camera(vec<EyeReal, 3> const&    eye,
                     vec<LookatReal, 3> const& lookat, vec<UpReal, 3> const& up,
                     FovReal const fov, size_t const res_x, size_t const res_y)
      : perspective_camera(eye, lookat, up, fov, 0.001, 1000, res_x, res_y) {}
  //----------------------------------------------------------------------------
  ~perspective_camera() override = default;
  //----------------------------------------------------------------------------
  // object methods
  //----------------------------------------------------------------------------
  /// gets a ray through plane at pixel with coordinate [x,y].
  /// [0,0] is bottom left.
  /// ray goes through center of pixel
  auto ray(Real const x, Real const y) const
      -> tatooine::ray<Real, 3> override {
    auto const view_plane_point =
        m_bottom_left + x * m_plane_base_x + y * m_plane_base_y;
    return {{eye()}, {view_plane_point - eye()}};
  }
  //============================================================================
 private:
  auto setup() -> void override {
    vec3 const view_dir = normalize(lookat() - eye());
    vec3 const u        = cross(view_dir, up());
    vec3 const v        = cross(u, view_dir);
    Real const plane_half_width =
        std::tan(fov() / Real(2) * Real(M_PI) / Real(180));
    Real const plane_half_height = plane_half_width / aspect_ratio();

    m_bottom_left =
        eye() + view_dir - u * plane_half_width - v * plane_half_height;
    m_plane_base_x = u * 2 * plane_half_width / (this->plane_width() - 1);
    m_plane_base_y = v * 2 * plane_half_height / (this->plane_height() - 1);
  }
  //----------------------------------------------------------------------------
 public:
  std::unique_ptr<parent_t> clone() const override {
    return std::unique_ptr<this_t>(new this_t{*this});
  }
  //----------------------------------------------------------------------------
  auto projection_matrix() const -> mat4 override {
    return perspective_matrix(fov(), aspect_ratio(), n(), f());
  }
  //----------------------------------------------------------------------------
  auto fov() const { return m_fov; }
  //----------------------------------------------------------------------------
  auto fov_radians() const {
    static Real constexpr degree_to_radian = Real(M_PI) / Real(180);
    return m_fov* degree_to_radian;
  }
  //----------------------------------------------------------------------------
  auto set_fov(Real const fov) -> void {
    m_fov = fov;
    setup();
  }
};
//------------------------------------------------------------------------------
template <typename EyeReal, typename LookatReal, typename UpReal,
          typename FovReal, typename NearReal, typename FarReal>
perspective_camera(vec<EyeReal, 3> const&, vec<LookatReal, 3> const&,
                   vec<UpReal, 3> const&, FovReal const, NearReal const,
                   FarReal const, size_t const, size_t const)
    -> perspective_camera<
        common_type<EyeReal, LookatReal, UpReal, FovReal, NearReal, FarReal>>;
//------------------------------------------------------------------------------
template <typename EyeReal, typename LookatReal, typename FovReal,
          typename NearReal, typename FarReal>
perspective_camera(vec<EyeReal, 3> const&, vec<LookatReal, 3> const&,
                   FovReal const, NearReal const, FarReal const, size_t const,
                   size_t const)
    -> perspective_camera<
        common_type<EyeReal, LookatReal, FovReal, NearReal, FarReal>>;
//------------------------------------------------------------------------------
template <typename EyeReal, typename LookatReal, typename FovReal>
perspective_camera(vec<EyeReal, 3> const&, vec<LookatReal, 3> const&,
                   FovReal const, size_t const, size_t const)
    -> perspective_camera<common_type<EyeReal, LookatReal, FovReal>>;
//------------------------------------------------------------------------------
template <typename EyeReal, typename LookatReal, typename UpReal,
          typename FovReal>
perspective_camera(vec<EyeReal, 3> const&, vec<LookatReal, 3> const&,
                   vec<UpReal, 3> const&, FovReal const, size_t const,
                   size_t const)
    -> perspective_camera<
        common_type<EyeReal, LookatReal, UpReal, std::decay_t<FovReal>>>;
//==============================================================================
}  // namespace tatooine::rendering
//==============================================================================
#endif
