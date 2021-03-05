#ifdef TATOOINE_HDF5_AVAILABLE
#ifndef TATOOINE_HDF5_H
#define TATOOINE_HDF5_H
//==============================================================================
#include <hdf5.h>
#include <tatooine/chunked_multidim_array.h>
#include <tatooine/concepts.h>
#include <tatooine/filesystem.h>
#include <tatooine/lazy_reader.h>
#include <tatooine/multidim.h>
#include <tatooine/multidim_array.h>

#include <boost/range/algorithm/reverse.hpp>
#include <cassert>
#include <memory>
#include <numeric>
#include <vector>
//==============================================================================
namespace tatooine::hdf5 {
//==============================================================================
template <typename T>
struct h5_type;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <>
struct h5_type<char> {
  static auto value() { return H5T_NATIVE_CHAR; }
};
template <>
struct h5_type<int> {
  static auto value() { return H5T_NATIVE_INT; }
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <>
struct h5_type<float> {
  static auto value() { return H5T_NATIVE_FLOAT; }
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <>
struct h5_type<double> {
  static auto value() { return H5T_NATIVE_DOUBLE; }
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename T, size_t M, size_t N>
struct h5_type<mat<T, M, N>> {
  static auto value() { return h5_type<T>::value(); }
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename T, size_t N>
struct h5_type<vec<T, N>> {
  static auto value() { return h5_type<T>::value(); }
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename T, size_t... Dims>
struct h5_type<tensor<T, Dims...>> {
  static auto value() { return h5_type<T>::value(); }
};
//==============================================================================
// class attribute {
// public:
//  using this_t = attribute;
//
// private:
//  mutable std::shared_ptr<H5::H5File> m_file_id;
//  H5::Attribute                       m_attribute;
//  std::string                         m_name;
//
// public:
//  attribute(std::shared_ptr<H5::H5File>& file_id,
//            H5::Attribute const&
//            attribute, std::string const& name)
//      : m_file_im_file_id{file_id}, m_attribute{attribute},
//      m_name{name} {}
//  //----------------------------------------------------------------------------
//  attribute(attribute const&)     = default;
//  attribute(attribute&&) noexcept = default;
//  //----------------------------------------------------------------------------
//  auto operator=(attribute const&) -> attribute& = default;
//  auto operator=(attribute&&) noexcept -> attribute& = default;
//  //============================================================================
//  template <typename T>
//  auto read() {
//    std::vector<T> t;
//    auto           s = m_attribute.getInMemDataSize();
//    t.resize(s / sizeof(T));
//    m_attribute.read(h5_type<T>::value(), t.data());
//    return t;
//  }
//};
//==============================================================================
template <typename T>
class dataset {
 public:
  using this_t     = dataset<T>;
  using value_type = T;

