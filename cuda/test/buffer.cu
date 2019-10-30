#include <catch2/catch.hpp>
#include <tatooine/cuda/buffer.cuh>

//==============================================================================
namespace tatooine {
namespace cuda {
namespace test {
//==============================================================================

TEST_CASE("cuda_buffer_1",
          "[cuda][buffer][upload][download][vector]") {
  int nDevices;
  cudaGetDeviceCount(&nDevices);
  for (int i = 0; i < nDevices; i++) {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, i);
    printf("Device Number: %d\n", i);
    printf("  Device name: %s\n", prop.name);
    printf("  Memory Clock Rate (KHz): %d\n",
           prop.memoryClockRate);
    printf("  Memory Bus Width (bits): %d\n",
           prop.memoryBusWidth);
    printf("  Peak Memory Bandwidth (GB/s): %f\n\n",
           2.0*prop.memoryClockRate*(prop.memoryBusWidth/8)/1.0e6);
  }
  cudaSetDevice(0);

  const std::vector<float> v1{1.0f, 2.0f};
  buffer<float>            dv1(v1);
  const auto               hv1 = dv1.download();
  for (size_t i = 0; i < v1.size(); ++i) { REQUIRE(v1[i] == hv1[i]); }
  free(dv1);
}
//==============================================================================
TEST_CASE("cuda_buffer_2",
          "[cuda][buffer][upload][download][initializer_list]") {
  buffer<float> dv1{1.0f, 2.0f};
  const auto          hv1 = dv1.download();
  REQUIRE(1.0f == hv1[0]);
  REQUIRE(2.0f == hv1[1]);
  free(dv1);
}
//==============================================================================
__global__ void kernel(buffer<float> in, buffer<float> out) {
  const size_t i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < in.size()) { out[i] = in[i]; }
}
TEST_CASE("cuda_buffer_3", "[cuda][buffer][kernel]") {
  buffer<float> in{1, 2, 3, 4, 5};
  buffer<float> out(5);
  kernel<<<5, 1>>>(in, out);
  auto hout = out.download();
  for (size_t i = 0; i < 5; ++i) {
    INFO(i);
    REQUIRE(hout[i] == i + 1);
  }
  free(in, out);
}

//==============================================================================
}  // namespace test
}  // namespace gpu
}  // namespace tatooine
//==============================================================================
