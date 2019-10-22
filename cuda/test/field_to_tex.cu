#include <tatooine/cuda/coordinate_conversion.h>
#include <tatooine/cuda/field_to_tex.h>
#include <tatooine/cuda/global_buffer.h>
#include <tatooine/cuda/sample_field.h>
#include <tatooine/doublegyre.h>

#include <catch2/catch.hpp>

//==============================================================================
namespace tatooine {
namespace cuda {
namespace test {
//==============================================================================
__global__ void kernel(cudaTextureObject_t tex, float *vf_out, float *pos_out,
                       unsigned int *idx_out, float2 min, float2 max,
                       uint2 res) {
  const auto globalIdx = make_uint2(blockIdx.x * blockDim.x + threadIdx.x,
                                    blockIdx.y * blockDim.y + threadIdx.y);
  if (globalIdx.x >= res.x || globalIdx.y >= res.y) { return; }

  // sample vectorfield
  const auto pos = global_idx_to_domain_pos(globalIdx, min, max, res);
  const auto vf  = sample_vectorfield_steady2(tex, pos, min, max, res);

  // sample texture and assign to output array
  const size_t plainIdx     = globalIdx.x + globalIdx.y * res.x;
  pos_out[plainIdx * 2]     = pos.x;
  pos_out[plainIdx * 2 + 1] = pos.y;
  vf_out[plainIdx * 2]      = vf.x;
  vf_out[plainIdx * 2 + 1]  = vf.y;
  idx_out[plainIdx * 2]     = globalIdx.x;
  idx_out[plainIdx * 2 + 1] = globalIdx.y;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TEST_CASE("field_to_tex", "[cuda][field_to_tex][dg]") {
  // create vector field
  const numerical::doublegyre<float> v;

  // sampled vector field and upload to gpu
  const double               t = 0;
  const linspace<double>     x_domain{0, 2, 21};
  const linspace<double>     y_domain{0, 1, 11};
  auto                       d_v = to_tex<float>(v, x_domain, y_domain, t);
  cuda::global_buffer<float> d_vf_out(2 * x_domain.size() * y_domain.size());
  cuda::global_buffer<float> d_pos_out(2 * x_domain.size() * y_domain.size());
  cuda::global_buffer<unsigned int> d_idx_out(2 * x_domain.size() *
                                              y_domain.size());
  // call kernel
  const dim3 num_grids{32, 32};
  const dim3 num_threads(x_domain.size() / num_grids.x + 1,
                         y_domain.size() / num_grids.y + 1);
  kernel<<<num_threads, num_grids>>>(
      d_v.device_ptr(), d_vf_out.device_ptr(), d_pos_out.device_ptr(),
      d_idx_out.device_ptr(), make_float2(x_domain.front(), y_domain.front()),
      make_float2(x_domain.back(), y_domain.back()),
      make_uint2(x_domain.size(), y_domain.size()));

  // download data from gpu
  const auto h_vf_out  = d_vf_out.download();
  const auto h_pos_out = d_pos_out.download();
  const auto h_idx_out = d_idx_out.download();
  for (size_t i = 0; i < h_pos_out.size(); i += 2) {
    vec<float, 2>        v_gpu{h_vf_out[i], h_vf_out[i + 1]};
    vec<float, 2>        x{h_pos_out[i], h_pos_out[i + 1]};
    vec<unsigned int, 2> idx{h_idx_out[i], h_idx_out[i + 1]};
    vec<float, 2>        x_expected{x_domain[idx(0)], y_domain[idx(1)]};
    auto                 v_cpu = v(x, t);
    INFO("expected Pos: " << x_expected);
    INFO("Pos: " << x);
    INFO("idx: " << idx);
    INFO("CPU: " << v_cpu);
    INFO("GPU: " << v_gpu);
    CHECK(((x_expected(0) == x(0)) && (x_expected(1) == x(1))));
    CHECK(((v_gpu(0) == Approx(v_cpu(0)).margin(1e-3)) &&
           (v_gpu(1) == Approx(v_cpu(1)).margin(1e-3))));
  }
}

//==============================================================================
}  // namespace test
}  // namespace cuda
}  // namespace tatooine
//==============================================================================