 private:
  std::unique_ptr<hid_t>              m_file_id;
  std::unique_ptr<hid_t>              m_dataset_id;
  std::string                         m_name;
  //============================================================================
 public:
  dataset(std::unique_ptr<hid_t> const& file_id, hid_t const dataset_id,
          std::string const& name)
      : m_file_id{std::make_unique<hid_t>(*file_id)},
        m_dataset_id{std::make_unique<hid_t>(dataset_id)},
        m_name{name} {
    H5Iinc_ref(*m_file_id);
    // std::cout << name << "\n";
    // auto const num_attrs =
    //    static_cast<unsigned int>(H5Aget_num_attrs(*m_dataset_id));
    // for (unsigned int i = 0; i < num_attrs; ++i) {
    //  auto attr_id = H5Aopen_idx(*m_dataset_id, i);
    //
    //  auto        name_size = H5Aget_name(attr_id, 0, nullptr);
    //  std::string name;
    //  name.resize(name_size);
    //  H5Aget_name(attr_id, name_size + 1, &name[0]);
    //
    //  std::cout << "  " << name << '\n';
    //  auto const type_id = H5Aget_type(attr_id);
    //  if (name == "vsIndexOrder" && H5Tget_class(type_id) == H5T_STRING) {
    //    auto const size = H5Tget_size(type_id);
    //
    //    std::string val;
    //    val.resize(size);
    //    H5Aread(attr_id, H5Tget_native_type(type_id, H5T_DIR_ASCEND),
    //    &val[0]);
    //    // std::cout << "    " << val << '\n';
    //
    //    if (val == "compMinorF") {
    //      m_index_order = index_order::Fortran;
    //    }
    //  }
    //  H5Tclose(type_id);
    //  H5Aclose(attr_id);
    //}
  }
  //----------------------------------------------------------------------------
  dataset(dataset const& other)
      : m_file_id{std::make_unique<hid_t>(*other.m_file_id)},
        m_dataset_id{std::make_unique<hid_t>(*other.m_dataset_id)},
        m_name{other.m_name} {
    H5Iinc_ref(*m_file_id);
    H5Iinc_ref(*m_dataset_id);
  }
  //----------------------------------------------------------------------------
  dataset(dataset&&) noexcept = default;
  //----------------------------------------------------------------------------
  auto operator=(dataset const& other) -> dataset& {
    if (m_file_id != nullptr) {
      H5Fclose(*m_file_id);
    }
    if (m_dataset_id != nullptr) {
      H5Dclose(*m_dataset_id);
    }
    m_file_id    = std::make_unique<hid_t>(*other.m_file_id);
    m_dataset_id = std::make_unique<hid_t>(*other.m_dataset_id);
    m_name       = other.m_name;
    H5Iinc_ref(*m_file_id);
    H5Iinc_ref(*m_dataset_id);
    return *this;
  }
  //----------------------------------------------------------------------------
  auto operator=(dataset&&) noexcept -> dataset& = default;
  //----------------------------------------------------------------------------
  ~dataset() {
    if (m_file_id != nullptr) {
      H5Fclose(*m_file_id);
    }
    if (m_dataset_id != nullptr) {
      H5Dclose(*m_dataset_id);
    }
  }
  //============================================================================
  auto write(T const* data) {
    H5Dwrite(*m_dataset_id, h5_type<T>::value(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
             data);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto write(std::vector<T> const& data) {
    H5Dwrite(*m_dataset_id, h5_type<T>::value(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
             data.data());
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <size_t N>
  auto write(std::array<T, N> const& data) {
    H5Dwrite(*m_dataset_id, h5_type<T>::value(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
             data.data());
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef __cpp_concepts
  template <range Range>
#else
  template <typename Range, enable_if<is_range<Range>> = true>
#endif
  auto write(Range r) {
    write(std::vector(begin(r), end(r)));
  }
//----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <typename IndexOrder>
#else
  template <typename IndexOrder>
#endif
  auto write(dynamic_multidim_array<T, IndexOrder> const& data) -> void {
    write(data.data_ptr());
  }
//  //----------------------------------------------------------------------------
//#ifdef __cpp_concepts
//  template <typename IndexOrder, integral... Is>
//#else
//  template <typename IndexOrder, typename... Is,
//            enable_if<is_arithmetic<Is...>> = true>
//#endif
//  auto write(dynamic_multidim_array<T, IndexOrder> const& data, Is const...
//  is)
//      -> void {
//    auto const s = data.size();
//    write<IndexOrder>(data.data_ptr(),
//                      std::vector<hsize_t>{static_cast<hsize_t>(is)...},
//                      std::vector<hsize_t>(begin(s), end(s)));
//  }
//----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <typename IndexOrder = x_fastest, integral... Is>
#else
  template <typename IndexOrder             = x_fastest, typename... Is,
            enable_if<is_arithmetic<Is...>> = true>
#endif
  auto write(T const& data, Is const... is) -> void {
    write<IndexOrder>(&data, std::vector<hsize_t>{static_cast<hsize_t>(is)...},
                      std::vector<hsize_t>(sizeof...(Is), 1));
  }
  //----------------------------------------------------------------------------
  auto write(T const& data, std::vector<size_t> const& offset) -> void {
    write(&data, std::vector<hsize_t>(begin(offset), end(offset)),
          std::vector<hsize_t>(size(offset), 1));
  }
  //----------------------------------------------------------------------------
  auto write(T const& data, std::vector<hsize_t> offset) -> void {
    write(&data, std::move(offset), std::vector<hsize_t>(size(offset), 1));
  }
  //----------------------------------------------------------------------------
  auto write(std::vector<T> const& data, std::vector<size_t> const& offset,
             std::vector<size_t> const& count) -> void {
    write(data.data(), std::vector<hsize_t>(begin(offset), end(offset)),
          std::vector<hsize_t>(begin(count), end(count)));
  }
  //----------------------------------------------------------------------------
  auto write(T const* data, std::vector<size_t> const& offset,
             std::vector<size_t> const& count) -> void {
    write(data, std::vector<hsize_t>(begin(offset), end(offset)),
          std::vector<hsize_t>(begin(count), end(count)));
  }
  //----------------------------------------------------------------------------
  auto write(std::vector<T> const& data, std::vector<hsize_t> offset,
             std::vector<hsize_t> count) -> void {
    write(data.data(), std::move(offset), std::move(count));
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto write(T const* data, std::vector<hsize_t> offset,
             std::vector<hsize_t> count) -> void {
    assert(offset.size() == count.size());

    hid_t      dspace = H5Dget_space(*m_dataset_id);
    auto const rank   = H5Sget_simple_extent_ndims(dspace);
    boost::reverse(offset);
    boost::reverse(count);
    H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset.data(), nullptr,
                        count.data(), nullptr);
    auto memspace = H5Screate_simple(rank, count.data(), nullptr);
    H5Dwrite(*m_dataset_id, h5_type<T>::value(), memspace, dspace, H5P_DEFAULT,
             data);
    H5Sclose(memspace);
    H5Sclose(dspace);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto read() const {
    dynamic_multidim_array<T, IndexOrder> arr;
    read(arr);
    return arr;
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder>
  auto read(dynamic_multidim_array<T, IndexOrder>& arr) const {
    hid_t      dspace   = H5Dget_space(*m_dataset_id);
    auto const num_dims = H5Sget_simple_extent_ndims(dspace);
    auto       size     = std::make_unique<hsize_t[]>(num_dims);
    H5Sget_simple_extent_dims(dspace, size.get(), nullptr);
    std::reverse(size.get(), size.get() + num_dims);
    bool must_resize = (unsigned int)num_dims != arr.num_dimensions();
    if (!must_resize) {
      for (int i = 0; i < num_dims; ++i) {
        if (arr.size(i) != size[i]) {
          must_resize = true;
          break;
        }
      }
    }
    if (must_resize) {
      arr.resize(std::vector<size_t>(size.get(), size.get() + num_dims));
    }

    H5Dread(*m_dataset_id, h5_type<T>::value(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
            arr.data_ptr());
    H5Sclose(dspace);
  }
  //----------------------------------------------------------------------------
  auto read_as_vector() const {
    std::vector<T> data;
    read(data);
    return data;
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto read(std::vector<T>& data) const {
    hid_t      dspace   = H5Dget_space(*m_dataset_id);
    auto const num_dims = H5Sget_simple_extent_ndims(dspace);
    auto       size     = std::make_unique<hsize_t[]>(num_dims);
    H5Sget_simple_extent_dims(dspace, size.get(), nullptr);
    std::reverse(size.get(), size.get() + num_dims);
    size_t num_entries = 1;
    for (int i = 0; i < num_dims; ++i) {
      num_entries *= size[i];
    }
    if (data.size() != num_entries) {
      data.resize(num_entries);
    }

    H5Dread(*m_dataset_id, h5_type<T>::value(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
            data.data());
    H5Sclose(dspace);
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder>
  auto read_chunk(std::vector<size_t> const&             offset,
                  std::vector<size_t> const&             count,
                  dynamic_multidim_array<T, IndexOrder>& arr) const {
    read_chunk(std::vector<hsize_t>(begin(offset), end(offset)),
               std::vector<hsize_t>(begin(count), end(count)), arr);
    return arr;
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto read_chunk(std::vector<size_t> const& offset,
                  std::vector<size_t> const& count) const {
    return read_chunk<IndexOrder>(
        std::vector<hsize_t>(begin(offset), end(offset)),
        std::vector<hsize_t>(begin(count), end(count)));
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto read_chunk(std::vector<hsize_t> const& offset,
                  std::vector<hsize_t> const& count) const {
    dynamic_multidim_array<T, IndexOrder> arr;
    read_chunk(offset, count, arr);
    return arr;
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder>
  auto read_chunk(std::vector<hsize_t> offset, std::vector<hsize_t> count,
                  dynamic_multidim_array<T, IndexOrder>& arr) const {
    assert(offset.size() == count.size());

    hid_t      dspace = H5Dget_space(*m_dataset_id);
    auto const rank   = H5Sget_simple_extent_ndims(dspace);

    if (static_cast<unsigned int>(rank) != arr.num_dimensions()) {
      arr.resize(count);
    } else {
      for (int i = 0; i < rank; ++i) {
        if (arr.size(i) != count[i]) {
          arr.resize(count);
          break;
        }
      }
    }
    boost::reverse(offset);
    boost::reverse(count);
    H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset.data(), nullptr,
                        count.data(), nullptr);

    auto memspace = H5Screate_simple(rank, count.data(), nullptr);
    H5Dread(*m_dataset_id, h5_type<T>::value(), memspace, dspace, H5P_DEFAULT,
            arr.data_ptr());
    H5Sclose(dspace);
    H5Sclose(memspace);
    return arr;
  }
  //----------------------------------------------------------------------------
#ifdef __cpp_concepts
  template <integral... Is>
#else
  template <typename... Is, enable_if<is_integral<Is...>> = true>
#endif
  auto read(Is const... is) const {
    std::vector<hsize_t> offset{static_cast<hsize_t>(is)...};
    std::vector<hsize_t> count(sizeof...(Is), 1);

    hid_t      dspace = H5Dget_space(*m_dataset_id);
    auto const rank   = H5Sget_simple_extent_ndims(dspace);
    auto       size   = std::make_unique<hsize_t[]>(rank);
    boost::reverse(offset);
    H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset.data(), nullptr,
                        count.data(), nullptr);
    auto memspace = H5Screate_simple(rank, count.data(), nullptr);
    T    data;
    H5Dread(*m_dataset_id, h5_type<T>::value(), memspace, dspace, H5P_DEFAULT,
            &data);
    H5Sclose(dspace);
    H5Sclose(memspace);
    return data;
  }
  //----------------------------------------------------------------------------
  auto num_dimensions() const {
    auto dspace = H5Dget_space(*m_dataset_id);
    auto ndims  = H5Sget_simple_extent_ndims(dspace);
    H5Sclose(dspace);
    return ndims;
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto size(size_t i) const {
    return size<IndexOrder>()[i];
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto size() const {
    hid_t      dspace   = H5Dget_space(*m_dataset_id);
    auto const num_dims = H5Sget_simple_extent_ndims(dspace);
    auto       size     = std::make_unique<hsize_t[]>(num_dims);
    H5Sget_simple_extent_dims(dspace, size.get(), nullptr);
    std::vector<size_t> s(size.get(), size.get() + num_dims);
    boost::reverse(s);
    H5Sclose(dspace);
    return s;
  }
  //----------------------------------------------------------------------------
  template <typename IndexOrder = x_fastest>
  auto read_lazy(std::vector<size_t> const& chunk_size) {
    return lazy_reader<this_t, IndexOrder>{*this, chunk_size};
  }
  //----------------------------------------------------------------------------
  auto name() const -> auto const& { return m_name; }
};
//==============================================================================
// class group {
// publicN:
//  using this_t = group;
//
// private:
//  mutable std::shared_ptr<H5::H5File> m_file_id;
//  H5::Group                           m_group;
//  std::string                         m_name;
//
// public:
//  group(std::shared_ptr<H5::H5File>& file_id, 
//          H5::Group const& group, std::string const& name)
//      : m_file_id{file_id}, m_group{group}, m_name{name} {}
//  //----------------------------------------------------------------------------
//  group(group const&)     = default;
//  group(group&&) noexcept = default;
//  //----------------------------------------------------------------------------
//  auto operator=(group const&) -> group& = default;
//  auto operator=(group&&) noexcept -> group& = default;
//  //============================================================================
//  auto attribute(std::string const& attribute_name) {
//    return hdf5::attribute{
//        m_file_id, m_group.openAttribute(attribute_name),
//        attribute_name};
//  }
//  //============================================================================
//  template <typename T>
//  auto dataset(std::string const& dataset_name) {
//    return hdf5::dataset<T>{
//        m_file_id, m_group.openDataSet(dataset_name), dataset_name};
//  }
//  //----------------------------------------------------------------------------
//#ifdef __cpp_concepts
//  template <typename T, integral... Size>
//#else
//  template <typename T, typename... Size, enable_if<is_integral<Size...>> =
//  true>
//#endif
//  auto add_dataset(std::string const& dataset_name, Size... size) {
//    H5::AtomType data_type{h5_type<T>::value()};
//    hsize_t      dimsf[]{static_cast<hsize_t>(size)...};  // data set
//    dimensions std::reverse(dimsf, dimsf + sizeof...(Size)); return
//    hdf5::dataset<T>{
//        m_file_id,
//        m_group.createDataSet(dataset_name, data_type,
//                              H5::DataSpace{sizeof...(Size), dimsf}),
//        dataset_name};
//  }
//};
//==============================================================================
class file {
  std::unique_ptr<hid_t>              m_file_id;
  //============================================================================
 public:
  file(filesystem::path const& path) : file{path.c_str()} {}
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  file(std::string const& path) : file{path.c_str()} {}
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  file(char const* path) {
    open(path);
  }
  //----------------------------------------------------------------------------
  file(file const& other)
      : m_file_id{std::make_unique<hid_t>(*other.m_file_id)} {
    H5Iinc_ref(*m_file_id);
  }
  //----------------------------------------------------------------------------
  file(file&&) noexcept = default;
  //----------------------------------------------------------------------------
  auto operator=(file const& other) -> file& {
    if (m_file_id != nullptr) {
      H5Fclose(*m_file_id);
    }
    m_file_id = std::make_unique<hid_t>(*other.m_file_id);
    H5Iinc_ref(*m_file_id);
    return *this;
  }
  //----------------------------------------------------------------------------
  auto operator=(file&&) noexcept -> file& = default;
  //----------------------------------------------------------------------------
  ~file() {
    if (m_file_id != nullptr) {
      H5Fclose(*m_file_id);
    }
  }

 private:
  auto open(char const* path) -> void {
    if (filesystem::exists(filesystem::path{path})) {
      m_file_id =
          std::make_unique<hid_t>(H5Fopen(path, H5F_ACC_RDWR, H5P_DEFAULT));
    } else {
      m_file_id = std::make_unique<hid_t>(
          H5Fcreate(path, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT));
    }
  }

 public:
  //============================================================================
  // auto group(std::string const& group_name) {
  //  return hdf5::group{m_file_id, m_file_id->openGroup(group_name),
  //                     group_name};
  //}
  //============================================================================
#ifdef __cpp_concepts
  template <typename T, typename IndexOrder = x_fastest, integral... Size>
#else
  template <typename T, typename IndexOrder = x_fastest, typename... Size,
            enable_if<is_integral<Size...>> = true>
#endif
  auto add_dataset(std::string const& dataset_name, Size... size) {
    hsize_t dimsf[]{static_cast<hsize_t>(size)...};  // data set dimensions
    std::reverse(dimsf, dimsf + sizeof...(size));
    auto dspace = H5Screate_simple(sizeof...(Size), dimsf, nullptr);
    auto dataset_id =
        H5Dcreate2(*m_file_id, dataset_name.c_str(), H5T_STD_I32BE, dspace,
                   H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    H5Sclose(dspace);
    return hdf5::dataset<T>{m_file_id, dataset_id, dataset_name};
  }
  //----------------------------------------------------------------------------
  template <typename T>
  auto dataset(char const* dataset_name) const {
    return hdf5::dataset<T>{m_file_id,
                            H5Dopen(*m_file_id, dataset_name, H5P_DEFAULT),
                            dataset_name};
  }
  //----------------------------------------------------------------------------
  template <typename T>
  auto dataset(std::string const& dataset_name) const {
    return hdf5::dataset<T>{
        m_file_id,
        H5Dopen(*m_file_id, dataset_name.c_str(), H5P_DEFAULT), dataset_name};
  }
};
//==============================================================================
}  // namespace tatooine::hdf5
//==============================================================================
#endif
#else
#pragma message(including <tatooine / hdf5.h> without HDF5 support.)
#endif
