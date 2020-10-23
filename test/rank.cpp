#include <tatooine/tensor.h>
#include <catch2/catch.hpp>
//==============================================================================
namespace tatooine::test {
//==============================================================================
TEST_CASE("rank") {
  REQUIRE(rank(2) == 0);
  REQUIRE(rank(vec{1,2}) == 1);
  REQUIRE(rank(mat{{1, 2}, {3, 4}}) == 2);
}
//==============================================================================
}  // namespace tatooine::test
//==============================================================================
