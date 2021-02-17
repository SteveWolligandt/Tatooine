#ifndef TATOOINE_COLOR_SCALES_MAGMA_H
#define TATOOINE_COLOR_SCALES_MAGMA_H
//==============================================================================
#include <tatooine/concepts.h>
#include <tatooine/grid.h>
#include <tatooine/interpolation.h>
//==============================================================================
namespace tatooine::color_scales {
//==============================================================================
#ifdef __cpp_concepts
template <arithmetic Real>
#else
template <typename Real>
#endif
struct magma : grid<linspace<Real>> {
  using real_t      = Real;
  using this_t      = magma<real_t>;
  using dim_t       = linspace<Real>;
  using col_t       = vec<Real, 3>;
  using parent_t    = grid<dim_t>;
  using grid_t      = parent_t;
  using container_t = dynamic_multidim_array<col_t, x_fastest>;
  template <typename T>
  using interpolation_t = interpolation::linear<T>;
  template <typename ValueType, bool HasNonConstReference>
  using typed_property_t =
      typename parent_t::template typed_property_t<ValueType,
                                                   HasNonConstReference>;
  using prop_t          = typed_property_t<col_t, true>;
  using sampler_t       = sampler<prop_t, interpolation::linear>;
  //==============================================================================
  prop_t&    m_scale_prop;
  sampler_t  m_sampler;
  //==============================================================================
  magma()
      : parent_t{dim_t{Real(0), Real(1), 256}},
        m_scale_prop{
            this->template add_contiguous_vertex_property<col_t>("magma")},
        m_sampler{m_scale_prop.template sampler<interpolation::linear>()} {
    m_scale_prop(0)  = {0.001462, 0.000466, 0.013866};
    m_scale_prop(1)  = {0.002258, 0.0012949999999999999, 0.018331};
    m_scale_prop(2)  = {0.0032789999999999998, 0.0023050000000000002, 0.023708};
    m_scale_prop(3)  = {0.0045120000000000004, 0.00349, 0.029964999999999999};
    m_scale_prop(4)  = {0.0059500000000000004, 0.0048430000000000001,
                       0.037130000000000003};
    m_scale_prop(5)  = {0.0075880000000000001, 0.0063559999999999997,
                       0.044972999999999999};
    m_scale_prop(6)  = {0.0094260000000000004, 0.0080219999999999996,
                       0.052844000000000002};
    m_scale_prop(7)  = {0.011464999999999999, 0.0098279999999999999,
                       0.060749999999999998};
    m_scale_prop(8)  = {0.013708, 0.011771, 0.068667000000000006};
    m_scale_prop(9)  = {0.016156, 0.01384, 0.076603000000000004};
    m_scale_prop(10) = {0.018814999999999998, 0.016025999999999999,
                        0.084584000000000006};
    m_scale_prop(11) = {0.021691999999999999, 0.018319999999999999,
                        0.092609999999999998};
    m_scale_prop(12) = {0.024792000000000002, 0.020715000000000001, 0.100676};
    m_scale_prop(13) = {0.028122999999999999, 0.023200999999999999,
                        0.10878699999999999};
    m_scale_prop(14) = {0.031696000000000002, 0.025765, 0.116965};
    m_scale_prop(15) = {0.035520000000000003, 0.028396999999999999,
                        0.12520899999999999};
    m_scale_prop(16) = {0.039607999999999997, 0.03109, 0.13351499999999999};
    m_scale_prop(17) = {0.043830000000000001, 0.033829999999999999,
                        0.14188600000000001};
    m_scale_prop(18) = {0.048062000000000001, 0.036607000000000001,
                        0.15032699999999999};
    m_scale_prop(19) = {0.052319999999999998, 0.039406999999999998,
                        0.15884100000000001};
    m_scale_prop(20) = {0.056614999999999999, 0.042160000000000003,
                        0.16744600000000001};
    m_scale_prop(21) = {0.060949000000000003, 0.044794, 0.17612900000000001};
    m_scale_prop(22) = {0.065329999999999999, 0.047317999999999999, 0.184892};
    m_scale_prop(23) = {0.069764000000000007, 0.049725999999999999,
                        0.19373499999999999};
    m_scale_prop(24) = {0.074257000000000004, 0.052017000000000001,
                        0.20266000000000001};
    m_scale_prop(25) = {0.078814999999999996, 0.054184000000000003,
                        0.21166699999999999};
    m_scale_prop(26) = {0.083446000000000006, 0.056224999999999997,
                        0.22075500000000001};
    m_scale_prop(27) = {0.088154999999999997, 0.058132999999999997,
                        0.22992199999999999};
    m_scale_prop(28) = {0.092949000000000004, 0.059903999999999999,
                        0.23916399999999999};
    m_scale_prop(29) = {0.097833000000000003, 0.061531000000000002, 0.248477};
    m_scale_prop(30) = {0.102815, 0.063009999999999997, 0.25785400000000003};
    m_scale_prop(31) = {0.10789899999999999, 0.064335000000000003, 0.267289};
    m_scale_prop(32) = {0.113094, 0.065491999999999995, 0.27678399999999997};
    m_scale_prop(33) = {0.118405, 0.066478999999999996, 0.28632099999999999};
    m_scale_prop(34) = {0.123833, 0.067294999999999994, 0.295879};
    m_scale_prop(35) = {0.12938, 0.067934999999999995, 0.30544300000000002};
    m_scale_prop(36) = {0.13505300000000001, 0.068390999999999993, 0.315};
    m_scale_prop(37) = {0.14085800000000001, 0.068654000000000007,
                        0.32453799999999999};
    m_scale_prop(38) = {0.146785, 0.068737999999999994, 0.334011};
    m_scale_prop(39) = {0.152839, 0.068637000000000004, 0.34340399999999999};
    m_scale_prop(40) = {0.15901799999999999, 0.068353999999999998, 0.352688};
    m_scale_prop(41) = {0.16530800000000001, 0.067910999999999999,
                        0.36181600000000003};
    m_scale_prop(42) = {0.171713, 0.067305000000000004, 0.37077100000000002};
    m_scale_prop(43) = {0.17821200000000001, 0.066575999999999996,
                        0.37949699999999997};
    m_scale_prop(44) = {0.18480099999999999, 0.065731999999999999,
                        0.38797300000000001};
    m_scale_prop(45) = {0.19145999999999999, 0.064818000000000001, 0.396152};
    m_scale_prop(46) = {0.19817699999999999, 0.063862000000000002,
                        0.40400900000000001};
    m_scale_prop(47) = {0.20493500000000001, 0.062907000000000005,
                        0.41151399999999999};
    m_scale_prop(48) = {0.21171799999999999, 0.061991999999999998,
                        0.41864699999999999};
    m_scale_prop(49) = {0.21851200000000001, 0.061157999999999997,
                        0.42539199999999999};
    m_scale_prop(50) = {0.225302, 0.060444999999999999, 0.43174200000000001};
    m_scale_prop(51) = {0.23207700000000001, 0.059888999999999998, 0.437695};
    m_scale_prop(52) = {0.23882600000000001, 0.059517, 0.44325599999999998};
    m_scale_prop(53) = {0.24554300000000001, 0.059352000000000002, 0.448436};
    m_scale_prop(54) = {0.25222, 0.059415000000000003, 0.45324799999999998};
    m_scale_prop(55) = {0.258857, 0.059706000000000002, 0.45771000000000001};
    m_scale_prop(56) = {0.26544699999999999, 0.060236999999999999,
                        0.46183999999999997};
    m_scale_prop(57) = {0.27199400000000001, 0.060994, 0.46566000000000002};
    m_scale_prop(58) = {0.27849299999999999, 0.061977999999999998, 0.46919};
    m_scale_prop(59) = {0.28495100000000001, 0.063168000000000002,
                        0.47245100000000001};
    m_scale_prop(60) = {0.29136600000000001, 0.064552999999999999, 0.475462};
    m_scale_prop(61) = {0.29774, 0.066116999999999995, 0.47824299999999997};
    m_scale_prop(62) = {0.30408099999999999, 0.067835000000000006,
                        0.48081200000000002};
    m_scale_prop(63) = {0.31038199999999999, 0.069702, 0.483186};
    m_scale_prop(64) = {0.31665399999999999, 0.071690000000000004,
                        0.48537999999999998};
    m_scale_prop(65) = {0.32289899999999999, 0.073782, 0.48740800000000001};
    m_scale_prop(66) = {0.32911400000000002, 0.075971999999999998,
                        0.48928700000000003};
    m_scale_prop(67) = {0.33530799999999999, 0.078236, 0.49102400000000002};
    m_scale_prop(68) = {0.34148200000000001, 0.080563999999999997,
                        0.49263099999999999};
    m_scale_prop(69) = {0.347636, 0.082946000000000006, 0.49412099999999998};
    m_scale_prop(70) = {0.353773, 0.085373000000000004, 0.49550100000000002};
    m_scale_prop(71) = {0.359898, 0.087831000000000006, 0.496778};
    m_scale_prop(72) = {0.366012, 0.090314000000000005, 0.49796000000000001};
    m_scale_prop(73) = {0.372116, 0.092815999999999996, 0.49905300000000002};
    m_scale_prop(74) = {0.37821100000000002, 0.095332, 0.50006700000000004};
    m_scale_prop(75) = {0.384299, 0.097854999999999998, 0.50100199999999995};
    m_scale_prop(76) = {0.39038400000000001, 0.100379, 0.50186399999999998};
    m_scale_prop(77) = {0.39646700000000001, 0.10290199999999999,
                        0.50265800000000005};
    m_scale_prop(78) = {0.40254800000000002, 0.10542, 0.503386};
    m_scale_prop(79) = {0.40862900000000002, 0.10793, 0.50405199999999994};
    m_scale_prop(80) = {0.41470899999999999, 0.110431, 0.50466200000000005};
    m_scale_prop(81) = {0.42079100000000003, 0.11292000000000001,
                        0.50521499999999997};
    m_scale_prop(82) = {0.42687700000000001, 0.115395, 0.505714};
    m_scale_prop(83) = {0.43296699999999999, 0.117855, 0.50616000000000005};
    m_scale_prop(84) = {0.43906200000000001, 0.120298, 0.50655499999999998};
    m_scale_prop(85) = {0.44516299999999998, 0.122724, 0.50690100000000005};
    m_scale_prop(86) = {0.45127099999999998, 0.12513199999999999,
                        0.50719800000000004};
    m_scale_prop(87) = {0.45738600000000001, 0.127522, 0.50744800000000001};
    m_scale_prop(88) = {0.46350799999999998, 0.12989300000000001,
                        0.50765199999999999};
    m_scale_prop(89) = {0.46964, 0.132245, 0.50780899999999995};
    m_scale_prop(90) = {0.47577999999999998, 0.134577, 0.50792099999999996};
    m_scale_prop(91) = {0.481929, 0.13689100000000001, 0.50798900000000002};
    m_scale_prop(92) = {0.48808800000000002, 0.139186, 0.50801099999999999};
    m_scale_prop(93) = {0.49425799999999998, 0.141462, 0.507988};
    m_scale_prop(94) = {0.50043800000000005, 0.14371900000000001,
                        0.50792000000000004};
    m_scale_prop(95) = {0.506629, 0.145958, 0.50780599999999998};
    m_scale_prop(96) = {0.51283100000000004, 0.14817900000000001,
                        0.50764799999999999};
    m_scale_prop(97) = {0.51904499999999998, 0.15038299999999999,
                        0.50744299999999998};
    m_scale_prop(98) = {0.52527000000000001, 0.15256900000000001,
                        0.50719199999999998};
    m_scale_prop(99) = {0.53150699999999995, 0.15473899999999999,
                        0.50689499999999998};
    m_scale_prop(100) = {0.53775499999999998, 0.15689400000000001,
                         0.50655099999999997};
    m_scale_prop(101) = {0.54401500000000003, 0.15903300000000001,
                         0.50615900000000003};
    m_scale_prop(102) = {0.55028699999999997, 0.161158, 0.50571900000000003};
    m_scale_prop(103) = {0.55657100000000004, 0.163269, 0.50522999999999996};
    m_scale_prop(104) = {0.56286599999999998, 0.16536799999999999,
                         0.50469200000000003};
    m_scale_prop(105) = {0.56917200000000001, 0.16745399999999999,
                         0.50410500000000003};
    m_scale_prop(106) = {0.57548999999999995, 0.16952999999999999,
                         0.50346599999999997};
    m_scale_prop(107) = {0.58181899999999998, 0.171596, 0.50277700000000003};
    m_scale_prop(108) = {0.58815799999999996, 0.173652, 0.50203500000000001};
    m_scale_prop(109) = {0.59450800000000004, 0.175701, 0.50124100000000005};
    m_scale_prop(110) = {0.60086799999999996, 0.17774300000000001,
                         0.50039400000000001};
    m_scale_prop(111) = {0.60723800000000006, 0.17977899999999999,
                         0.49949199999999999};
    m_scale_prop(112) = {0.61361699999999997, 0.181811, 0.49853599999999998};
    m_scale_prop(113) = {0.62000500000000003, 0.18384, 0.49752400000000002};
    m_scale_prop(114) = {0.62640099999999999, 0.185867, 0.49645600000000001};
    m_scale_prop(115) = {0.63280499999999995, 0.187893, 0.49533199999999999};
    m_scale_prop(116) = {0.63921600000000001, 0.18992100000000001,
                         0.49414999999999998};
    m_scale_prop(117) = {0.64563300000000001, 0.19195200000000001,
                         0.49291000000000001};
    m_scale_prop(118) = {0.65205599999999997, 0.19398599999999999,
                         0.49161100000000002};
    m_scale_prop(119) = {0.65848300000000004, 0.19602700000000001,
                         0.49025299999999999};
    m_scale_prop(120) = {0.66491500000000003, 0.198075, 0.48883599999999999};
    m_scale_prop(121) = {0.67134899999999997, 0.20013300000000001,
                         0.48735800000000001};
    m_scale_prop(122) = {0.677786, 0.20220299999999999, 0.485819};
    m_scale_prop(123) = {0.68422400000000005, 0.204286, 0.48421900000000001};
    m_scale_prop(124) = {0.69066099999999997, 0.20638400000000001,
                         0.48255799999999999};
    m_scale_prop(125) = {0.697098, 0.20850099999999999, 0.48083500000000001};
    m_scale_prop(126) = {0.70353200000000005, 0.21063799999999999, 0.479049};
    m_scale_prop(127) = {0.70996199999999998, 0.21279699999999999,
                         0.47720099999999999};
    m_scale_prop(128) = {0.716387, 0.21498200000000001, 0.47528999999999999};
    m_scale_prop(129) = {0.72280500000000003, 0.217194, 0.47331600000000001};
    m_scale_prop(130) = {0.72921599999999998, 0.21943699999999999, 0.471279};
    m_scale_prop(131) = {0.73561600000000005, 0.22171299999999999,
                         0.46917999999999999};
    m_scale_prop(132) = {0.742004, 0.224025, 0.46701799999999999};
    m_scale_prop(133) = {0.74837799999999999, 0.22637699999999999,
                         0.46479399999999998};
    m_scale_prop(134) = {0.75473699999999999, 0.228772, 0.462509};
    m_scale_prop(135) = {0.761077, 0.231214, 0.46016200000000002};
    m_scale_prop(136) = {0.76739800000000002, 0.233705, 0.45775500000000002};
    m_scale_prop(137) = {0.77369500000000002, 0.23624899999999999, 0.455289};
    m_scale_prop(138) = {0.77996799999999999, 0.23885100000000001,
                         0.45276499999999997};
    m_scale_prop(139) = {0.78621200000000002, 0.24151400000000001,
                         0.45018399999999997};
    m_scale_prop(140) = {0.79242699999999999, 0.24424199999999999,
                         0.44754300000000002};
    m_scale_prop(141) = {0.79860799999999998, 0.24704000000000001,
                         0.44484800000000002};
    m_scale_prop(142) = {0.80475200000000002, 0.24991099999999999,
                         0.44210199999999999};
    m_scale_prop(143) = {0.81085499999999999, 0.252861, 0.439305};
    m_scale_prop(144) = {0.81691400000000003, 0.25589499999999998,
                         0.43646099999999999};
    m_scale_prop(145) = {0.82292600000000005, 0.25901600000000002,
                         0.43357299999999999};
    m_scale_prop(146) = {0.82888600000000001, 0.26222899999999999,
                         0.43064400000000003};
    m_scale_prop(147) = {0.83479099999999995, 0.26554, 0.42767100000000002};
    m_scale_prop(148) = {0.84063600000000005, 0.268953, 0.42466599999999999};
    m_scale_prop(149) = {0.84641599999999995, 0.27247300000000002,
                         0.42163099999999998};
    m_scale_prop(150) = {0.85212600000000005, 0.27610600000000002,
                         0.41857299999999997};
    m_scale_prop(151) = {0.85776300000000005, 0.27985700000000002,
                         0.41549599999999998};
    m_scale_prop(152) = {0.86331999999999998, 0.28372900000000001,
                         0.41240300000000002};
    m_scale_prop(153) = {0.86879300000000004, 0.28772799999999998,
                         0.40930299999999997};
    m_scale_prop(154) = {0.87417599999999995, 0.29185899999999998,
                         0.40620499999999998};
    m_scale_prop(155) = {0.87946400000000002, 0.29612500000000003,
                         0.40311799999999998};
    m_scale_prop(156) = {0.88465099999999997, 0.30053000000000002,
                         0.40004699999999999};
    m_scale_prop(157) = {0.88973100000000005, 0.30507899999999999,
                         0.39700200000000002};
    m_scale_prop(158) = {0.89470000000000005, 0.30977300000000002,
                         0.39399499999999998};
    m_scale_prop(159) = {0.89955200000000002, 0.31461600000000001,
                         0.39103700000000002};
    m_scale_prop(160) = {0.904281, 0.31961000000000001, 0.38813700000000001};
    m_scale_prop(161) = {0.90888400000000003, 0.32475500000000002,
                         0.38530799999999998};
    m_scale_prop(162) = {0.913354, 0.33005200000000001, 0.38256299999999999};
    m_scale_prop(163) = {0.91768899999999998, 0.33550000000000002, 0.379915};
    m_scale_prop(164) = {0.92188400000000004, 0.34109800000000001,
                         0.37737599999999999};
    m_scale_prop(165) = {0.92593700000000001, 0.34684399999999999,
                         0.37495899999999999};
    m_scale_prop(166) = {0.92984500000000003, 0.35273399999999999,
                         0.37267699999999998};
    m_scale_prop(167) = {0.93360600000000005, 0.35876400000000003,
                         0.37054100000000001};
    m_scale_prop(168) = {0.93722099999999997, 0.364929, 0.36856699999999998};
    m_scale_prop(169) = {0.94068700000000005, 0.371224, 0.36676199999999998};
    m_scale_prop(170) = {0.94400600000000001, 0.37764300000000001,
                         0.36513600000000002};
    m_scale_prop(171) = {0.94718000000000002, 0.38417800000000002, 0.363701};
    m_scale_prop(172) = {0.95021, 0.39082, 0.36246800000000001};
    m_scale_prop(173) = {0.95309900000000003, 0.397563, 0.36143799999999998};
    m_scale_prop(174) = {0.95584899999999995, 0.40439999999999998,
                         0.36061900000000002};
    m_scale_prop(175) = {0.95846399999999998, 0.41132400000000002, 0.360014};
    m_scale_prop(176) = {0.96094900000000005, 0.418323, 0.35963000000000001};
    m_scale_prop(177) = {0.96331, 0.42538999999999999, 0.35946899999999998};
    m_scale_prop(178) = {0.96554899999999999, 0.43251899999999999,
                         0.35952899999999999};
    m_scale_prop(179) = {0.96767099999999995, 0.43970300000000001,
                         0.35981000000000002};
    m_scale_prop(180) = {0.96967999999999999, 0.446936, 0.36031099999999999};
    m_scale_prop(181) = {0.97158199999999995, 0.45421, 0.36103000000000002};
    m_scale_prop(182) = {0.97338100000000005, 0.46151999999999999,
                         0.36196499999999998};
    m_scale_prop(183) = {0.975082, 0.46886100000000003, 0.36311100000000002};
    m_scale_prop(184) = {0.97668999999999995, 0.47622599999999998,
                         0.36446600000000001};
    m_scale_prop(185) = {0.97821000000000002, 0.48361199999999999,
                         0.36602499999999999};
    m_scale_prop(186) = {0.97964499999999999, 0.49101400000000001,
                         0.36778300000000003};
    m_scale_prop(187) = {0.98099999999999998, 0.49842799999999998,
                         0.36973400000000001};
    m_scale_prop(188) = {0.98227900000000001, 0.50585100000000005,
                         0.37187399999999998};
    m_scale_prop(189) = {0.98348500000000005, 0.51327999999999996,
                         0.37419799999999998};
    m_scale_prop(190) = {0.984622, 0.52071299999999998, 0.37669799999999998};
    m_scale_prop(191) = {0.98569300000000004, 0.52814799999999995,
                         0.37937100000000001};
    m_scale_prop(192) = {0.98670000000000002, 0.535582, 0.38220999999999999};
    m_scale_prop(193) = {0.98764600000000002, 0.54301500000000003, 0.38521};
    m_scale_prop(194) = {0.988533, 0.55044599999999999, 0.38836500000000002};
    m_scale_prop(195) = {0.98936299999999999, 0.55787299999999995,
                         0.39167099999999999};
    m_scale_prop(196) = {0.99013799999999996, 0.56529600000000002,
                         0.39512199999999997};
    m_scale_prop(197) = {0.99087099999999995, 0.57270600000000005,
                         0.39871400000000001};
    m_scale_prop(198) = {0.99155800000000005, 0.58010700000000004,
                         0.40244099999999999};
    m_scale_prop(199) = {0.99219599999999997, 0.58750199999999997,
                         0.40629900000000002};
    m_scale_prop(200) = {0.99278500000000003, 0.59489099999999995,
                         0.41028300000000001};
    m_scale_prop(201) = {0.99332600000000004, 0.602275, 0.41438999999999998};
    m_scale_prop(202) = {0.993834, 0.60964399999999996, 0.41861300000000001};
    m_scale_prop(203) = {0.994309, 0.61699899999999996, 0.42294999999999999};
    m_scale_prop(204) = {0.99473800000000001, 0.62434999999999996,
                         0.42739700000000003};
    m_scale_prop(205) = {0.99512199999999995, 0.63169600000000004,
                         0.43195099999999997};
    m_scale_prop(206) = {0.99548000000000003, 0.63902700000000001,
                         0.43660700000000002};
    m_scale_prop(207) = {0.99580999999999997, 0.64634400000000003, 0.441361};
    m_scale_prop(208) = {0.99609599999999998, 0.65365899999999999,
                         0.44621300000000003};
    m_scale_prop(209) = {0.99634100000000003, 0.66096900000000003,
                         0.45116000000000001};
    m_scale_prop(210) = {0.99658000000000002, 0.66825599999999996,
                         0.45619199999999999};
    m_scale_prop(211) = {0.99677499999999997, 0.67554099999999995, 0.461314};
    m_scale_prop(212) = {0.99692499999999995, 0.68282799999999999, 0.466526};
    m_scale_prop(213) = {0.99707699999999999, 0.69008800000000003,
                         0.47181099999999998};
    m_scale_prop(214) = {0.99718600000000002, 0.697349, 0.477182};
    m_scale_prop(215) = {0.99725399999999997, 0.70461099999999999,
                         0.48263499999999998};
    m_scale_prop(216) = {0.99732500000000002, 0.71184800000000004,
                         0.48815399999999998};
    m_scale_prop(217) = {0.99735099999999999, 0.71908899999999998, 0.493755};
    m_scale_prop(218) = {0.99735099999999999, 0.72632399999999997,
                         0.49942799999999998};
    m_scale_prop(219) = {0.99734100000000003, 0.733545, 0.50516700000000003};
    m_scale_prop(220) = {0.99728499999999998, 0.74077199999999999,
                         0.51098299999999997};
    m_scale_prop(221) = {0.997228, 0.74798100000000001, 0.51685899999999996};
    m_scale_prop(222) = {0.99713799999999997, 0.75519000000000003,
                         0.52280599999999999};
    m_scale_prop(223) = {0.99701899999999999, 0.76239800000000002,
                         0.52882099999999999};
    m_scale_prop(224) = {0.99689799999999995, 0.76959100000000003,
                         0.53489200000000003};
    m_scale_prop(225) = {0.99672700000000003, 0.77679500000000001,
                         0.54103900000000005};
    m_scale_prop(226) = {0.99657099999999998, 0.78397700000000003,
                         0.54723299999999997};
    m_scale_prop(227) = {0.99636899999999995, 0.79116699999999995,
                         0.55349899999999996};
    m_scale_prop(228) = {0.99616199999999999, 0.79834799999999995,
                         0.55981999999999998};
    m_scale_prop(229) = {0.99593200000000004, 0.80552699999999999,
                         0.56620199999999998};
    m_scale_prop(230) = {0.99568000000000001, 0.81270600000000004,
                         0.57264499999999996};
    m_scale_prop(231) = {0.99542399999999998, 0.81987500000000002,
                         0.57913999999999999};
    m_scale_prop(232) = {0.99513099999999999, 0.82705200000000001,
                         0.58570100000000003};
    m_scale_prop(233) = {0.99485100000000004, 0.83421299999999998,
                         0.59230700000000003};
    m_scale_prop(234) = {0.99452399999999996, 0.841387, 0.59898300000000004};
    m_scale_prop(235) = {0.99422200000000005, 0.84853999999999996,
                         0.60569600000000001};
    m_scale_prop(236) = {0.99386600000000003, 0.855711, 0.61248199999999997};
    m_scale_prop(237) = {0.99354500000000001, 0.86285900000000004,
                         0.61929900000000004};
    m_scale_prop(238) = {0.99317, 0.87002400000000002, 0.626189};
    m_scale_prop(239) = {0.99283100000000002, 0.87716799999999995,
                         0.63310900000000003};
    m_scale_prop(240) = {0.99243999999999999, 0.88432999999999995,
                         0.64009899999999997};
    m_scale_prop(241) = {0.992089, 0.89146999999999998, 0.64711600000000002};
    m_scale_prop(242) = {0.99168800000000001, 0.89862699999999995,
                         0.65420199999999995};
    m_scale_prop(243) = {0.99133199999999999, 0.90576299999999998,
                         0.66130900000000004};
    m_scale_prop(244) = {0.99092999999999998, 0.91291500000000003,
                         0.66848099999999999};
    m_scale_prop(245) = {0.99056999999999995, 0.92004900000000001,
                         0.67567500000000003};
    m_scale_prop(246) = {0.99017500000000003, 0.92719600000000002,
                         0.68292600000000003};
    m_scale_prop(247) = {0.989815, 0.93432899999999997, 0.69019799999999998};
    m_scale_prop(248) = {0.98943400000000004, 0.94147000000000003, 0.697519};
    m_scale_prop(249) = {0.98907699999999998, 0.948604, 0.70486300000000002};
    m_scale_prop(250) = {0.98871699999999996, 0.95574199999999998,
                         0.71224200000000004};
    m_scale_prop(251) = {0.988367, 0.96287800000000001, 0.71964899999999998};
    m_scale_prop(252) = {0.98803300000000005, 0.97001199999999999,
                         0.72707699999999997};
    m_scale_prop(253) = {0.98769099999999999, 0.97715399999999997,
                         0.73453599999999997};
    m_scale_prop(254) = {0.98738700000000001, 0.98428800000000005,
                         0.74200200000000005};
    m_scale_prop(255) = {0.98705299999999996, 0.99143800000000004,
                         0.74950399999999995};
  }
  //----------------------------------------------------------------------------
  auto sample(real_t const t) const {
    assert(t >= 0 && t <= 1);
    return sampler(t);
  }
  auto operator()(real_t const t) const {
    assert(t >= 0 && t <= 1);
    return m_sampler(t);
  }
};
//==============================================================================
magma()->magma<double>;
//==============================================================================
}  // namespace tatooine::color_scales
//==============================================================================
#endif

