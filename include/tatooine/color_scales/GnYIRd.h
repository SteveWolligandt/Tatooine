#ifndef TATOOINE_COLOR_SCALES_GNYIRD_H
#define TATOOINE_COLOR_SCALES_GNYIRD_H
//==============================================================================
#include <tatooine/concepts.h>
#include <tatooine/vec.h>

#include <memory>
//==============================================================================
namespace tatooine::color_scales {
//==============================================================================
template <floating_point Real>
struct GnYIRd {
  using real_type  = Real;
  using this_type  = GnYIRd<Real>;
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
  GnYIRd()
      : m_data{new color_type[]{
            {0, 0.40784300000000001, 0.21568599999999999},
            {0.063975000000000004, 0.52595199999999998, 0.27720099999999998},
            {0.17793200000000001, 0.63306399999999996, 0.33271800000000001},
            {0.36493700000000001, 0.72410600000000003, 0.379469},
            {0.52795099999999995, 0.79715499999999995, 0.40222999999999998},
            {0.67843100000000001, 0.86282199999999998, 0.43344899999999997},
            {0.80392200000000003, 0.91695499999999996, 0.51464799999999999},
            {0.90941899999999998, 0.96186099999999997, 0.62506700000000004},
            {0.99992300000000001, 0.99761599999999995, 0.74502100000000004},
            {0.99746299999999999, 0.92133799999999999, 0.61707000000000001},
            {0.99500200000000005, 0.82460599999999995, 0.49988500000000002},
            {0.99254100000000001, 0.70157599999999998, 0.39654},
            {0.973472, 0.54740500000000003, 0.318108},
            {0.93902300000000005, 0.38992700000000002, 0.24552099999999999},
            {0.86766600000000005, 0.23983099999999999, 0.176624},
            {0.76239900000000005, 0.11072700000000001, 0.15132599999999999},
            {0.64705900000000005, 0, 0.14902000000000001}
        }} {}
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
GnYIRd()->GnYIRd<real_number>;
//==============================================================================
}  // namespace tatooine::color_scales
//==============================================================================
#endif

