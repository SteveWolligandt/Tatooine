#ifndef TATOOINE_CUDA_CHANNEL_FORMAT_DESCRIPTION_CUH
#define TATOOINE_CUDA_CHANNEL_FORMAT_DESCRIPTION_CUH

#include "channel_format_kind.cuh"

//==============================================================================
namespace tatooine {
namespace cuda {
//==============================================================================

template <typename T, size_t NumChannels>
constexpr auto channel_format_description() {
  return cudaChannelFormatDesc{NumChannels >= 1 ? sizeof(T) * 8 : 0,
                               NumChannels >= 2 ? sizeof(T) * 8 : 0,
                               NumChannels >= 3 ? sizeof(T) * 8 : 0,
                               NumChannels >= 4 ? sizeof(T) * 8 : 0,
                               channel_format_kind<T>()};
}

//==============================================================================
}  // namespace cuda
}  // namespace tatooine
//==============================================================================

#endif