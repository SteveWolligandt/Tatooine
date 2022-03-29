#ifndef TATOOINE_COLOR_SCALES_GYPI_H
#define TATOOINE_COLOR_SCALES_GYPI_H
//==============================================================================
#include <tatooine/concepts.h>
#include <tatooine/vec.h>
#include <tatooine/gl/texture.h>

#include <memory>
//==============================================================================
namespace tatooine::color_scales {
//==============================================================================
template <floating_point Real>
struct GYPi  {
  using real_type  = Real;
  using this_type  = GYPi<Real>;
  using color_type = vec<Real, 3>;
  static constexpr std::size_t num_samples = 17;
  //==============================================================================
 private:
  std::unique_ptr<Real[]> m_data;

 public:
  auto data() -> Real const* { return m_data.get(); }
  //==============================================================================
  GYPi()
      : m_data{new Real[]{0.15294099999999999,
                          0.39215699999999998,
                          0.098039000000000001,
                          0.246444,
                          0.50534400000000002,
                          0.117724,
                          0.35194199999999998,
                          0.614533,
                          0.16139899999999999,
                          0.47497099999999998,
                          0.71787800000000002,
                          0.24013799999999999,
                          0.61199499999999996,
                          0.811226,
                          0.392849,
                          0.74632799999999999,
                          0.89311799999999997,
                          0.56532099999999996,
                          0.85951599999999995,
                          0.94233,
                          0.74740499999999999,
                          0.92810499999999996,
                          0.96386000000000005,
                          0.87566299999999997,
                          0.96908899999999998,
                          0.96685900000000002,
                          0.96801199999999998,
                          0.98385199999999995,
                          0.91026499999999999,
                          0.94832799999999995,
                          0.97923899999999997,
                          0.83321800000000001,
                          0.91464800000000002,
                          0.949712,
                          0.72987299999999999,
                          0.86297599999999997,
                          0.90565200000000001,
                          0.58292999999999995,
                          0.76355200000000001,
                          0.85521000000000003,
                          0.41007300000000002,
                          0.65221099999999999,
                          0.79369500000000004,
                          0.183699,
                          0.53164199999999995,
                          0.68373700000000004,
                          0.063898999999999997,
                          0.420761,
                          0.556863,
                          0.0039220000000000001,
                          0.32156899999999999}} {}
  //----------------------------------------------------------------------------
  auto sample(real_type const t) const {
    if (t <= 0) {
      return color_type{m_data[0], m_data[1], m_data[2]};
    }
    if (t >= 1) {
      return color_type{m_data[(num_samples - 1) * 3],
                     m_data[(num_samples - 1) * 3 + 1],
                     m_data[(num_samples - 1) * 3 + 2]};
    }
    t *= num_samples - 1;
    auto const i = static_cast<std::size_t>(std::floor(t));
    t            = t - i;
    return color_type{m_data[i * 3] * (1 - t) + m_data[(i + 1) * 3] * t,
                   m_data[i * 3 + 1] * (1 - t) + m_data[(i + 1) * 3 + 1] * t,
                   m_data[i * 3 + 2] * (1 - t) + m_data[(i + 1) * 3 + 2] * t};
  }
  auto operator()(real_type const t) const { return sample(t); }
  //----------------------------------------------------------------------------
  auto to_gpu_tex() {
    auto gpu_data = std::vector<GLfloat>(4 * num_samples);
    for (std::size_t i = 0; i < num_samples; ++i) {
      gpu_data[i * 4]     = data()[i * 3];
      gpu_data[i * 4 + 1] = data()[i * 3 + 1];
      gpu_data[i * 4 + 2] = data()[i * 3 + 2];
      gpu_data[i * 4 + 3] = 1;
    }
    auto tex = gl::tex1rgba32f{gpu_data.data(), num_samples};
    tex.set_wrap_mode(gl::wrap_mode::clamp_to_edge);
    return tex;
  }
};
//==============================================================================
GYPi()->GYPi<double>;
//==============================================================================
}  // namespace tatooine::color_scales
//==============================================================================
#endif

