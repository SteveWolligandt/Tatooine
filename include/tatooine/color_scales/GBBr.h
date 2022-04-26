#ifndef TATOOINE_COLOR_SCALES_GBBR_H
#define TATOOINE_COLOR_SCALES_GBBR_H
//==============================================================================
#include <tatooine/concepts.h>
#include <tatooine/vec.h>

#include <memory>
//==============================================================================
namespace tatooine::color_scales {
//==============================================================================
template <floating_point Real>
struct GBBr {
  using real_type  = Real;
  using this_type  = GBBr<Real>;
  using color_type = vec<Real, 3>;
  static constexpr std::size_t num_samples() { return 17; }
  //==============================================================================
 private:
  std::unique_ptr<color_type[]> m_data;

 public:
  auto data_container() -> color_type* { return m_data; }
  auto data_container() const -> color_type const* { return m_data; }
  auto data() -> color_type* { return m_data.get(); }
  auto data() const -> color_type const* { return m_data.get(); }
  //==============================================================================
  GBBr()
      : m_data{new color_type[]{
            {0, 0.235294, 0.18823500000000001},
            {0.0024610000000000001, 0.33863900000000002, 0.301423},
            {0.055902, 0.44898100000000002, 0.41760900000000001},
            {0.18385199999999999, 0.56955, 0.53817800000000005},
            {0.35778500000000002, 0.70011500000000004, 0.66074600000000006},
            {0.54017700000000002, 0.81953100000000001, 0.77624000000000004},
            {0.71487900000000004, 0.89088800000000001, 0.86482099999999995},
            {0.85113399999999995, 0.93456399999999995, 0.92264500000000005},
            {0.96086099999999997, 0.959785, 0.95694000000000001},
            {0.96332200000000001, 0.92779699999999998, 0.83391000000000004},
            {0.93994599999999995, 0.86889700000000003, 0.68935000000000002},
            {0.88335300000000005, 0.77539400000000003, 0.51710900000000004},
            {0.80807399999999996, 0.62583599999999995, 0.32410600000000001},
            {0.71764700000000003, 0.47635499999999997, 0.15493999999999999},
            {0.59215700000000004, 0.35824699999999998, 0.068820000000000006},
            {0.45859299999999997, 0.26435999999999998, 0.031142},
            {0.32941199999999998, 0.18823500000000001, 0.019608}}} {}
  //----------------------------------------------------------------------------
  auto sample(real_type t) const {
    if (t <= 0) {
      return m_data[0];
    }
    if (t >= 1) {
      return m_data[(num_samples() - 1)];
    }
    t *= num_samples() - 1;
    auto const i = static_cast<std::size_t>(std::floor(t));
    t            = t - i;
    return m_data[i] * (1 - t) + m_data[i + 1] * t;
  }
  auto operator()(real_type const t) const { return sample(t); }
};
//==============================================================================
GBBr()->GBBr<real_number>;
//==============================================================================
}  // namespace tatooine::color_scales
//==============================================================================
#endif

