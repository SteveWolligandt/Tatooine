#include <tatooine/tensor.h>
#include <catch2/catch.hpp>
//==============================================================================
namespace tatooine::test {
//==============================================================================
TEST_CASE("dynamic_tensor_transposed", "[dynamic][tensor][transpose][transposed]"){
  auto A = tensor<double>::ones(3, 3);
  A(1,0) = 3;
  A(2,1) = 5;
  SECTION("non-const") {
    auto At = transposed(A);
    STATIC_REQUIRE(
        std::is_same_v<decltype(At),
                       transposed_dynamic_tensor<tensor<double>>>);
    REQUIRE(At(0, 1) == 3);
    REQUIRE(At(1, 2) == 5);
  }
  SECTION("const") {
    auto const B  = A;
    auto       Bt = transposed(B);
    STATIC_REQUIRE(std::is_same_v<decltype(Bt), const_transposed_dynamic_tensor<
                                                    tensor<double>>>);
    REQUIRE(Bt(0, 1) == 3);
    REQUIRE(Bt(1, 2) == 5);
  }
}
//==============================================================================
TEST_CASE("dynamic_tensor_diag", "[dynamic][tensor][diag]"){
  auto a = tensor<double>::ones(3);
  a(1) = 2;
  a(2) = 3;
  SECTION("non-const") {
    auto A = diag(a);
    STATIC_REQUIRE(
        std::is_same_v<decltype(A), diag_dynamic_tensor<decltype(a)>>);
    REQUIRE(A(0, 0) == 1);
    REQUIRE(A(1, 0) == 0);
    REQUIRE(A(2, 0) == 0);
    REQUIRE(A(0, 1) == 0);
    REQUIRE(A(1, 1) == 2);
    REQUIRE(A(2, 1) == 0);
    REQUIRE(A(0, 2) == 0);
    REQUIRE(A(1, 2) == 0);
    REQUIRE(A(2, 2) == 3);
  }
  SECTION("const"){
    auto const b = a;
    auto       B = diag(b);
    STATIC_REQUIRE(
        std::is_same_v<decltype(B),
                       const_diag_dynamic_tensor<tensor<double>>>);
    REQUIRE(B(0, 0) == 1);
    REQUIRE(B(1, 0) == 0);
    REQUIRE(B(2, 0) == 0);
    REQUIRE(B(0, 1) == 0);
    REQUIRE(B(1, 1) == 2);
    REQUIRE(B(2, 1) == 0);
    REQUIRE(B(0, 2) == 0);
    REQUIRE(B(1, 2) == 0);
    REQUIRE(B(2, 2) == 3);
  }
}
//==============================================================================
TEST_CASE("dynamic_tensor_contraction", "[dynamic][tensor][contraction]") {
  SECTION("matrix-vector-multiplication") {
    tensor<double> A{3, 3}, b{3}, c{3};

    A(0, 0) = 2.37546;
    A(0, 1) = 8.93258;
    A(0, 2) = 7.19590;
    A(1, 0) = 0.12527;
    A(1, 1) = 5.39804;
    A(1, 2) = 4.91963;
    A(2, 0) = 2.62273;
    A(2, 1) = 0.25176;
    A(2, 2) = 2.42668;
    b(0)    = 1.6209;
    b(1)    = 5.2425;
    b(2)    = 9.6812;
    c(0)    = 120.344;
    c(1)    = 76.130;
    c(2)    = 29.064;
    auto const d = A * b;
    REQUIRE(d(0) == Approx(c(0)));
    REQUIRE(d(1) == Approx(c(1)));
    REQUIRE(d(2) == Approx(c(2)));
  }
}
//==============================================================================
TEST_CASE("dynamic_tensor_assignement",
          "[dynamic][tensor][assignment]") {
  auto M1 = tensor<double>{
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}};
  auto M2 = tensor<double>{
    { 2,  3,  4,  5},
    { 6,  7,  8,  9},
    {10, 11, 12, 13}};
  M1 = M2;
  REQUIRE(M1.size(0) == 3);
  REQUIRE(M1.size(1) == 4);
  REQUIRE(M1(0, 0) == 2);
  REQUIRE(M1(1, 0) == 6);
  REQUIRE(M1(2, 0) == 10);
  REQUIRE(M1(0, 1) == 3);
  REQUIRE(M1(1, 1) == 7);
  REQUIRE(M1(2, 1) == 11);
  REQUIRE(M1(0, 2) ==  4);
  REQUIRE(M1(1, 2) == 8);
  REQUIRE(M1(2, 2) == 12);
  REQUIRE(M1(0, 3) ==  5);
  REQUIRE(M1(1, 3) == 9);
  REQUIRE(M1(2, 3) == 13);
}
//==============================================================================
TEST_CASE("dynamic_tensor_assignement_transposed",
          "[dynamic][tensor][assignment][transposed]") {
  auto M = tensor<double>{
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}};
  M = transposed(M);
  REQUIRE(M(0,0) == 1);
  REQUIRE(M(1,0) == 2);
  REQUIRE(M(2,0) == 3);
  REQUIRE(M(0,1) == 4);
  REQUIRE(M(1,1) == 5);
  REQUIRE(M(2,1) == 6);
  REQUIRE(M(0,2) == 7);
  REQUIRE(M(1,2) == 8);
  REQUIRE(M(2,2) == 9);
}
//==============================================================================
TEST_CASE("dynamic_tensor_vander", "[dynamic][tensor][vander]") {
  auto const xs = std::vector{1, 2, 3};
  auto const V = tensor<double>::vander(xs);
  REQUIRE(V.size(0) == 3);
  REQUIRE(V.size(1) == 3);
  REQUIRE(V(0,0) == 1);
  REQUIRE(V(1,0) == 1);
  REQUIRE(V(2,0) == 1);
  REQUIRE(V(0,1) == xs[0]);
  REQUIRE(V(1,1) == xs[1]);
  REQUIRE(V(2,1) == xs[2]);
  REQUIRE(V(0,2) == xs[0] * xs[0]);
  REQUIRE(V(1,2) == xs[1] * xs[1]);
  REQUIRE(V(2,2) == xs[2] * xs[2]);
}
//==============================================================================
}  // namespace tatooine::test
//==============================================================================
