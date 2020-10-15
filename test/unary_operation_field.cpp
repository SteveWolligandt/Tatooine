#include <tatooine/unary_operation_field.h>
#include <tatooine/analytical/fields/numerical/doublegyre.h>
#include <catch2/catch.hpp>
//==============================================================================
namespace tatooine::test {
//==============================================================================
TEST_CASE("unary_operation_field_identity_move",
          "[unary_operation_field][identity][move]") {
  constexpr auto identity = [](auto&& p) -> decltype(auto) {
    return std::forward<decltype(p)>(p);
  };
  auto v = analytical::fields::numerical::doublegyre{} | identity;
  using V = decltype(v);
  REQUIRE(!std::is_reference_v<V::internal_field_t>);
  REQUIRE(std::is_same_v<V::internal_field_t,
                          analytical::fields::numerical::doublegyre<double>>);
}
//==============================================================================
TEST_CASE("unary_operation_field_identity_ref", "[unary_operation_field][identity][ref]"){
  analytical::fields::numerical::doublegyre v;
  constexpr auto identity = [](auto&& p) -> decltype(auto) {
    return std::forward<decltype(p)>(p);
  };
  auto v_id = v | identity;
  using V = decltype(v);
  using VId = decltype(v_id);
  REQUIRE(std::is_same_v<V::real_t, VId::real_t>);
  REQUIRE(std::is_same_v<V::tensor_t, VId::tensor_t>);
  for (auto t : linspace(0.0, 10.0, 10)) {
    for (auto y : linspace(0.0, 1.0, 10)) {
      for (auto x : linspace(0.0, 2.0, 20)) {
        vec pos{x, y};
        REQUIRE(v(pos, t) == v_id(pos, t));
      }
    }
  }
}
//==============================================================================
TEST_CASE("unary_operation_field_identity_ptr",
          "[unary_operation_field][identity][ptr][pointer]") {
  analytical::fields::numerical::doublegyre v;
  parent::field<double, 2, 2>*              v_ptr = &v;
  constexpr auto identity = [](auto&& p) -> decltype(auto) {
    return std::forward<decltype(p)>(p);
  };
  auto v_id = v_ptr | identity;
  using V   = decltype(v);
  using VId = decltype(v_id);
  REQUIRE(v_ptr == v_id.internal_field());
  REQUIRE(std::is_same_v<V::real_t, VId::real_t>);
  REQUIRE(std::is_same_v<V::tensor_t, VId::tensor_t>);
  for (auto t : linspace(0.0, 10.0, 10)) {
    for (auto y : linspace(0.0, 1.0, 10)) {
      for (auto x : linspace(0.0, 2.0, 20)) {
        vec pos{x, y};
        REQUIRE(v(pos, t) == v_id(pos, t));
      }
    }
  }
}
//==============================================================================
TEST_CASE("unary_operation_field_length", "[unary_operation_field][length]") {
  analytical::fields::numerical::doublegyre v;
  auto v_len = v | [](auto const& v) { return length(v); };
  using V   = decltype(v);
  using VLen = decltype(v_len);
  REQUIRE(std::is_same_v<V::real_t, VLen::real_t>);
  REQUIRE(std::is_same_v<VLen::tensor_t, double>);
  for (auto t : linspace(0.0, 10.0, 10)) {
    for (auto y : linspace(0.0, 1.0, 10)) {
      for (auto x : linspace(0.0, 2.0, 20)) {
        REQUIRE(length(v({x, y}, t)) == v_len({x, y}, t));
      }
    }
  }
}
//==============================================================================
TEST_CASE("unary_operation_field_concat",
          "[unary_operation_field][concat]") {
  analytical::fields::numerical::doublegyre v;
  auto v_double_len = v | [](auto const& v) { return length(v); }
                        | [](auto const l) { return l * 2; };

  using V        = decltype(v);
  using VDLen    = decltype(v_double_len);
  using VDLenInt = typename VDLen::internal_field_t;
  using VDLenIntInt = typename VDLenInt::internal_field_t;
  REQUIRE(!std::is_reference_v<VDLenInt>);
  REQUIRE(std::is_reference_v<VDLenIntInt>);
  REQUIRE(std::is_same_v<std::decay_t<VDLenIntInt>, V>);

  REQUIRE(std::is_same_v<V::real_t, VDLen::real_t>);
  REQUIRE(std::is_same_v<VDLen::tensor_t, double>);
  for (auto t : linspace(0.0, 10.0, 10)) {
    for (auto y : linspace(0.0, 1.0, 10)) {
      for (auto x : linspace(0.0, 2.0, 20)) {
        REQUIRE(length(v({x, y}, t)) * 2 == v_double_len({x, y}, t));
      }
    }
  }
}
//==============================================================================
TEST_CASE("unary_operation_field_scaling",
          "[unary_operation_field][scaling]") {
  using V = analytical::fields::numerical::doublegyre<double>;
  V    v;
  auto v2 = v * 2;

  for (auto t : linspace(0.0, 10.0, 10)) {
    for (auto y : linspace(0.0, 1.0, 10)) {
      for (auto x : linspace(0.0, 2.0, 20)) {
        REQUIRE(v({x, y}, t) * 2 == v2({x, y}, t));
      }
    }
  }

  v.set_epsilon(0);
  REQUIRE(v2.internal_field().epsilon() > 0);
}
//==============================================================================
}  // namespace tatooine::test
//==============================================================================