#ifndef TATOOINE_CUDA_VAR_CUH
#define TATOOINE_CUDA_VAR_CUH

//==============================================================================
namespace tatooine {
namespace cuda {
//==============================================================================

template <typename T>
class var {
  //============================================================================
  // class functions
  //============================================================================
 public:
  //----------------------------------------------------------------------------
  static auto host_to_device(const T& ht) {
    T* dt;
    cudaMalloc(&dt, sizeof(T));
    cudaMemcpy(dt, &ht, sizeof(T), cudaMemcpyHostToDevice);
    return dt;
  }
  //----------------------------------------------------------------------------
  static auto device_to_host(const T* dt) {
    T ht;
    cudaMemcpy(&ht, dt, sizeof(T), cudaMemcpyHostToDevice);
    return dt;
  }

  //============================================================================
  // members
  //============================================================================
 private:
  T* m_device_ptr;

  //============================================================================
  // ctors / dtor
  //============================================================================
 public:
  //----------------------------------------------------------------------------
  var(const T& ht) : m_device_ptr{host_to_device(ht)} {}
  //----------------------------------------------------------------------------
  void free() {
    cuda::free(m_device_ptr);
  }

  //============================================================================
  // assign operator
  //============================================================================
 public:
  //----------------------------------------------------------------------------
  __device__ __host__ auto& operator=(const T& t) {
#ifdef __CUDA_ARCH__
    get() = t;
#else
    cudaMemcpy(m_device_ptr, &t, sizeof(T), cudaMemcpyHostToDevice);
#endif
    return *this;
  }
  //----------------------------------------------------------------------------
  __device__ auto& operator=(T&& t) {
    *m_device_ptr = std::move(t);
    return *this;
  }

  //============================================================================
  // methods
  //============================================================================
 public:
  //----------------------------------------------------------------------------
  operator T() const { return *m_device_ptr; }
  //----------------------------------------------------------------------------
  __device__ T& get() { return *m_device_ptr; }
  //----------------------------------------------------------------------------
  __device__ const T& get() const { return *m_device_ptr; }
  //----------------------------------------------------------------------------
  __device__ T& operator*() { return *m_device_ptr; }
  //----------------------------------------------------------------------------
  __device__ const T& operator*() const { return *m_device_ptr; }
  //----------------------------------------------------------------------------
  __host__ T download() const {
    T host_mem;
    cudaMemcpy(&host_mem, m_device_ptr, sizeof(T), cudaMemcpyDeviceToHost);
    return host_mem;
  }
  //----------------------------------------------------------------------------
  __host__ __device__ auto device_ptr() const { return m_device_ptr; }
};

//==============================================================================
}  // namespace cuda
}  // namespace tatooine
//==============================================================================

#endif
